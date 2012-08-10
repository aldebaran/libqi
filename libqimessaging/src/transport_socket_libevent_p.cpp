/*
** Author(s):
**  - Herve Cuche <hcuche@aldebaran-robotics.com>
**
** Copyright (C) 2012 Aldebaran Robotics
*/

#include <iostream>
#include <cstring>
#include <map>

#ifdef ANDROID
#include <linux/in.h> // for  IPPROTO_TCP
#endif

#include <event2/util.h>
#include <event2/event.h>
#include <event2/buffer.h>
#include <event2/bufferevent.h>
#include <boost/thread.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

#include "src/transport_socket_p.hpp"
#include "src/transport_socket_libevent_p.hpp"
#include "src/network_thread.hpp"
#include "src/message_p.hpp"
#include "src/buffer_p.hpp"
#include "src/session_p.hpp"

#include <qi/log.hpp>
#include <qi/types.hpp>
#include <qimessaging/session.hpp>
#include <qimessaging/message.hpp>
#include <qimessaging/datastream.hpp>
#include <qimessaging/buffer.hpp>

#define MAX_LINE 16384

namespace qi
{
  class ScopeAtomicSetter
  {
  public:
    ScopeAtomicSetter(qi::atomic<long>& b)
    : b(b) { ++b;}
    ~ScopeAtomicSetter() { --b;}
    qi::atomic<long>& b;
  };

  static int _gst()
  {
    static const std::string st = qi::os::getenv("QI_SOCKET_TIMEOUT");
    if (st != "")
    {
      return atoi(st.c_str());
    }
    else
    {
      // Default timeout in NAOqi 1
      return 5 * 60;
    }
  }
  static int _socket_timeout = _gst();
  static inline unsigned int getSocketTimeout()
  {
    return _socket_timeout;
  }

  static void cleancb(evutil_socket_t fd, short what, void *arg)
  {
    TransportSocketLibEvent *ts = static_cast<TransportSocketLibEvent*>(arg);
    ts->onCleanPendingMessages();
  }

  static void readcb(struct bufferevent *bev,
                     void *context)
  {
    TransportSocketLibEvent *tc = static_cast<TransportSocketLibEvent*>(context);
    tc->readcb(bev, context);
  }

  static void writecb(struct bufferevent* bev,
                      void* context)
  {
    TransportSocketLibEvent *tc = static_cast<TransportSocketLibEvent*>(context);
    tc->writecb(bev, context);
  }


  static void eventcb(struct bufferevent *bev,
                      short error,
                      void *context)
  {
    TransportSocketLibEvent *tc = static_cast<TransportSocketLibEvent*>(context);
    tc->eventcb(bev, error, context);
  }


  void TransportSocketLibEvent::readcb(struct bufferevent *bev,
                                       void               *QI_UNUSED(context))
  {
    struct evbuffer *input = bufferevent_get_input(bev);
    int              msgId = 0;
    unsigned int magicValue = qi::MessagePrivate::magic;

    while (true)
    {
      if (msg == NULL)
      {
        msg = new qi::Message();
        readHdr = true;
      }

      if (readHdr)
      {
        if (evbuffer_get_length(input) < sizeof(MessagePrivate::MessageHeader))
          return;

        struct evbuffer_ptr p;
        evbuffer_ptr_set(input, &p, 0, EVBUFFER_PTR_SET);
        // search a messageMagic number
        p = evbuffer_search(input, (const char *)&magicValue, sizeof(qi::uint32_t), &p);
        if (p.pos < 0)
        {
          qiLogWarning("qimessaging.TransportSocketLibevent") << "No magic found in the message. Waiting for more data.";
          return;
        }
        // Drain to the magic
        evbuffer_drain(input, p.pos);
        // Get the message header
        // there is a copy here.
        evbuffer_copyout(input, msg->_p->getHeader(), sizeof(MessagePrivate::MessageHeader));
        // check if the msg is valid
        if (!msg->isValid())
        {
          qiLogError("qimessaging.TransportSocketLibevent") << "Message received is invalid! Try to find a new one.";
          // only drop the magic and restart scanning
          evbuffer_drain(input, sizeof(qi::uint32_t));
          return;
        }
        // header is valid, next step get all the buffer
        // remove the header from the evbuffer
        evbuffer_drain(input, sizeof(MessagePrivate::MessageHeader));

        readHdr = false;
      }

      if (evbuffer_get_length(input) <
          static_cast<MessagePrivate::MessageHeader*>(msg->_p->getHeader())->size)
        return;

      /* we have to let the Buffer know we are going to push some data inside */
      qi::Buffer buf;
      buf.reserve(static_cast<MessagePrivate::MessageHeader*>(msg->_p->getHeader())->size);
      msg->setBuffer(buf);

      evbuffer_remove(input,
                      buf.data(),
                      buf.size());

      {
        boost::mutex::scoped_lock l(mtx);

        msgSend[msg->id()].msg = msg;
        msgId = msg->id();
        msg = NULL;
        cond.notify_all();
      }
      std::vector<TransportSocketInterface *> localCallbacks;
      boost::recursive_mutex::scoped_lock l(mtxCallback);
      localCallbacks = tcd;
      ScopeAtomicSetter bs(inMethod);
      std::vector<TransportSocketInterface *>::const_iterator it;
      for (it = localCallbacks.begin(); it != localCallbacks.end(); ++it)
        (*it)->onSocketReadyRead(self, msgId);
    }
  }

