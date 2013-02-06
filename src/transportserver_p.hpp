#pragma once
/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/

#ifndef _SRC_TRANSPORTSERVER_P_HPP_
#define _SRC_TRANSPORTSERVER_P_HPP_

# include <qimessaging/api.hpp>
# include <qimessaging/url.hpp>
# include <qimessaging/session.hpp>
# include <qimessaging/transportserver.hpp>

# include <string>
# include <queue>
# include <vector>

#include <boost/thread.hpp>

namespace qi {

  class TransportServerImplPrivate
  {
  public:
    TransportServerImplPrivate(TransportServer* self, const qi::Url &url, EventLoop* ctx)
      : self(self)
      , context(ctx)
      , listenUrl(url)
    {}

    virtual ~TransportServerImplPrivate()
    {
    }

    virtual qi::Future<void> listen() = 0;
    virtual void close() = 0;
    virtual void destroy() = 0;

  public:
    TransportServer                        *self;
    boost::mutex                            mutexCallback;
    qi::EventLoop                          *context;
    qi::Url                                 listenUrl;
    qi::UrlVector                           _endpoints;
    qi::Promise<void>                       _connectionPromise;

  protected:
    TransportServerImplPrivate()
      : context(0)
      , listenUrl("")
    {};
  };

  class TransportServerPrivate
  {
  public:
    std::string                 _identityKey;
    std::string                 _identityCertificate;
    std::vector<TransportServerImplPrivate*> _impl;
  };

}

#endif  // _SRC_TRANSPORTSERVER_P_HPP_
