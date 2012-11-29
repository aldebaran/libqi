#pragma once
/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/

#ifndef _SRC_SESSIONSERVICE_HPP_
#define _SRC_SESSIONSERVICE_HPP_

#include <qi/future.hpp>
#include <string>
#include <boost/thread/mutex.hpp>
#include <boost/thread/barrier.hpp>
#include <boost/enable_shared_from_this.hpp>
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

    qi::Promise<qi::ObjectPtr>    promise;
    std::string                   name;
    unsigned int                  serviceId;
    RemoteObject                 *remoteObject;
  };

  class Session_Service
  {
  public:
    Session_Service(TransportSocketCache *socketCache, ServiceDirectoryClient *sdClient, ObjectRegistrar *server);
    ~Session_Service();

    void close();

    qi::Future<qi::ObjectPtr> service(const std::string &service,
                                      Session::ServiceLocality locality);

    void removeService(const std::string &service);

  private:
    //FutureInterface
    void onServiceInfoResult(qi::Future<qi::ServiceInfo> value, long requestId);
    void onRemoteObjectComplete(qi::Future<void> value, long requestId);
    void onTransportSocketResult(qi::Future<TransportSocketPtr> value, long requestId);

    //ServiceDirectoryClient
    void onServiceRemoved(const unsigned int &index, const std::string &service);

    ServiceRequest *serviceRequest(long requestId);
    void            removeRequest(long requestId);

  private:
    boost::mutex                    _requestsMutex;
    std::map<int, ServiceRequest*> _requests;
    qi::Atomic<int>                _requestsIndex;

    //maintain a cache of remote object
    typedef std::map<std::string, ObjectPtr> RemoteObjectMap;
    RemoteObjectMap                 _remoteObjects;
    boost::mutex                    _remoteObjectsMutex;

  private:
    qi::SignalBase::Link    _linkServiceRemoved;
    TransportSocketCache   *_socketCache;
    ServiceDirectoryClient *_sdClient;  //not owned by us
    ObjectRegistrar        *_server;    //not owned by us
    boost::shared_ptr<Session_Service> _self;
    Promise<void>                      _destructionBarrier;
    friend inline void sessionServiceWaitBarrier(Session_Service* ptr);
  };

}
#endif  // _SRC_SESSIONSERVICE_HPP_