  void TransportSocketLibEvent::onWriteDone()
  {
    std::vector<TransportSocketInterface *> localCallbacks;
    boost::recursive_mutex::scoped_lock l(mtxCallback);
    localCallbacks = tcd;

    ScopeAtomicSetter bs(inMethod);
    std::vector<TransportSocketInterface *>::const_iterator it;
    for (it = localCallbacks.begin(); it != localCallbacks.end(); ++it)
      (*it)->onSocketWriteDone(self);
  }

  void TransportSocketLibEvent::writecb(struct bufferevent *QI_UNUSED(bev),
                                        void               *QI_UNUSED(context))
  {
    /*
     * EV_WRITE
     * This flag indicates an event that becomes active when the provided file
     * descriptor is ready for writing.
     * (http://www.wangafu.net/~nickm/libevent-book/Ref4_event.html)
     *
     * This means the callback is not called when data was sent, only when
     * the socket is ready for writing.
     */
  }

  void TransportSocketLibEvent::eventcb(struct bufferevent *bev,
                                        short events,
                                        void *QI_UNUSED(context))
  {
    boost::recursive_mutex::scoped_lock sl(mutex);
    if (!this->bev)
      return;
    boost::recursive_mutex::scoped_lock l(mtxCallback);

    std::vector<TransportSocketInterface *>::const_iterator it;
    std::vector<TransportSocketInterface *> localCallbacks;
    localCallbacks = tcd;

    if (events & BEV_EVENT_CONNECTED)
    {
      connected = true;
      for (it = localCallbacks.begin(); it != localCallbacks.end(); ++it)
        (*it)->onSocketConnected(self);
    }
    else if ((events & BEV_EVENT_EOF) || (events & BEV_EVENT_ERROR))
    {
      if (this->bev)
      {
        bufferevent_setcb(bev, 0, 0, 0, 0);
        bufferevent_free(bev);
        this->bev = 0;
      }
      if (clean_event) {
        event_del(clean_event);
        event_free(clean_event);
        clean_event = NULL;
      }
      bool wasco = connected;
      connected = false;
      //for waitForId
      cond.notify_all();
      ScopeAtomicSetter bs(inMethod);
      for (it = localCallbacks.begin(); it != localCallbacks.end(); ++it)
        (*it)->onSocketConnectionError(self);
      if (wasco)
      {
        for (it = localCallbacks.begin(); it != localCallbacks.end(); ++it)
          (*it)->onSocketDisconnected(self);
      }
      cleanPendingMessages();

      status = errno;
      // check errno to see what error occurred
      qiLogVerbose("qimessaging.TransportSocketLibevent")  << "socket terminate (" << errno << "): " << strerror(errno) << std::endl;
    }
    else if (events & BEV_EVENT_TIMEOUT)
    {
      // must be a timeout event handle, handle it
      qiLogError("qimessaging.TransportSocketLibevent")  << "must be a timeout event handle, handle it" << std::endl;
    }
  }

  TransportSocketLibEvent::TransportSocketLibEvent(TransportSocket *socket)
    : TransportSocketPrivate(socket)
    , bev(NULL)
    , fd(-1)
    , clean_event(0)
    , inMethod(false)
  {
  }

  TransportSocketLibEvent::TransportSocketLibEvent(TransportSocket *socket, int fileDesc, void *data)
    : TransportSocketPrivate(socket)
    , bev(NULL)
    , fd(fileDesc)
    , clean_event(0)
    , inMethod(false)
  {
    struct event_base *base = static_cast<event_base *>(data);
    bev = bufferevent_socket_new(base, fd, BEV_OPT_CLOSE_ON_FREE | BEV_OPT_THREADSAFE);
    bufferevent_setcb(bev, ::qi::readcb, ::qi::writecb, ::qi::eventcb, this);
    bufferevent_setwatermark(bev, EV_WRITE, 0, MAX_LINE);
    bufferevent_enable(bev, EV_READ|EV_WRITE);
    connected = true;

    if (!clean_event)
    {
      struct timeval clean_period = { 1, 0 };
      clean_event = event_new(base, -1, EV_PERSIST, cleancb, this);
      event_add(clean_event, &clean_period);
    }
  }

