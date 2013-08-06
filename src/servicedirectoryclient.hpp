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
#include <qi/trackable.hpp>
#include <qimessaging/serviceinfo.hpp>
#include <qimessaging/session.hpp>
#include "remoteobject_p.hpp"

namespace qi {

  class TransportSocket;
  class ServiceDirectoryClient: public qi::Trackable<ServiceDirectoryClient> {
  public:
    ServiceDirectoryClient();
    ~ServiceDirectoryClient();

    //Socket API
    qi::FutureSync<void> connect(const qi::Url &serviceDirectoryURL);
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
    qi::Future< void >                     updateServiceInfo(const ServiceInfo &svcinfo);

    qi::Signal<>                                  connected;
    qi::Signal<std::string>                       disconnected;
    qi::Signal<unsigned int, std::string>         serviceAdded;
    qi::Signal<unsigned int, std::string>         serviceRemoved;

  private:
    //ServiceDirectory Interface
    void onServiceAdded(unsigned int idx, const std::string &name);
    void onServiceRemoved(unsigned int idx, const std::string &name);

    //TransportSocket Interface
    void onSocketConnected(qi::FutureSync<void> future, qi::Promise<void> prom);
    void onSocketDisconnected(std::string error);

    //RemoteObject Interface
    void onMetaObjectFetched(qi::Future<void> fut, qi::Promise<void> prom);

    //wait for serviceAdded/serviceRemoved are connected
    void onSDEventConnected(qi::Future<SignalLink> ret,
                            qi::Promise<void> fco,
                            bool isAdd);

  private:
    qi::TransportSocketPtr _sdSocket;
    unsigned int           _sdSocketDisconnectedSignalLink;
    qi::RemoteObject       _remoteObject;
    qi::AnyObject          _object;
    qi::SignalLink               _addSignalLink;
    qi::SignalLink               _removeSignalLink;
    boost::mutex           _mutex;
  };
}

#endif  // _SRC_SERVICEDIRECTORYCLIENT_HPP_
