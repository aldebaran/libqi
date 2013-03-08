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
# include "transportserver.hpp"

# include <string>
# include <queue>
# include <vector>

#include <boost/thread.hpp>

namespace qi {

  class TransportServerImpl: public boost::enable_shared_from_this<TransportServerImpl>
  {
  public:
    TransportServerImpl(TransportServer* self, EventLoop* ctx)
      : self(self)
      , context(ctx)
    {}

    virtual ~TransportServerImpl()
    {
    }

    virtual qi::Future<void> listen(const qi::Url& listenUrl) = 0;
    virtual void close() = 0;

  public:
    TransportServer                        *self;
    boost::mutex                            mutexCallback;
    qi::EventLoop                          *context;
    boost::mutex                            _endpointsMutex;
    qi::UrlVector                           _endpoints;
    qi::Promise<void>                       _connectionPromise;

  protected:
    TransportServerImpl()
      : context(0)
    {};
  };

  typedef boost::shared_ptr<TransportServerImpl> TransportServerImplPtr;

  class TransportServerPrivate
  {
  public:
    std::string                 _identityKey;
    std::string                 _identityCertificate;
    std::vector<TransportServerImplPtr> _impl;
  };

}

#endif  // _SRC_TRANSPORTSERVER_P_HPP_
