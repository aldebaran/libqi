#pragma once
/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/

#ifndef _SRC_TRANSPORTSOCKET_P_HPP_
#define _SRC_TRANSPORTSOCKET_P_HPP_

# include <qimessaging/url.hpp>
# include "src/messagedispatcher.hpp"

namespace qi
{

  class EventLoop;
  class TransportSocket;
  class TransportSocketPrivate
  {
  public:
    TransportSocketPrivate(TransportSocket *self, qi::EventLoop *eventLoop)
      : _self(self)
      , _eventLoop(eventLoop)
      , _dispatcher()
      , _connected(false)
      , _status(0)
      , _url()
    {}

    virtual ~TransportSocketPrivate() {}

    qi::TransportSocket*  _self;
    qi::EventLoop*        _eventLoop;
    qi::MessageDispatcher _dispatcher;
    bool                  _connected;
    int                   _status;
    qi::Url               _url;
  };
}
#endif  // _SRC_TRANSPORTSOCKET_P_HPP_
