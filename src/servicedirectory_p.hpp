#pragma once
/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/

#ifndef _SRC_SERVICEDIRECTORY_P_HPP_
#define _SRC_SERVICEDIRECTORY_P_HPP_

#include "transportsocket.hpp"
#include <boost/thread/recursive_mutex.hpp>
#include "boundobject.hpp"
#include "server.hpp"

namespace qi {

  class ServiceDirectoryBoundObject : public ServiceBoundObject {
  public:
    ServiceDirectoryBoundObject();
    virtual ~ServiceDirectoryBoundObject();

    //TransportSocket
    virtual void onSocketDisconnected(TransportSocketPtr socket, std::string error);


  public:
    //Public Bound API
    std::vector<ServiceInfo> services();
    ServiceInfo              service(const std::string &name);
    unsigned int             registerService(const ServiceInfo &svcinfo);
    void                     unregisterService(const unsigned int &idx);
    void                     serviceReady(const unsigned int &idx);
    void                     updateServiceInfo(const ServiceInfo &svcinfo);

    qi::Signal<unsigned int, std::string>  serviceAdded;
    qi::Signal<unsigned int, std::string>  serviceRemoved;

  public:
    std::map<unsigned int, ServiceInfo>                       pendingServices;
    std::map<unsigned int, ServiceInfo>                       connectedServices;
    std::map<std::string, unsigned int>                       nameToIdx;
    std::map<TransportSocketPtr, std::vector<unsigned int> >  socketToIdx;
    unsigned int                                              servicesCount;
    /* Our methods can be invoked from remote, and from socket callbacks,
    * so thread-safeness is required.
    */
    boost::recursive_mutex                                    mutex;
  }; // !ServiceDirectoryPrivate


  class ServiceDirectoryPrivate
  {
  public:
    ServiceDirectoryPrivate();
    ~ServiceDirectoryPrivate();

    void                         updateServiceInfo();

    BoundObjectPtr               _sdbo;
    Server                       _server;
  };

}

#endif  // _SRC_SERVICEDIRECTORY_P_HPP_
