#pragma once
/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/

#ifndef _SRC_SERVICEDIRECTORY_P_HPP_
#define _SRC_SERVICEDIRECTORY_P_HPP_

#include <qimessaging/transportserver.hpp>
#include <qimessaging/transportsocket.hpp>
#include <qimessaging/genericobject.hpp>
#include <set>
#include <boost/thread/recursive_mutex.hpp>
#include "objectregistrar.hpp"
#include "boundobject.hpp"

namespace qi {

  class ServiceDirectoryBoundObject : public ServiceBoundObject {
  public:
    ServiceDirectoryBoundObject();
    virtual ~ServiceDirectoryBoundObject();

    //TransportSocket
    virtual void onSocketDisconnected(TransportSocketPtr socket, int error);


  public:
    //Public Bound API
    std::vector<ServiceInfo> services();
    ServiceInfo              service(const std::string &name);
    unsigned int             registerService(const ServiceInfo &svcinfo);
    void                     unregisterService(const unsigned int &idx);
    void                     serviceReady(const unsigned int &idx);
    TransportSocketPtr       socket() { return currentSocket; }

  public:
    std::map<unsigned int, ServiceInfo>                       pendingServices;
    std::map<unsigned int, ServiceInfo>                       connectedServices;
    std::map<std::string, unsigned int>                       nameToIdx;
    std::map<TransportSocketPtr, std::vector<unsigned int> >  socketToIdx;
    unsigned int                                              servicesCount;
    TransportSocketPtr                                        currentSocket;
  }; // !ServiceDirectoryPrivate


  class ServiceDirectoryPrivate
  {
  public:
    ServiceDirectoryPrivate();
    ~ServiceDirectoryPrivate();

    BoundObjectPtr               _sdbo;
    Server                       _server;
  };

}

QI_TYPE_NOT_CLONABLE(qi::ServiceDirectoryBoundObject);

#endif  // _SRC_SERVICEDIRECTORY_P_HPP_
