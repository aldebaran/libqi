#pragma once
/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/

#ifndef _SRC_SERVICEDIRECTORYCLIENT_HPP_
#define _SRC_SERVICEDIRECTORYCLIENT_HPP_

#include <vector>
#include <qimessaging/serviceinfo.hpp>
#include <qimessaging/session.hpp>
#include "remoteobject_p.hpp"

namespace qi {

  class TransportSocket;
  class ServiceDirectoryClient {
  public:
    ServiceDirectoryClient();
    ~ServiceDirectoryClient();

    qi::FutureSync<bool> connect(const qi::Url &serviceDirectoryURL);
    bool isConnected() const;
    qi::Url url() const;
    qi::FutureSync<void> disconnect();

    qi::Future< std::vector<ServiceInfo> > services();
    qi::Future< ServiceInfo >              service(const std::string &name);
    qi::Future< unsigned int >             registerService(const ServiceInfo &svcinfo);
    qi::Future< void >                     unregisterService(const unsigned int &idx);
    qi::Future< void >                     serviceReady(const unsigned int &idx);

  protected:
    void onSocketDisconnected(TransportSocketPtr client, void *data);

  public:
    TransportSocketPtr   _socket;
  private:
    qi::RemoteObject     _remoteObject;
    qi::GenericObject           _object;
  };
}

#endif  // _SRC_SERVICEDIRECTORYCLIENT_HPP_
