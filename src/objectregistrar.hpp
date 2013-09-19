#pragma once
/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/

#ifndef _SRC_OBJECTREGISTRAR_HPP_
#define _SRC_OBJECTREGISTRAR_HPP_

#include <string>
#include <set>
#include <boost/thread/recursive_mutex.hpp>
#include <qimessaging/api.hpp>
#include <qimessaging/session.hpp>
#include <qi/atomic.hpp>
#include "server.hpp"

namespace qi {

  class GenericObject;
  class ServiceDirectoryClient;

  //hold all information about a service
  struct BoundService {
    std::string   name;
    qi::AnyObject object;
    unsigned int  id;
    ServiceInfo   serviceInfo;
  };


  /** the Server 0 object
   * handle metaObject request, event connection/disconnection
   */
  class ObjectRegistrar : private Server {

  public:
    ObjectRegistrar(ServiceDirectoryClient *sdClient, Session *session);
    virtual ~ObjectRegistrar();

    //register/unregister services
    qi::Future<unsigned int>     registerService(const std::string &name, qi::AnyObject obj);
    qi::Future<void>             unregisterService(unsigned int idx);
    void                         updateServiceInfo();

    //list services
    std::vector<qi::ServiceInfo>  registeredServices();
    qi::ServiceInfo               registeredService(const std::string &service);
    qi::AnyObject                 registeredServiceObject(const std::string &service);

    using Server::close;
    using Server::listen;
    using Server::setIdentity;
    using Server::endpoints;

  private:
    //throw on error
    qi::AnyObject  object(unsigned int serviceId);
    //0 on error
    unsigned int   objectId(const std::string &name);

  private:
    //Future
    void onFutureFinished(qi::Future<unsigned int> future, long id, qi::Promise<unsigned int> result);

  private:
    typedef std::map<unsigned int, BoundService>                       BoundServiceMap;
    typedef std::map<int, std::pair<qi::AnyObject, qi::ServiceInfo> > RegisterServiceMap;
    typedef std::map<std::string, unsigned int>                        ServiceNameToIndexMap;

  public:
    BoundServiceMap                     _services;
    boost::mutex                        _servicesMutex;

    ServiceNameToIndexMap               _serviceNameToIndex;
    boost::mutex                        _serviceNameToIndexMutex;


    //used by registerService
    RegisterServiceMap                  _registerServiceRequest;
    qi::Atomic<int>                    _registerServiceRequestIndex;
    boost::mutex                        _registerServiceRequestMutex;

    bool                                _dying;
    ServiceDirectoryClient             *_sdClient;
    Session                            *_session;
    const std::string                   _id;
  };

}

QI_TYPE_NOT_CONSTRUCTIBLE(qi::ObjectRegistrar);


#endif  // _SRC_OBJECTREGISTRAR_HPP_
