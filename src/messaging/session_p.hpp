#pragma once
/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/

#ifndef _SRC_SESSION_P_HPP_
#define _SRC_SESSION_P_HPP_

#include <qi/session.hpp>
#include "servicedirectoryclient.hpp"
#include "objectregistrar.hpp"
#include "sessionservice.hpp"
#include "sessionservices.hpp"
#include "transportsocketcache.hpp"
#include "servicedirectory.hpp"

namespace qi {

  class SessionPrivate : public qi::Trackable<SessionPrivate>
  {
  public:
    SessionPrivate(qi::Session *session);
    virtual ~SessionPrivate();

    qi::FutureSync<void> connect(const qi::Url &serviceDirectoryURL);
    qi::FutureSync<void> listenStandalone(const qi::Url& listenUrl);
    qi::FutureSync<void> close();
    bool isConnected() const;

    void onDisconnected(std::string error);
    void onServiceAdded(unsigned int idx, const std::string &name);
    void onServiceRemoved(unsigned int idx, const std::string &name);

  public:
    void listenStandaloneCont(qi::Promise<void> p, qi::Future<void> f);
    // internal, add sd socket to socket cache
    void addSdSocketToCache(Future<void>, const qi::Url& url, qi::Promise<void> p);

    //ServiceDirectoryClient have a transportsocket not belonging to transportsocketcache
    ServiceDirectoryClient _sdClient;

    ObjectRegistrar        _serverObject;
    Session_Service        _serviceHandler;
    Session_Services       _servicesHandler;
    Session_SD             _sd;
    TransportSocketCache   _socketsCache;
  };
}


#endif  // _SRC_SESSION_P_HPP_
