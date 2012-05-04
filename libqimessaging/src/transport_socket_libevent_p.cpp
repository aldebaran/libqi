/*
** Author(s):
**  - Herve Cuche <hcuche@aldebaran-robotics.com>
**
** Copyright (C) 2012 Aldebaran Robotics
*/

#include <iostream>
#include <cstring>
#include <map>
#include <stdint.h>

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
#include <qimessaging/session.hpp>
#include <qimessaging/message.hpp>
#include <qimessaging/datastream.hpp>
#include <qimessaging/buffer.hpp>

#define MAX_LINE 16384

namespace qi
{
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

        evbuffer_remove(input,
                        msg->_p->getHeader(),
                        sizeof(MessagePrivate::MessageHeader));
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
      assert(msg->isValid());

      {
        boost::mutex::scoped_lock l(mtx);
        msgSend[msg->id()] = msg;
        cond.notify_all();
      }
      if (tcd)
        tcd->onSocketReadyRead(self, msg->id());

      msg = NULL;
    }
  }


  void TransportSocketLibEvent::writecb(struct bufferevent *QI_UNUSED(bev),
                                        void               *QI_UNUSED(context))
  {
    if (tcd)
      tcd->onSocketWriteDone(self);
  }

  void TransportSocketLibEvent::eventcb(struct bufferevent *bev,
                                        short events,
                                        void *QI_UNUSED(context))
  {
    if (events & BEV_EVENT_CONNECTED)
    {
      connected = true;
      if (tcd)
        tcd->onSocketConnected(self);
    }
#ifdef WIN32
    /* On windows, BEV_EVENT_EOF doesn't pop on service kill
    ** but BEV_EVENT_READING and BEV_EVENT_ERROR suggest an error while reading.
    ** For a better fix, need to find out how to make libevent pop the proper error. (pr)
    */
    else if ((events & BEV_EVENT_READING) && (events & BEV_EVENT_ERROR))
#else
    else if (events & BEV_EVENT_EOF)
#endif
    {
      bufferevent_free(bev);
      bev = 0;
      connected = false;
      //for waitForId
      cond.notify_all();
      if (tcd)
        tcd->onSocketDisconnected(self);
      // connection has been closed, do any clean up here
    }
    else if (events & BEV_EVENT_ERROR)
    {
      bufferevent_free(bev);
      bev = 0;
      connected = false;
      //for waitForId
      cond.notify_all();

      if (tcd)
        tcd->onSocketConnectionError(self);

      // check errno to see what error occurred
      qiLogError("qimessaging.TransportSocketLibevent")  << "Cannot connect" << std::endl;
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
  {
  }

  TransportSocketLibEvent::TransportSocketLibEvent(TransportSocket *socket, int fileDesc, void *data)
    : TransportSocketPrivate(socket)
    , bev(NULL)
    , fd(fileDesc)
  {
    struct event_base *base = static_cast<event_base *>(data);
    bev = bufferevent_socket_new(base, fd, BEV_OPT_CLOSE_ON_FREE);
    bufferevent_setcb(bev, ::qi::readcb, ::qi::writecb, ::qi::eventcb, this);
    bufferevent_setwatermark(bev, EV_WRITE, 0, MAX_LINE);
    bufferevent_enable(bev, EV_READ|EV_WRITE);
    connected = true;
  }

  TransportSocketLibEvent::~TransportSocketLibEvent()
  {
    if (isConnected())
      disconnect();
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
    const std::string &address = url.host();
    unsigned short port = url.port();

    if (!isConnected())
    {
      bev = bufferevent_socket_new(session->_p->_networkThread->getEventBase(), -1, BEV_OPT_CLOSE_ON_FREE);
      bufferevent_setcb(bev, ::qi::readcb, ::qi::writecb, ::qi::eventcb, this);
      bufferevent_setwatermark(bev, EV_WRITE, 0, MAX_LINE);
      bufferevent_enable(bev, EV_READ|EV_WRITE);

      int result = bufferevent_socket_connect_hostname(bev, NULL, AF_INET, address.c_str(), port);
      if (result == 0)
      {
        connected = true;
        return true;
      }
    }
    else
    {
      qiLogError("qimessaging.TransportSocketLibevent") << "socket is already connected.";
    }

    return false;
  }

  void TransportSocketLibEvent::disconnect()
  {
    if (isConnected())
    {
      bufferevent_free(bev);
      bev = NULL;
      connected = false;
    }
    else
    {
      qiLogError("qimessaging.TransportSocketLibevent") << "socket is not connected.";
    }
  }

  bool TransportSocketLibEvent::send(const qi::Message &msg)
  {
    if (!isConnected())
    {
      qiLogError("qimessaging.TransportSocketLibevent") << "socket is not connected.";
      return false;
    }

    qi::Message *m = new qi::Message();
    *m = msg;
    m->_p->complete();
    assert(m->isValid());

    struct evbuffer *evb = bufferevent_get_output(bev);

    // m might be deleted.
    qi::Buffer *b = new qi::Buffer(m->buffer());

    if (evbuffer_add_reference(evb,
                               m->_p->getHeader(),
                               sizeof(qi::MessagePrivate::MessageHeader),
                               qi::TransportSocketLibEvent::onMessageSent,
                               static_cast<void *>(m)) != 0)
    {
      return false;
    }

    if (b->size() &&
        evbuffer_add_reference(evb,
                               b->data(),
                               b->size(),
                               qi::TransportSocketLibEvent::onBufferSent,
                               static_cast<void *>(b)) != 0)
    {
      return false;
    }

    if (bufferevent_write_buffer(bev, evb) != 0)
    {
      return false;
    }

    return true;
  }
}

