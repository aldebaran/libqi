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
#include "src/transportserver_p.hpp"
#include "src/transportserverdummy_p.hpp"

namespace qi
{
  bool TransportServerDummyPrivate::listen()
  {
    qiLogWarning("TransportServer") << "listen: You are currently running on dummy"
                                    << " TransportServer!";
    return true;
  }

  void TransportServerDummyPrivate::close()
  {
    qiLogWarning("TransportServer") << "close: You are currently running on dummy"
                                    << " TransportServer!";
  }

  TransportServerDummyPrivate::TransportServerDummyPrivate(TransportServer* self,
                                                           const qi::Url &url,
                                                           EventLoop* ctx)
    : TransportServerPrivate(self, url, ctx)
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
