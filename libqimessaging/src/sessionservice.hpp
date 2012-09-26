#pragma once
/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/

#ifndef _SRC_SESSIONSERVICE_HPP_
#define _SRC_SESSIONSERVICE_HPP_

#include <qimessaging/future.hpp>
#include <string>
#include <boost/thread/mutex.hpp>
#include <qimessaging/session.hpp>
#include <qi/atomic.hpp>
#include "src/remoteobject_p.hpp"
#include "src/transportsocketcache.hpp"

namespace qi {

  class GenericObject;
  class ServiceDirectoryClient;
  class TransportSocketCache;
  class Session_Server;
  class ServerClient;

  struct ServiceRequest
  {

    ServiceRequest(const std::string &service = "", const std::string &protocol = "")
      : name(service)
      , serviceId(0)
      , protocol(protocol)
      , connected(false)
      , socket()
      , sclient(0)
    {}

    qi::Promise<qi::ObjectPtr>    promise;
    std::string                   name;
    unsigned int                  serviceId;
    std::string                   protocol;
    bool                          connected; // True if the service server was reached
    TransportSocketPtr            socket;
    ServerClient                 *sclient;
  };

  class Session_Service
  {
  public:
    Session_Service(TransportSocketCache *socketCache, ServiceDirectoryClient *sdClient, Session_Server *server)
      : _socketCache(socketCache)
      , _sdClient(sdClient)
      , _server(server)
    {}
    ~Session_Service();

    void close();

    qi::Future<qi::ObjectPtr> service(const std::string &service,
                                      Session::ServiceLocality locality,
                                      const std::string &protocol);

  protected:
    //FutureInterface
    void onServiceInfoResult(qi::Future<qi::ServiceInfo> value, long requestId);
    void onMetaObjectResult(qi::Future<qi::MetaObject> value, long requestId);
    void onTransportSocketResult(qi::Future<TransportSocketPtr> value, long requestId);

  protected:
    ServiceRequest *serviceRequest(long requestId);
    void            removeRequest(long requestId);

  protected:
    boost::mutex                    _requestsMutex;
    std::map<long, ServiceRequest*> _requests;
    qi::atomic<long>                _requestsIndex;

    //maintain a cache of remote object
    typedef std::map<std::string, ObjectPtr> RemoteObjectMap;
    RemoteObjectMap                 _remoteObjects;
    boost::mutex                    _remoteObjectsMutex;

  private:
    TransportSocketCache   *_socketCache;
    ServiceDirectoryClient *_sdClient;  //not owned by us
    Session_Server         *_server;    //not owned by us
  };

}
#endif  // _SRC_SESSIONSERVICE_HPP_
