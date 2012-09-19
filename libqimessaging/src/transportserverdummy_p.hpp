#pragma once
/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/

#ifndef _SRC_TRANSPORTSERVERDUMMY_P_HPP_
#define _SRC_TRANSPORTSERVERDUMMY_P_HPP_

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

#endif  // _SRC_TRANSPORTSERVERDUMMY_P_HPP_
