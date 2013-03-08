#pragma once
/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/

#ifndef _SRC_SESSION_P_HPP_
#define _SRC_SESSION_P_HPP_

#include <map>
#include <set>
#include <vector>
#include <boost/thread.hpp>
#include "transportsocket.hpp"
#include "transportserver.hpp"
#include <qitype/genericobject.hpp>
#include <qimessaging/serviceinfo.hpp>
#include <qimessaging/session.hpp>
#include <qitype/signal.hpp>
#include "serverresult.hpp"
#include "servicedirectoryclient.hpp"
#include "objectregistrar.hpp"
#include "sessionservice.hpp"
#include "sessionservices.hpp"
#include "transportsocketcache.hpp"

namespace qi {

  class SessionPrivate
  {
  public:
    SessionPrivate(qi::Session *session);
    virtual ~SessionPrivate();

    qi::FutureSync<void> connect(const qi::Url &serviceDirectoryURL);
    qi::FutureSync<void> close();
    bool isConnected() const;

    void onConnected();
    void onDisconnected(int error);
    void onServiceAdded(unsigned int idx, const std::string &name);
    void onServiceRemoved(unsigned int idx, const std::string &name);

  public:
    Session               *_self;

    //ServiceDirectoryClient have a transportsocket not belonging to transportsocketcache
    ServiceDirectoryClient _sdClient;
    unsigned int           _sdClientConnectedLink;
    unsigned int           _sdClientDisconnectedLink;
    unsigned int           _sdClientServiceAddedLink;
    unsigned int           _sdClientServiceRemovedLink;

    ObjectRegistrar        _serverObject;
    Session_Service        _serviceHandler;
    Session_Services       _servicesHandler;
    TransportSocketCache   _socketsCache;
  };
}


#endif  // _SRC_SESSION_P_HPP_
