#pragma once
/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/

#ifndef _SRC_TRANSPORTSERVER_HPP_
#define _SRC_TRANSPORTSERVER_HPP_

# include <qimessaging/api.hpp>
# include <qimessaging/url.hpp>
# include <qi/eventloop.hpp>
# include <qitype/signal.hpp>
# include <vector>


namespace qi {
  class TransportServer;

  class TransportServerImpl: public boost::enable_shared_from_this<TransportServerImpl>
  {
  public:
    TransportServerImpl(TransportServer* self, EventLoop* ctx)
      : self(self)
      , context(ctx)
    {}

    virtual qi::Future<void> listen(const qi::Url& listenUrl) = 0;
    virtual void close() = 0;

  public:
    TransportServer                        *self;
    boost::mutex                            mutexCallback;
    qi::EventLoop                          *context;
    boost::mutex                            _endpointsMutex;
    qi::UrlVector                           _endpoints;
    qi::Promise<void>                       _connectionPromise;
  };

  typedef boost::shared_ptr<TransportServerImpl> TransportServerImplPtr;


  class TransportSocket;
  typedef boost::shared_ptr<TransportSocket> TransportSocketPtr;

  class TransportServer
  {
    QI_DISALLOW_COPY_AND_ASSIGN(TransportServer);

  public:
    TransportServer();
    virtual ~TransportServer();

    qi::Future<void> listen(const qi::Url &url,
                            qi::EventLoop* ctx = qi::getDefaultNetworkEventLoop());
    bool setIdentity(const std::string& key, const std::string& crt);
    void close();

    std::vector<qi::Url> endpoints() const;

  public:
    /** Emitted each time a new connection happens. startReading must be
     * called on the socket
     */
    // C4251
    qi::Signal<TransportSocketPtr> newConnection;
    // C4251
    qi::Signal<int>                acceptError;
    qi::Signal<void>               endpointsChanged;
    std::string                           _identityKey;
    std::string                           _identityCertificate;
    std::vector<TransportServerImplPtr>   _impl;
  };

}

#endif  // _SRC_TRANSPORTSERVER_HPP_
