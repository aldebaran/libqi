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


#include <qi/log.hpp>

#include <boost/thread.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

#include <qimessaging/transport_socket.hpp>
#include "src/tcptransportsocket.hpp"
#include "src/message_p.hpp"
#include "src/buffer_p.hpp"

#include <qimessaging/message.hpp>
#include <qimessaging/datastream.hpp>
#include <qimessaging/buffer.hpp>
#include "src/transport_socket_p.hpp"

namespace qi
{

  TransportSocket::TransportSocket(qi::EventLoop *eventLoop)
    : _p(new TransportSocketPrivate(this, eventLoop))
  {
  }

  TransportSocket::TransportSocket(TransportSocketPrivate *p)
    : _p(p)
  {
  }

  TransportSocket::~TransportSocket()
  {
    delete _p;
    _p = 0;
  }


  qi::Url TransportSocket::url() const {
    return _p->_url;
  }

  int TransportSocket::status() const {
    return _p->_status;
  }

  bool TransportSocket::isConnected() const
  {
    return _p->_connected;
  }

  qi::SignalBase::Link TransportSocket::messagePendingConnect(unsigned int serviceId, boost::function<void (qi::Message)> fun) {
    return _p->_dispatcher.messagePendingConnect(serviceId, fun);
  }

  bool                 TransportSocket::messagePendingDisconnect(unsigned int serviceId, qi::SignalBase::Link linkId) {
    return _p->_dispatcher.messagePendingDisconnect(serviceId, linkId);
  }

  TransportSocketPtr makeTransportSocket(const std::string &protocol, qi::EventLoop *eventLoop) {
    TransportSocketPtr ret;

    if (protocol != "tcp") {
      qiLogError("TransportSocket") << "Unrecognized protocol to create the TransportSocket: " << protocol;
      return ret;
    }
    return TcpTransportSocketPtr(new TcpTransportSocket(eventLoop));
  }
}

