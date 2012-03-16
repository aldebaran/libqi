/*
** transport-client.cpp
** Login : <hcuche@hcuche-de>
** Started on  Thu Jan  5 15:21:13 2012 Herve Cuche
** $Id$
**
** Author(s):
**  - Herve Cuche <hcuche@aldebaran-robotics.com>
**
** Copyright (C) 2012 Herve Cuche
*/

#include <iostream>
#include <cstring>
#include <map>
#include <stdint.h>

#include <qi/log.hpp>

#include <event2/util.h>
#include <event2/event.h>
#include <event2/buffer.h>
#include <event2/bufferevent.h>
#include <boost/thread.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

#include <qimessaging/transport_socket.hpp>
#include "src/network_thread.hpp"
#include "src/message_p.hpp"
#include "src/buffer_p.hpp"

#include <qimessaging/session.hpp>
#include <qimessaging/message.hpp>
#include <qimessaging/datastream.hpp>
#include <qimessaging/buffer.hpp>
#include "src/session_p.hpp"

#define MAX_LINE 16384

namespace qi {

class TransportSocketPrivate
{
public:
  explicit TransportSocketPrivate(TransportSocket *socket)
    : tcd(NULL)
    , bev(NULL)
    , connected(false)
    , fd(-1)
    , readHdr(true)
    , msg(0)
    , _self(socket)
  {
  }

  TransportSocketPrivate(TransportSocket *socket, int fileDesc)
    : tcd(NULL)
    , bev(NULL)
    , connected(false)
    , fd(fileDesc)
    , readHdr(true)
    , msg(0)
    , _self(socket)
  {
  }

  ~TransportSocketPrivate()
  {
  }

  static void onBufferSent(const void *data, size_t datalen, void *buffer)
  {
    Buffer *b = static_cast<Buffer *>(buffer);
    delete b;
  }

  static void onMessageSent(const void *data, size_t datalen, void *msg)
  {
    Message *m = static_cast<Message *>(msg);
    delete m;
  }

  TransportSocketInterface            *tcd;
  struct bufferevent                  *bev;
  bool                                 connected;
  std::map<unsigned int, qi::Message*> msgSend;
  boost::mutex                         mtx;
  boost::condition_variable            cond;
  int                                  fd;
  void readcb(struct bufferevent *bev , void *context);
  void writecb(struct bufferevent* bev, void* context);
  void eventcb(struct bufferevent *bev, short error, void *context);

  // data to rebuild message
  bool                         readHdr;
  qi::Message                 *msg;
  qi::TransportSocket         *_self;
};



static void readcb(struct bufferevent *bev,
                   void *context)
{
  TransportSocketPrivate *tc = static_cast<TransportSocketPrivate*>(context);
  tc->readcb(bev, context);
}

static void writecb(struct bufferevent* bev,
                    void* context)
{
  TransportSocketPrivate *tc = static_cast<TransportSocketPrivate*>(context);
  tc->writecb(bev, context);
}


static void eventcb(struct bufferevent *bev,
                    short error,
                    void *context)
{
  TransportSocketPrivate *tc = static_cast<TransportSocketPrivate*>(context);
  tc->eventcb(bev, error, context);
}


void TransportSocketPrivate::readcb(struct bufferevent *bev,
                             void *context)
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
      tcd->onSocketReadyRead(_self, msg->id());

    msg = NULL;
  }
}


void TransportSocketPrivate::writecb(struct bufferevent* bev,
                              void* context)
{
  if (tcd)
    tcd->onSocketWriteDone(_self);
}

void TransportSocketPrivate::eventcb(struct bufferevent *bev,
                              short events,
                              void *context)
{
  if (events & BEV_EVENT_CONNECTED)
  {
    connected = true;
    if (tcd)
      tcd->onSocketConnected(_self);
  }
  else if (events & BEV_EVENT_EOF)
  {
    bufferevent_free(bev);
    bev = 0;
    connected = false;
    //for waitForId
    cond.notify_all();
    if (tcd)
      tcd->onSocketDisconnected(_self);
    // connection has been closed, do any clean up here
    qiLogInfo("qimessaging.TransportSocket") << "connection has been closed, do any clean up here" << std::endl;
  }
  else if (events & BEV_EVENT_ERROR)
  {
    bufferevent_free(bev);
    bev = 0;
    connected = false;
    //for waitForId
    cond.notify_all();
    if (tcd)
      tcd->onSocketConnectionError(_self);
    // check errno to see what error occurred
    qiLogError("qimessaging.TransportSocket")  << "Cannnot connect" << std::endl;
  }
  else if (events & BEV_EVENT_TIMEOUT)
  {
    // must be a timeout event handle, handle it
    qiLogError("qimessaging.TransportSocket")  << "must be a timeout event handle, handle it" << std::endl;
  }
}

TransportSocketInterface::~TransportSocketInterface() {
}

