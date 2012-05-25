/*
 ** Author(s):
 **  - Herve Cuche <hcuche@aldebaran-robotics.com>
 **
 ** Copyright (C) 2012 Aldebaran Robotics
 */

#include <iostream>
#include <qi/log.hpp>

#include <qimessaging/session.hpp>
#include <qimessaging/url.hpp>
#include "src/transport_socket_p.hpp"
#include "src/transport_socket_dummy_p.hpp"

namespace qi
{
  TransportSocketDummyPrivate::TransportSocketDummyPrivate(TransportSocket *socket)
    : TransportSocketPrivate(socket)
  {
  }

  TransportSocketDummyPrivate::~TransportSocketDummyPrivate()
  {
  }

  bool TransportSocketDummyPrivate::connect(qi::Session *QI_UNUSED(session),
                                            const qi::Url &QI_UNUSED(url))
  {
    qiLogWarning("TransportSocket") << "You are currently running on dummy"
                                    << " TransportSocket! "
                                    << "You can not connect to anything!";
    return false;
  }

  void TransportSocketDummyPrivate::disconnect()
  {
    qiLogWarning("TransportSocket") << "You are currently running on dummy"
                                    << " TransportSocket! "
                                    << "You can not disconnect from something!";
  }

  bool TransportSocketDummyPrivate::send(const qi::Message &QI_UNUSED(msg))
  {
    qiLogWarning("TransportSocket") << "You are currently running on dummy"
                                    << " TransportSocket! "
                                    << "You can not send something!";
    return false;

  }
}
