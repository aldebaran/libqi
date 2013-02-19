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
    //ServiceDirectory Interface
    void onServiceAdded(unsigned int idx, const std::string &name);
    void onServiceRemoved(unsigned int idx, const std::string &name);

    //TransportSocket Interface
    void onSocketConnected(qi::FutureSync<bool> future, qi::Promise<bool> prom);
    void onSocketDisconnected(int error);

    //RemoteObject Interface
    void onMetaObjectFetched(qi::Future<void> fut, qi::Promise<bool> prom);

    //wait for serviceAdded/serviceRemoved are connected
    void onSDEventConnected(qi::Future<Link> ret,
                            qi::Promise<bool> fco,
                            bool isAdd);

  private:
    qi::TransportSocketPtr _sdSocket;
    unsigned int           _sdSocketDisconnectedLink;
    qi::RemoteObject       _remoteObject;
    qi::ObjectPtr          _object;
    unsigned int           _addLink;
    unsigned int           _removeLink;
  };
}

#endif  // _SRC_SERVICEDIRECTORYCLIENT_HPP_
