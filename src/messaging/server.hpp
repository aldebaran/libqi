#pragma once
/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/

#ifndef _SRC_SERVER_HPP_
#define _SRC_SERVER_HPP_

#include <boost/thread/recursive_mutex.hpp>
#include <boost/noncopyable.hpp>
#include "boundobject.hpp"
#include "authprovider_p.hpp"

namespace qi {

  /**
   * Do all the plumbing between sockets and objects.
   *
   * Support a special kind of objects: (SocketObject, that are aware of Socket)
   *
   * Thread-safety warning: do not call listen and addSocketObject at the same time.
   *
   */
  class Server: public qi::Trackable<Server>, private boost::noncopyable {
  public:
    Server(bool enforceAuth = false);
    ~Server();

    //make the server listening
    qi::Future<void> listen(const qi::Url &address);
    void close();
    // Notify the server that it's up again.
    void open();
    bool setIdentity(const std::string& key, const std::string& crt);

    //Create a BoundObject
    bool addObject(unsigned int idx, qi::AnyObject obj);
    bool addObject(unsigned int idx, qi::BoundAnyObject obj);
    bool removeObject(unsigned int idx);

    std::vector<qi::Url> endpoints() const;

    void onTransportServerNewConnection(TransportSocketPtr socket, bool startReading);
    void setAuthProviderFactory(AuthProviderFactoryPtr factory);

  private:
    void setSocketObjectEndpoints();

  private:

    //TransportSocket
    void onSocketDisconnected(TransportSocketPtr socket, std::string error);
    void onMessageReady(const qi::Message &msg, TransportSocketPtr socket);
    void onMessageReadyNotAuthenticated(const qi::Message& msg, TransportSocketPtr socket, AuthProviderPtr authProvider,
                                        boost::shared_ptr<bool> first, boost::shared_ptr<SignalLink> signalLink);

  private:
    //bool: true if it's a socketobject
    using BoundAnyObjectMap = std::map<unsigned int, BoundAnyObject>;

    //ObjectList
    BoundAnyObjectMap                   _boundObjects;
    boost::mutex                        _boundObjectsMutex;

    boost::mutex                        _stateMutex;
    AuthProviderFactoryPtr              _authProviderFactory;
    bool                                _enforceAuth;
  public:
    TransportServer                     _server;
    bool                                _dying;
    qi::MetaCallType                    _defaultCallType;

  private:
    // The connection to the server's signal. Needs to be disconnected in the destructor.
    qi::SignalLink                      _newConnectionLink = qi::SignalBase::invalidSignalLink;

    struct SocketSubscriber
    {
      qi::SignalLink disconnected = qi::SignalBase::invalidSignalLink;
      qi::SignalLink messageReady = qi::SignalBase::invalidSignalLink;
    };
    std::map<TransportSocketPtr, SocketSubscriber> _subscribers;

    boost::recursive_mutex              _socketsMutex;

    void connectMessageReady(const TransportSocketPtr& socket);
    void disconnectSignals(const TransportSocketPtr& socket, const SocketSubscriber& subscriber);
  };
}


#endif  // _SRC_SERVER_HPP_