  TransportSocketLibEvent::~TransportSocketLibEvent()
  {
    if (isConnected())
      disconnect();
  }

  void TransportSocketLibEvent::onCleanPendingMessages()
  {
    std::map<unsigned int, TransportSocketPrivate::PendingMessage>::iterator it;
    boost::mutex::scoped_lock ll(mtx);
    std::vector<TransportSocketInterface *> localCallbacks;
    boost::recursive_mutex::scoped_lock l(mtxCallback);
    localCallbacks = tcd;
    for (it = msgSend.begin();it != msgSend.end(); ++it)
    {
      if (time(0) - it->second.timestamp >= getSocketTimeout())
      {
        // Call onSocketTimeout callback
        ScopeAtomicSetter bs(inMethod);
        for (std::vector<TransportSocketInterface *>::const_iterator it2 = localCallbacks.begin();
             it2 != localCallbacks.end();
             ++it2)
        {
          (*it2)->onSocketTimeout(self, it->first);
        }
        qiLogError("qimessaging.TransportSocket") << "Message timed out: "
          << it->first;
        msgSend.erase(it);
      }
    }
  }

  void TransportSocketLibEvent::cleanPendingMessages()
  {
    std::vector<TransportSocketInterface *> localCallbacks;
    boost::recursive_mutex::scoped_lock l(mtxCallback);
    localCallbacks = tcd;

    // Call onSocketTimeout callback
    while (true)
    {
      unsigned int id = 0;
      {
        boost::mutex::scoped_lock l(mtx);
        std::map<unsigned int, TransportSocketPrivate::PendingMessage>::iterator msgSendIt = msgSend.begin();
        if (msgSendIt == msgSend.end())
        {
          break;
        }
        id = msgSendIt->first;
        msgSend.erase(msgSendIt);
      }

      for (std::vector<TransportSocketInterface*>::const_iterator localCallbacksIt = localCallbacks.begin();
           localCallbacksIt != localCallbacks.end();
           ++localCallbacksIt)
      {
        (*localCallbacksIt)->onSocketTimeout(self, id);
      }
    }
  }

  void TransportSocketLibEvent::onBufferSent(const void *QI_UNUSED(data),
                                             size_t QI_UNUSED(datalen),
                                             void *buffer)
  {
    Buffer *b = static_cast<Buffer *>(buffer);
    delete b;
  }

  void TransportSocketLibEvent::onMessageSent(const void *QI_UNUSED(data),
                                              size_t QI_UNUSED(datalen),
                                              void *msg)
  {
    Message *m = static_cast<Message *>(msg);
    delete m;
  }

  bool TransportSocketLibEvent::connect(qi::Session *session,
                                        const qi::Url &url)
  {
    this->session = session;
    const std::string &address = url.host();
    struct evutil_addrinfo *ai = NULL;
    struct evutil_addrinfo  hint;
    char                    portbuf[10];

    if (url.port() == 0) {
      qiLogError("qimessaging.TransportSocket") << "Error try to connect to a bad address: " << url.str();
      return false;
    }
    qiLogVerbose("qimessaging.transportsocket.connect") << "Trying to connect to " << url.host() << ":" << url.port();
    if (!isConnected())
    {
      bev = bufferevent_socket_new(session->_p->_networkThread->getEventBase(), -1, BEV_OPT_CLOSE_ON_FREE | BEV_OPT_THREADSAFE);
      bufferevent_setcb(bev, ::qi::readcb, ::qi::writecb, ::qi::eventcb, this);
      bufferevent_setwatermark(bev, EV_WRITE, 0, MAX_LINE);
      bufferevent_enable(bev, EV_READ|EV_WRITE);

      evutil_snprintf(portbuf, sizeof(portbuf), "%d", url.port());
      memset(&hint, 0, sizeof(hint));
      hint.ai_family = AF_UNSPEC;
      hint.ai_protocol = IPPROTO_TCP;
      hint.ai_socktype = SOCK_STREAM;
      int err = evutil_getaddrinfo(address.c_str(), portbuf, &hint, &ai);
      if (err != 0)
      {
        qiLogError("qimessaging.TransportSocketLibEvent") << "Cannot resolve dns (" << address << ")";
        return (false);
      }

      if (!clean_event)
      {
        struct timeval clean_period = { 1, 0 };
        clean_event = event_new(session->_p->_networkThread->getEventBase(), -1, EV_PERSIST, cleancb, this);
        event_add(clean_event, &clean_period);
      }

      int result = bufferevent_socket_connect(bev, ai->ai_addr, ai->ai_addrlen);

      if (result == 0)
        return true;

      return false;
    }
    else
    {
      qiLogError("qimessaging.TransportSocketLibevent") << "socket is already connected.";
    }

    return false;
  }

