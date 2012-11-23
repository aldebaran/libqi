/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/

// Disable "'this': used in base member initializer list"
#pragma warning(disable: 4355)

#include <iostream>
#include <cstring>
#include <map>


#include <qi/log.hpp>

#include <boost/thread.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

#include <qimessaging/transportsocket.hpp>
#include "tcptransportsocket.hpp"
#include "message_p.hpp"

#include <qimessaging/message.hpp>
#include <qi/buffer.hpp>
#include "transportsocket_p.hpp"

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

  TransportSocket::Status TransportSocket::status() const {
    return _p->_status;
  }

  int TransportSocket::error() const
  {
    return _p->_err;
  }

  bool TransportSocket::isConnected() const
  {
    return _p->_status == qi::TransportSocket::Status_Connected;
  }

  qi::SignalBase::Link TransportSocket::messagePendingConnect(unsigned int serviceId, boost::function<void (const qi::Message&)> fun) {
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

