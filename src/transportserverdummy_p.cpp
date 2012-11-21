/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/
#include <iostream>
#include <qi/log.hpp>

#include <qimessaging/session.hpp>
#include <qimessaging/url.hpp>
#include "transportserver_p.hpp"
#include "transportserverdummy_p.hpp"

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
    qiLogVerbose("TransportServer") << "close: You are currently running on dummy"
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