  void TransportSocketLibEvent::destroy()
  {
    // Queue request if not in network thread.
    if (!session->_p->_networkThread->isInNetworkThread())
    {
      session->_p->_networkThread->asyncCall(1,
        boost::bind(&TransportSocketLibEvent::destroy, this));
    }
    else
    {
      bool delay;
      {
        // If we are connected, disconnect and retry asynchronously.
        boost::recursive_mutex::scoped_lock sl(mutex);
        delay = bev;
        disconnect();
      }
      // While our tracker says something is running, try again.
      delay = delay || *inMethod;
      if (delay)
      {
        session->_p->_networkThread->asyncCall(1,
          boost::bind(&TransportSocketLibEvent::destroy, this));
      }
      else
      {
        delete this;
      }
    }
  }

  void disconnect_dec(TransportSocketLibEvent* ptr)
  {
    --ptr->inMethod;
    ptr->disconnect();
  }

  void TransportSocketLibEvent::disconnect()
  {
    boost::recursive_mutex::scoped_lock sl(mutex);
    if (!session->_p->_networkThread->isInNetworkThread())
    {
      ++inMethod;
      session->_p->_networkThread->asyncCall(1,
        boost::bind(disconnect_dec, this));
      return;
    }
    if (clean_event) {
      event_del(clean_event);
      event_free(clean_event);
      clean_event = NULL;
    }
    if (isConnected())
    {
      /*
       * Currently (as of Libevent 2.0.5-beta), bufferevent_flush() is only
       * implemented for some bufferevent types. In particular, socket-based
       * bufferevents dont have it.
       * (http://www.wangafu.net/~nickm/libevent-book/Ref6_bufferevent.html)
       *
       * :'(
       */
       //bufferevent_flush(bev, EV_WRITE, BEV_NORMAL);
       // Must be in network thread due to a libevent deadlock bug.
       bufferevent_setcb(bev, 0, 0, 0, 0);
       bufferevent_free(bev);
       bev = NULL;
       connected = false;
       cleanPendingMessages();
    }
    //else
    //{
    //  qiLogError("qimessaging.TransportSocketLibevent") << "socket is not connected.";
    //}
  }

  void send_dec(TransportSocketLibEvent* sock,
    qi::Message* m)
  {
    --sock->inMethod;
    sock->_send(m);
  }

  bool TransportSocketLibEvent::send(const qi::Message &msg)
  {
    boost::recursive_mutex::scoped_lock sl(mutex);
    if (!isConnected())
    {
      qiLogError("qimessaging.TransportSocketLibevent") << "socket is not connected.";
      return false;
    }
    qi::Message *m = new qi::Message();
    *m = msg;
    if (!session->_p->_networkThread->isInNetworkThread())
    {
      ++inMethod;
      session->_p->_networkThread->asyncCall(1,
        boost::bind(&send_dec, this, m));
      return true;
    }
    else
      return _send(m);
  }

  bool TransportSocketLibEvent::_send(qi::Message* m)
  {
    if (!isConnected())
    {
      qiLogError("qimessaging.TransportSocketLibevent") << "socket is not connected.";
      return false;
    }

    m->_p->complete();

    if (m->type() == qi::Message::Type_Call)
    {
      boost::mutex::scoped_lock l(mtx);
      msgSend[m->id()].timestamp = time(0);
    }


    struct evbuffer *mess = evbuffer_new();
    // m might be deleted.
    qi::Buffer *b = new qi::Buffer(m->buffer());
    if (evbuffer_add_reference(mess,
                               m->_p->getHeader(),
                               sizeof(qi::MessagePrivate::MessageHeader),
                               qi::TransportSocketLibEvent::onMessageSent,
                               static_cast<void *>(m)) != 0)
    {
      qiLogError("qimessaging.TransportSocketLibevent") << "Add reference fail in header";
      evbuffer_free(mess);
      delete m;
      delete b;
      return false;
    }
    size_t sz = b->size();

    if (sz)
    {
      if (evbuffer_add_reference(mess,
                                 b->data(),
                                 sz,
                                 qi::TransportSocketLibEvent::onBufferSent,
                                 static_cast<void *>(b)) != 0)
      {
        qiLogError("qimessaging.TransportSocketLibevent") << "Add reference fail for block of size " << sz;
        evbuffer_free(mess);
        delete b;
        return false;
      }
    }
    else
    {
      delete b;
    }

    if (bufferevent_write_buffer(bev, mess) != 0)
    {
      qiLogError("qimessaging.TransportSocketLibevent") << "Can't add buffer to the send queue";
      evbuffer_free(mess);
      return false;
    }
    evbuffer_free(mess);

    onWriteDone();

    return true;
  }
}