TransportSocket::TransportSocket()
  :_p(new TransportSocketPrivate(this))
{

}

TransportSocket::TransportSocket(int fd, void *data)
  :_p(new TransportSocketPrivate(this, fd))
{
  struct event_base *base = static_cast<event_base *>(data);
  _p->bev = bufferevent_socket_new(base, _p->fd, BEV_OPT_CLOSE_ON_FREE);
  bufferevent_setcb(_p->bev, ::qi::readcb, ::qi::writecb, ::qi::eventcb, _p);
  bufferevent_setwatermark(_p->bev, EV_WRITE, 0, MAX_LINE);
  bufferevent_enable(_p->bev, EV_READ|EV_WRITE);
  _p->connected = true;
}

TransportSocket::~TransportSocket()
{
  if (isConnected())
  {
    disconnect();
  }
  delete _p;
}

bool TransportSocket::connect(qi::Session *session,
                              const qi::Url &url)
{
  const std::string &address = url.host();
  unsigned short port = url.port();

  if (!_p->connected)
  {
    _p->bev = bufferevent_socket_new(session->_p->_networkThread->getEventBase(), -1, BEV_OPT_CLOSE_ON_FREE);
    bufferevent_setcb(_p->bev, ::qi::readcb, ::qi::writecb, ::qi::eventcb, _p);
    bufferevent_setwatermark(_p->bev, EV_WRITE, 0, MAX_LINE);
    bufferevent_enable(_p->bev, EV_READ|EV_WRITE);

    int result = bufferevent_socket_connect_hostname(_p->bev, NULL, AF_INET, address.c_str(), port);
    if (result == 0)
    {
      _p->connected = true;
      return true;
    }
  }

  return false;
}

bool TransportSocket::waitForConnected(int msecs)
{
  // no timeout
  if (msecs < 0)
  {
    while (!_p->connected)
      ;

    return true;
  }

  while (!_p->connected && msecs > 0)
  {
    qi::os::msleep(1);
    msecs--;
  }

  // timeout
  if (msecs == 0)
    return false;

  return true;
}

void TransportSocket::disconnect()
{
  if (_p->connected)
  {
    bufferevent_free(_p->bev);
    _p->bev = NULL;
    _p->connected = false;
  }
}

bool TransportSocket::waitForDisconnected(int msecs)
{
  // no timeout
  if (msecs < 0)
  {
    while (_p->connected)
      ;

    return true;
  }

  while (_p->connected && msecs > 0)
  {
    qi::os::msleep(1);
    msecs--;
  }

  // timeout
  if (msecs == 0)
    return false;

  return true;
}

bool TransportSocket::waitForId(int id, int msecs)
{
  std::map<unsigned int, qi::Message*>::iterator it;
  {
    {
      boost::mutex::scoped_lock l(_p->mtx);
      it = _p->msgSend.find(id);
      if (it != _p->msgSend.end())
        return true;
      if (!_p->connected)
        return false;
      if (msecs > 0)
        _p->cond.timed_wait(l, boost::posix_time::milliseconds(msecs));
      else
        _p->cond.wait(l);
      it = _p->msgSend.find(id);
      if (it != _p->msgSend.end())
        return true;
    }
  }
  return false;
}

bool TransportSocket::read(int id, qi::Message *msg)
{
  std::map<unsigned int, qi::Message*>::iterator it;
  {
    boost::mutex::scoped_lock l(_p->mtx);
    it = _p->msgSend.find(id);
    if (it != _p->msgSend.end())
    {
      *msg = *it->second;
      delete it->second;
      _p->msgSend.erase(it);
      return true;
    }
  }
  return false;
}


bool TransportSocket::send(const qi::Message &msg)
{
  qi::Message *m = new qi::Message;
  *m = msg;
  m->_p->complete();
  assert(m->isValid());

  struct evbuffer *evb = bufferevent_get_output(_p->bev);

  if (!_p->connected)
  {
    return false;
  }

  // m might be deleted.
  qi::Buffer *b = new qi::Buffer(m->buffer());

  if (evbuffer_add_reference(evb,
                             m->_p->getHeader(),
                             sizeof(qi::MessagePrivate::MessageHeader),
                             qi::TransportSocketPrivate::onMessageSent,
                             static_cast<void *>(m)) != 0)
  {
    return false;
  }

  if (b->size() &&
      evbuffer_add_reference(evb,
                             b->data(),
                             b->size(),
                             qi::TransportSocketPrivate::onBufferSent,
                             static_cast<void *>(b)) != 0)
  {
    return false;
  }

  if (bufferevent_write_buffer(_p->bev, evb) != 0)
  {
    return false;
  }

  return true;
}

void TransportSocket::setCallbacks(TransportSocketInterface *delegate)
{
  _p->tcd = delegate;
}

bool TransportSocket::isConnected()
{
  return _p->connected;
}

}

