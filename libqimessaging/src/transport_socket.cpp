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
#include "src/buffer_p.hpp"
#include "src/message_p.hpp"

#include <qimessaging/message.hpp>
#include <qimessaging/datastream.hpp>
#include <qimessaging/buffer.hpp>

#define MAX_LINE 16384

namespace qi {

const unsigned int headerSize = 8 * sizeof(uint32_t);

struct TransportSocketPrivate
{
  TransportSocketPrivate()
    : tcd(NULL)
    , bev(NULL)
    , connected(false)
    , fd(-1)
    , readHdr(true)
  {
    msg = NULL;
  }

  TransportSocketPrivate(int fileDesc)
    : tcd(NULL)
    , bev(NULL)
    , connected(false)
    , fd(fileDesc)
    , readHdr(true)
  {
    msg = NULL;
  }

  ~TransportSocketPrivate()
  {
  }


  TransportSocketInterface            *tcd;
  struct bufferevent                  *bev;
  bool                                 connected;
  std::map<unsigned int, qi::Message*> msgSend;
  boost::mutex                         mtx;
  boost::condition_variable            cond;
  int                                  fd;


  // data to rebuild message
  bool                         readHdr;
  qi::Message                 *msg;
};



static void readcb(struct bufferevent *bev,
                   void *context)
{
  TransportSocket *tc = static_cast<TransportSocket*>(context);
  tc->readcb(bev, context);
}

static void writecb(struct bufferevent* bev,
                    void* context)
{
  TransportSocket *tc = static_cast<TransportSocket*>(context);
  tc->writecb(bev, context);
}


static void eventcb(struct bufferevent *bev,
                    short error,
                    void *context)
{
  TransportSocket *tc = static_cast<TransportSocket*>(context);
  tc->eventcb(bev, error, context);
}


void TransportSocket::readcb(struct bufferevent *bev,
                             void *context)
{
  struct evbuffer *input = bufferevent_get_input(bev);
  size_t length = 0;

  while (true)
  {
    if (_p->msg == NULL)
    {
      _p->msg = new qi::Message();
      _p->readHdr = true;
    }

    if (_p->readHdr)
    {
      length = evbuffer_get_length(input);
      if (length < headerSize)
        return;

      evbuffer_remove(input,
                      (char*)_p->msg->_p,
                      headerSize);

      _p->msg->checkMagic();
      _p->readHdr = false;
    }

    length = evbuffer_get_length(input);
    if (length < _p->msg->_p->size)
      return;

    evbuffer_remove_buffer(input,
                           _p->msg->buffer()->_p->data(),
                           _p->msg->_p->size);

    {
      boost::mutex::scoped_lock l(_p->mtx);
      _p->msgSend[_p->msg->id()] = _p->msg;
      _p->cond.notify_all();
    }
    if (_p->tcd)
      _p->tcd->onSocketReadyRead(this, _p->msg->id());

    _p->msg = NULL;
  }
}


void TransportSocket::writecb(struct bufferevent* bev,
                              void* context)
{
  if (_p->tcd)
    _p->tcd->onSocketWriteDone(this);
}

void TransportSocket::eventcb(struct bufferevent *bev,
                              short events,
                              void *context)
{
  if (events & BEV_EVENT_CONNECTED)
  {
    qi::Message msg;
    _p->connected = true;
    if (_p->tcd)
      _p->tcd->onSocketConnected(this);
  }
  else if (events & BEV_EVENT_EOF)
  {
    qi::Message msg;
    _p->connected = false;
    //for waitForId
    _p->cond.notify_all();
    if (_p->tcd)
      _p->tcd->onSocketDisconnected(this);
    // connection has been closed, do any clean up here
    qiLogInfo("qimessaging.TransportSocket") << "connection has been closed, do any clean up here" << std::endl;
  }
  else if (events & BEV_EVENT_ERROR)
  {
    bufferevent_free(_p->bev);
    _p->bev = 0;
    _p->connected = false;
    //for waitForId
    _p->cond.notify_all();
    if (_p->tcd)
      _p->tcd->onSocketConnectionError(this);
    // check errno to see what error occurred
    qiLogError("qimessaging.TransportSocket")  << "Cannnot connect" << std::endl;
  }
  else if (events & BEV_EVENT_TIMEOUT)
  {
    // must be a timeout event handle, handle it
    qiLogError("qimessaging.TransportSocket")  << "must be a timeout event handle, handle it" << std::endl;
  }
}

TransportSocket::TransportSocket()
{
  _p = new TransportSocketPrivate();
}

TransportSocket::TransportSocket(int fd,
                                 struct event_base *base)
{
  _p = new TransportSocketPrivate(fd);

  _p->bev = bufferevent_socket_new(base, _p->fd, BEV_OPT_CLOSE_ON_FREE);
  bufferevent_setcb(_p->bev, ::qi::readcb, ::qi::writecb, ::qi::eventcb, this);
  bufferevent_setwatermark(_p->bev, EV_WRITE, 0, MAX_LINE);
  bufferevent_enable(_p->bev, EV_READ|EV_WRITE);

  _p->connected = true;
}

TransportSocket::~TransportSocket()
{
  disconnect();
  delete _p;
}

bool TransportSocket::connect(const qi::Url     &url,
                              struct event_base *base)
{
  const std::string &address = url.host();
  unsigned short port = url.port();

  if (!_p->connected)
  {
    _p->bev = bufferevent_socket_new(base, -1, BEV_OPT_CLOSE_ON_FREE);
    bufferevent_setcb(_p->bev, ::qi::readcb, ::qi::writecb, ::qi::eventcb, this);
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
    {
      boost::mutex::scoped_lock l(_p->mtx);
      it = _p->msgSend.find(id);
      if (it != _p->msgSend.end())
      {
        *msg = *(it->second);
        _p->msgSend.erase(it);
        return true;
      }
    }
  }
  return false;
}


bool TransportSocket::send(const qi::Message &msg)
{
  if (msg.type() == qi::Message::Type_None)
  {
    qiLogError("qimessaging.TransportSocket")  << "Message dropped (type is None)" << std::endl;
    assert(msg.type() != qi::Message::Type_None);
    return false;
  }

  if (msg.service() == qi::Message::Service_None)
  {
    qiLogError("qimessaging.TransportSocket")  << "Message dropped (service is 0)" << std::endl;
    assert(msg.service() != qi::Message::Service_None);
    return false;
  }

  if (msg.path() == qi::Message::Path_None)
  {
    qiLogError("qimessaging.TransportSocket")  << "Message dropped (path is 0)" << std::endl;
    assert(msg.path() != qi::Message::Path_None);
    return false;
  }

  // If we do not use qi::Buffer (using qi::Message instead of const qi::Message)
  // we win 500 msg/s.
  qi::Buffer     buf;
  qi::DataStream d(&buf);
  d << msg;

  if (_p->connected &&
      (bufferevent_write_buffer(_p->bev, buf._p->data()) == 0))
    return true;

  return false;
}

void TransportSocket::setDelegate(TransportSocketInterface *delegate)
{
  _p->tcd = delegate;
}

bool TransportSocket::isConnected()
{
  return _p->connected;
}

}

