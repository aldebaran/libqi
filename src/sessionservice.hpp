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
#include <qimessaging/session.hpp>
#include <qi/atomic.hpp>
#include "remoteobject_p.hpp"
#include "transportsocketcache.hpp"

namespace qi {

  class GenericObject;
  class ServiceDirectoryClient;
  class TransportSocketCache;
  class ObjectRegistrar;
  class ServerClient;

  struct ServiceRequest
  {

    ServiceRequest(const std::string &service = "")
      : name(service)
      , serviceId(0)
      , remoteObject(0)
    {}

    qi::Promise<qi::AnyObject>    promise;
    std::string                   name;
    unsigned int                  serviceId;
    RemoteObject                 *remoteObject;
  };

  class Session_Service: public qi::Trackable<Session_Service>
  {
  public:
    Session_Service(TransportSocketCache *socketCache, ServiceDirectoryClient *sdClient, ObjectRegistrar *server);
    ~Session_Service();

    void close();

    qi::Future<qi::AnyObject> service(const std::string &service,
                                      const std::string &protocol);

    void addService(const std::string& name, const qi::AnyObject &obj);
    void removeService(const std::string &service);

  private:
    //FutureInterface
    void onServiceInfoResult(qi::Future<qi::ServiceInfo> value, long requestId, std::string protocol);
    void onRemoteObjectComplete(qi::Future<void> value, long requestId);
    void onTransportSocketResult(qi::Future<TransportSocketPtr> value, long requestId);

    //ServiceDirectoryClient
    void onServiceRemoved(const unsigned int &index, const std::string &service);

    ServiceRequest *serviceRequest(long requestId);
    void            removeRequest(long requestId);

  private:
    boost::recursive_mutex         _requestsMutex;
    std::map<int, ServiceRequest*> _requests;
    qi::Atomic<int>                _requestsIndex;

    //maintain a cache of remote object
    typedef std::map<std::string, AnyObject> RemoteObjectMap;
    RemoteObjectMap                 _remoteObjects;
    boost::recursive_mutex          _remoteObjectsMutex;

  private:
    qi::SignalLink    _linkServiceRemoved;
    TransportSocketCache   *_socketCache;
    ServiceDirectoryClient *_sdClient;  //not owned by us
    ObjectRegistrar        *_server;    //not owned by us
    boost::shared_ptr<Session_Service> _self;
    Promise<void>                      _destructionBarrier;
    friend inline void sessionServiceWaitBarrier(Session_Service* ptr);
    friend inline void onServiceInfoResultIfExists(Session_Service* s, qi::Future<qi::ServiceInfo> f,
    long requestId, std::string protocol, boost::weak_ptr<Session_Service> self);
  };

}
#endif  // _SRC_SESSIONSERVICE_HPP_
