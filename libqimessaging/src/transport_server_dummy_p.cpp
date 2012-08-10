/*
 ** transport-server.cpp
 ** Login : <hcuche@hcuche-de>
 ** Started on  Wed Jan 11 10:19:42 2012 Herve Cuche
 ** $Id$
 **
 ** Author(s):
 **  - Herve Cuche <hcuche@aldebaran-robotics.com>
 **
 ** Copyright (C) 2012 Herve Cuche
 */

#include <iostream>
#include <qi/log.hpp>

#include <qimessaging/session.hpp>
#include <qimessaging/url.hpp>
#include "src/transport_server_p.hpp"
#include "src/transport_server_dummy_p.hpp"

namespace qi
{
  bool TransportServerDummyPrivate::listen()
  {
    qiLogWarning("TransportServer") << "listen: You are currently running on dummy"
                                    << " TransportServer!";
    return true;
  }

  bool TransportServerDummyPrivate::close()
  {
    qiLogWarning("TransportServer") << "close: You are currently running on dummy"
                                    << " TransportServer!";
    return true;
  }

  void TransportServerDummyPrivate::join()
  {
    qiLogWarning("TransportServer") << "join: You are currently running on dummy"
                                    << " TransportServer!";
  }

  TransportServerDummyPrivate::TransportServerDummyPrivate(TransportServer* self,
                                                           qi::Session *session,
                                                           const qi::Url &url)
    : TransportServerPrivate(self, session, url)
  {
  }

  void TransportServerDummyPrivate::destroy()
  {
    delete this;
  }
  TransportServerDummyPrivate::~TransportServerDummyPrivate()
  {
  }
}
