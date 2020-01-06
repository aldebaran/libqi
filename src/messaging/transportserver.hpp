#pragma once
/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/

#ifndef _SRC_TRANSPORTSERVER_HPP_
#define _SRC_TRANSPORTSERVER_HPP_

#include <utility>
# include <boost/noncopyable.hpp>
# include <qi/url.hpp>
# include <qi/eventloop.hpp>
# include <qi/signal.hpp>
# include <qi/messaging/messagesocket_fwd.hpp>
# include <qi/messaging/ssl/ssl.hpp>
# include <vector>
# include <boost/asio/ip/tcp.hpp>
# include <boost/asio/ssl/stream.hpp>


namespace qi {
  class TransportServer;

  class TransportServerImpl
  {
  public:
    TransportServerImpl(TransportServer* self, EventLoop* ctx)
      : self(self)
      , context(ctx)
    {}
    virtual ~TransportServerImpl() {}

    virtual qi::Future<void> listen(const qi::Url& listenUrl) = 0;
    virtual void close() = 0;

  public:
    TransportServer                        *self;
    boost::mutex                            mutexCallback;
    qi::EventLoop                          *context;
    boost::mutex                            _endpointsMutex;
    std::vector<qi::Uri>                    _endpoints;
    qi::Promise<void>                       _connectionPromise;
  };

  using TransportServerImplPtr = boost::shared_ptr<TransportServerImpl>;


  /**
   * @brief A socket acceptor (not a server, duh).
   * It automatically accepts incoming connections to the endpoint it
   * listens to. When a connection is accepted, a transport socket is created
   * and emitted through the newConnection signal.
   *
   * Each socket emitted is already connected to the client, but the messages
   * are not read from the socket at this level. It is the user responsibility
   * to manage the life cycle of the socket.
   */
  class TransportServer : private boost::noncopyable
  {
  public:
    TransportServer(ssl::ServerConfig sslConfig = {});
    virtual ~TransportServer();

    qi::Future<void> listen(const qi::Url &url,
                            qi::EventLoop* ctx = qi::getNetworkEventLoop());
    void close();

    /// Endpoints ordered by preference.
    /// @see `qi::isPreferredEndpoint`.
    std::vector<qi::Uri> endpoints() const;

  public:
    /** Emitted each time a new connection happens. startReading must be
     * called on the socket
     */
    // C4251
    qi::Signal<std::pair<MessageSocketPtr, Url>> newConnection;
    // C4251
    qi::Signal<int>                acceptError;
    qi::Signal<void>               endpointsChanged;
    const ssl::ServerConfig _sslConfig;
    std::vector<TransportServerImplPtr>   _impl;
    mutable boost::mutex                 _implMutex;
  };

}

#endif  // _SRC_TRANSPORTSERVER_HPP_
