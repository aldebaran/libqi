#pragma once
/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/

#ifndef _SRC_TRANSPORTSOCKET_P_HPP_
#define _SRC_TRANSPORTSOCKET_P_HPP_

# include <qimessaging/url.hpp>
# include "messagedispatcher.hpp"

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
      , _err(0)
      , _status(qi::TransportSocket::Status_Disconnected)
      , _url()
    {}

    virtual ~TransportSocketPrivate() {}

    qi::TransportSocket*    _self;
    qi::EventLoop*          _eventLoop;
    qi::MessageDispatcher   _dispatcher;

    int                     _err;
    TransportSocket::Status _status;
    qi::Url                 _url;
  };
}
#endif  // _SRC_TRANSPORTSOCKET_P_HPP_
