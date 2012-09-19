/*
 ** Author(s):
 **  - Herve Cuche <hcuche@aldebaran-robotics.com>
 **
 ** Copyright (C) 2012 Aldebaran Robotics
 */

#pragma once
#ifndef _QIMESSAGING_TRANSPORT_SERVER_DUMMY_P_HPP_
#define _QIMESSAGING_TRANSPORT_SERVER_DUMMY_P_HPP_

# include <qimessaging/api.hpp>
# include <qimessaging/url.hpp>
# include <qimessaging/session.hpp>
# include "src/transportserver_p.hpp"

# include <string>
# include <queue>

namespace qi
{
  class TransportServerDummyPrivate : public TransportServerPrivate
  {
  public:
    TransportServerDummyPrivate(TransportServer* self,
                                const qi::Url &url,
                                EventLoop* ctx);
    virtual ~TransportServerDummyPrivate();
    virtual bool listen();
    virtual void close();
    virtual void destroy();
  private:
    TransportServerDummyPrivate() {};
  };
}

#endif  // _QIMESSAGING_TRANSPORT_SERVER_DUMMY_P_HPP_
