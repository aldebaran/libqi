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

namespace qi {

  class ServiceDirectoryPrivate : public GenericObject
  {
  public:
    ServiceDirectoryPrivate();
    ~ServiceDirectoryPrivate();

    //TransportServer
    void onTransportServerNewConnection(TransportSocketPtr socket);

    //TransportSocket
    void onMessageReady(const qi::Message &msg, qi::TransportSocketPtr socket);
    void onSocketDisconnected(int error, TransportSocketPtr client);

    std::vector<ServiceInfo> services();
    ServiceInfo              service(const std::string &name);
    unsigned int             registerService(const ServiceInfo &svcinfo);
    void                     unregisterService(const unsigned int &idx);
    TransportSocketPtr       socket() { return currentSocket; }
    void                     serviceReady(const unsigned int &idx);

  public:
    qi::TransportServer                                    _server;
    std::map<unsigned int, ServiceInfo>                    pendingServices;
    std::map<unsigned int, ServiceInfo>                    connectedServices;
    std::map<std::string, unsigned int>                    nameToIdx;
    std::map<TransportSocketPtr, std::vector<unsigned int> > socketToIdx;
    unsigned int                                           servicesCount;
    TransportSocketPtr                                     currentSocket;
    std::set<TransportSocketPtr>                           _clients;
    boost::recursive_mutex                                 _clientsMutex;
  }; // !ServiceDirectoryPrivate

  template<> struct TypeDefaultClone<ServiceDirectoryPrivate>: public TypeNoClone<ServiceDirectoryPrivate>{};
}

#endif  // _SRC_SERVICEDIRECTORY_P_HPP_
