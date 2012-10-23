#pragma once
/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/

#ifndef _SRC_SERVICEDIRECTORYCLIENT_HPP_
#define _SRC_SERVICEDIRECTORYCLIENT_HPP_

#include <vector>
#include <string>
#include <qitype/signal.hpp>
#include <qimessaging/serviceinfo.hpp>
#include <qimessaging/session.hpp>
#include "remoteobject_p.hpp"

namespace qi {

  class TransportSocket;
  class ServiceDirectoryClient {
  public:
    ServiceDirectoryClient();
    ~ServiceDirectoryClient();

    //Socket API
    qi::FutureSync<bool> connect(const qi::Url &serviceDirectoryURL);
    qi::FutureSync<void> close();
    bool                 isConnected() const;
    qi::Url              url() const;

  public:
    //Bound Interface
    qi::Future< std::vector<ServiceInfo> > services();
    qi::Future< ServiceInfo >              service(const std::string &name);
    qi::Future< unsigned int >             registerService(const ServiceInfo &svcinfo);
    qi::Future< void >                     unregisterService(const unsigned int &idx);
    qi::Future< void >                     serviceReady(const unsigned int &idx);

    qi::Signal<void ()>                                  connected;
    qi::Signal<void (int error)>                         disconnected;
    qi::Signal<void (unsigned int, std::string)>         serviceAdded;
    qi::Signal<void (unsigned int, std::string)>         serviceRemoved;

  private:

    //TransportSocket Interface
    void onSocketConnected();
    void onSocketDisconnected(int error);

  private:
    qi::TransportSocketPtr _sdSocket;
    unsigned int           _sdSocketConnectedLink;
    unsigned int           _sdSocketDisconnectedLink;
    qi::RemoteObject       _remoteObject;
    qi::ObjectPtr          _object;
  };
}

#endif  // _SRC_SERVICEDIRECTORYCLIENT_HPP_
