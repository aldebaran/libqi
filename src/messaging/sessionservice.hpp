#pragma once
/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/

#ifndef _SRC_SESSIONSERVICE_HPP_
#define _SRC_SESSIONSERVICE_HPP_

#include <qi/future.hpp>
#include <qi/trackable.hpp>
#include <string>
#include <boost/thread/mutex.hpp>
#include <qi/session.hpp>
#include <qi/atomic.hpp>
#include "remoteobject_p.hpp"
#include "transportsocketcache.hpp"
#include "messagesocket.hpp"
#include "clientauthenticator_p.hpp"

namespace qi {

  class GenericObject;
  class ServiceDirectoryClient;
  class TransportSocketCache;
  class ObjectRegistrar;
  class ServerClient;

  struct ServiceRequest
  {
    explicit ServiceRequest(const std::string &service = "")
    {
      serviceInfo.setName(service);
    }

    ServiceRequest(const ServiceRequest&) = delete;
    ServiceRequest& operator=(const ServiceRequest&) = delete;

    ServiceRequest(ServiceRequest&&) = default;
    ServiceRequest& operator=(ServiceRequest&&) = default;

    qi::Promise<qi::AnyObject>    promise;
    ServiceInfo                   serviceInfo;
    RemoteObjectPtr               remoteObject;
  };

  class Session_Service: public qi::Trackable<Session_Service>
  {
  public:
    Session_Service(TransportSocketCache* socketCache,
                    ServiceDirectoryClient* sdClient,
                    ObjectRegistrar* server,
                    boost::optional<ClientAuthenticatorFactoryPtr> clientAuthenticatorFactory);
    ~Session_Service();

    void close();

    qi::Future<qi::AnyObject> service(const std::string &service,
                                      const std::string &protocol);

    void addService(const std::string& name, const qi::AnyObject &obj);
    void removeService(const std::string &service);

    void setClientAuthenticatorFactory(ClientAuthenticatorFactoryPtr factory);

  private:
    //FutureInterface
    void onRemoteObjectComplete(qi::Future<void> value, long requestId);
    void onTransportSocketResult(qi::Future<MessageSocketPtr> value, long requestId);

    //ServiceDirectoryClient
    void onAuthentication(const MessageSocket::SocketEventData& data, long requestId, MessageSocketPtr socket, ClientAuthenticatorPtr auth, boost::optional<SignalLink&> old);

    ServiceRequest *serviceRequest(long requestId);
    void            removeRequest(long requestId);

  private:
    boost::recursive_mutex         _requestsMutex;
    std::map<int, std::unique_ptr<ServiceRequest>> _requests;
    qi::Atomic<int>                _requestsIndex;

    //maintain a cache of remote object
    using RemoteObjectMap = std::map<std::string, AnyObject>;
    RemoteObjectMap                 _remoteObjects;
    boost::recursive_mutex          _remoteObjectsMutex;

  private:
    // RAII type to ensure a promise is set in error by default.
    //
    // If the optional promise is not set, or the caller deactivated the set,
    // or the promise is already set, the call will do nothing.
    //
    // The service request is also removed from the pending requests.
    struct SetPromiseInError
    {
      Session_Service& session;
      boost::optional<Promise<AnyObject>>& promise;
      bool& mustSetPromise;
      long requestId;

      void operator()();
    };

    TransportSocketCache   *_socketCache;
    ServiceDirectoryClient *_sdClient;  //not owned by us
    ObjectRegistrar        *_server;    //not owned by us
    ClientAuthenticatorFactoryPtr      _authFactory;
    bool _enforceAuth;
    friend inline void sessionServiceWaitBarrier(Session_Service* ptr);

    void setErrorAndRemoveRequest(
      Promise<AnyObject> p, const std::string& message, long requestId);
  };

}
#endif  // _SRC_SESSIONSERVICE_HPP_

