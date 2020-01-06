#pragma once
/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/

#ifndef _SRC_SERVICEDIRECTORYCLIENT_HPP_
#define _SRC_SERVICEDIRECTORYCLIENT_HPP_

#include <vector>
#include <string>
#include <qi/signal.hpp>
#include <qi/trackable.hpp>
#include <qi/messaging/serviceinfo.hpp>
#include <qi/messaging/messagesocket_fwd.hpp>
#include <qi/session.hpp>
#include "remoteobject_p.hpp"
#include "clientauthenticator_p.hpp"
#include "messagesocket.hpp"

namespace qi {

  class ServiceDirectoryClient: public qi::Trackable<ServiceDirectoryClient> {
  public:
    ServiceDirectoryClient(ssl::ClientConfig sslConfig,
                           boost::optional<ClientAuthenticatorFactoryPtr> clientAuthFactory = {});
    ~ServiceDirectoryClient();

    // Connect to a remote service directory service
    qi::FutureSync<void> connect(const qi::Url &serviceDirectoryURL);
    // Setup with an existing object on the service directory
    void setServiceDirectory(AnyObject serviceDirectoryService);
    qi::FutureSync<void> close();
    bool                 isConnected() const;
    qi::Url              url() const;

    qi::AnyObject        object() { return _object; }
    void                 setClientAuthenticatorFactory(ClientAuthenticatorFactoryPtr);

  public:
    //Bound Interface
    qi::Future< std::vector<ServiceInfo> > services();
    qi::Future< ServiceInfo >              service(const std::string &name);
    qi::Future< unsigned int >             registerService(const ServiceInfo &svcinfo);
    qi::Future< void >                     unregisterService(const unsigned int &idx);
    qi::Future< void >                     serviceReady(const unsigned int &idx);
    qi::Future< void >                     updateServiceInfo(const ServiceInfo &svcinfo);
    qi::Future< std::string >              machineId();
    /// if isLocal() only, return socket holding given service id
    qi::Future<qi::MessageSocketPtr>     _socketOfService(unsigned int serviceId);

    qi::Signal<>                                  connected;
    qi::Signal<std::string>                       disconnected;
    qi::Signal<unsigned int, std::string>         serviceAdded;
    qi::Signal<unsigned int, std::string>         serviceRemoved;

    MessageSocketPtr socket();
    // True if ServiceDirectory is local
    bool isLocal();
  private:
    //ServiceDirectory Interface
    void onServiceAdded(unsigned int idx, const std::string &name);
    void onServiceRemoved(unsigned int idx, const std::string &name);

    //MessageSocket Interface
    void onSocketConnected(MessageSocketPtr socket,
                           qi::Future<void> future,
                           qi::Promise<void> prom);
    qi::FutureSync<void> onSocketFailure(MessageSocketPtr socket,
                                         std::string error,
                                         bool sendSignalDisconnected = true);

    //RemoteObject Interface
    void onMetaObjectFetched(MessageSocketPtr socket, qi::Future<void> fut, qi::Promise<void> prom);

    void onAuthentication(MessageSocketPtr socket,
                          const MessageSocket::SocketEventData& data,
                          qi::Promise<void> prom,
                          ClientAuthenticatorPtr authenticator);

    //wait for serviceAdded/serviceRemoved are connected
    void onSDEventConnected(qi::Future<SignalLink> ret,
                            qi::Promise<void> fco,
                            bool isAdd);

    bool isPreviousSdSocket(const MessageSocketPtr& socket);
    void cleanupPreviousSdSocket(const MessageSocketPtr& socket,
                                 qi::Promise<void> connectionPromise) const;

    Future<void> closeImpl(const std::string& reason, bool sendSignalDisconnected);

  private:
    struct StateData
    {
      StateData() = default;
      StateData(StateData&& o);
      StateData& operator=(StateData&& o);

      MessageSocketPtr sdSocket;
      SignalLink sdSocketDisconnectedSignalLink = SignalBase::invalidSignalLink;
      SignalLink sdSocketSocketEventSignalLink = SignalBase::invalidSignalLink;
      SignalLink addSignalLink = SignalBase::invalidSignalLink;
      SignalLink removeSignalLink = SignalBase::invalidSignalLink;
      bool localSd = false; // true if sd is local (no socket)
      Url url;
    };

    StateData _stateData; // protected by _mutex
    RemoteObjectPtr _remoteObject; // Must be shared to allow using weakPtr in other systems referring it.
    // _object is a remote object of serviceDirectory
    AnyObject _object;
    ClientAuthenticatorFactoryPtr _authFactory;
    const ssl::ClientConfig _sslConfig;
    bool _enforceAuth;
    mutable boost::mutex _mutex;
  };
}

#endif  // _SRC_SERVICEDIRECTORYCLIENT_HPP_
