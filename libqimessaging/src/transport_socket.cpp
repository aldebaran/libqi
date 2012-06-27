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

#include <boost/thread.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

#include <qimessaging/transport_socket.hpp>
#include "src/network_thread.hpp"
#include "src/message_p.hpp"
#include "src/buffer_p.hpp"
#include "src/transport_socket_libevent_p.hpp"
#include "src/transport_socket_dummy_p.hpp"

#include <qimessaging/session.hpp>
#include <qimessaging/message.hpp>
#include <qimessaging/datastream.hpp>
#include <qimessaging/buffer.hpp>
#include "src/session_p.hpp"

namespace qi
{
  TransportSocketInterface::~TransportSocketInterface()
  {
  }

  TransportSocket::TransportSocket()
    : _p(new TransportSocketDummyPrivate(this))
  {
  }

  TransportSocket::~TransportSocket()
  {
    delete _p;
  }

  bool TransportSocket::connect(qi::Session *session,
                                const qi::Url &url)
  {
    TransportSocketPrivate *save = _p;
    _p->status = 0;
    switch (url.protocol())
    {
      case qi::Url::Protocol_Tcp:
        _p = new qi::TransportSocketLibEvent(this);
        _p->tcd = save->tcd;
        _p->connected = save->connected;
        _p->readHdr = save->readHdr;
        _p->msg = save->msg;
        _p->self = save->self;
        break;
      default:
        qiLogError("TransportSocket") << "Unrecognized protocol to create the TransportSocket."
                                      << " TransportSocket create with dummy implementation.";
        delete save;
        return false;
    }
    delete save;
    return _p->connect(session, url);
  }

  void TransportSocket::disconnect()
  {
    if (_p == NULL)
    {
      qiLogError("qimessaging.TransportSocket") << "socket is not connected.";
      return;
    }

    _p->disconnect();
  }

  bool TransportSocket::waitForConnected(int msecs)
  {
    if (_p == NULL)
    {
      qiLogError("qimessaging.TransportSocket") << "socket is not connected.";
      return false;
    }
    return _p->waitForConnected(msecs);

  }

  bool TransportSocket::waitForDisconnected(int msecs)
  {
    if (_p == NULL)
    {
      qiLogError("qimessaging.TransportSocket") << "socket is not connected.";
      return false;
    }
    return _p->waitForDisconnected(msecs);
  }

  int TransportSocket::status() const {
    return _p->status;
  }

  bool TransportSocket::waitForId(int id, int msecs)
  {
    if (_p == NULL)
    {
      qiLogError("qimessaging.TransportSocket") << "socket is not connected.";
      return false;
    }
    return _p->waitForId(id, msecs);
  }

  bool TransportSocket::read(int id, qi::Message *msg)
  {
    if (_p == NULL)
    {
      qiLogError("qimessaging.TransportSocket") << "socket is not connected.";
      return false;
    }
    return _p->read(id, msg);
  }


  bool TransportSocket::send(const qi::Message &msg)
  {
    if (_p == NULL)
    {
      qiLogError("qimessaging.TransportSocket") << "socket is not connected.";
      return false;
    }

    return _p->send(msg);
  }

  void TransportSocket::setCallbacks(TransportSocketInterface *delegate)
  {
    if (_p == NULL)
    {
      qiLogError("qimessaging.TransportSocket") << "socket is not connected.";
      return;
    }
    _p->setCallbacks(delegate);
  }

  bool TransportSocket::isConnected() const
  {
    if (_p == NULL)
    {
      qiLogError("qimessaging.TransportSocket") << "socket is not connected.";
      return false;
    }
    return _p->isConnected();
  }

}

