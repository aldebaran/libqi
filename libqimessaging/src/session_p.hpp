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
#include <qimessaging/transportsocket.hpp>
#include <qimessaging/transportserver.hpp>
#include <qitype/genericobject.hpp>
#include <qimessaging/serviceinfo.hpp>
#include <qimessaging/session.hpp>
#include <qitype/signal.hpp>
#include "src/serverresult.hpp"
#include "src/servicewatcher.hpp"
#include "src/servicedirectoryclient.hpp"
#include "src/objectregistrar.hpp"
#include "src/sessionservice.hpp"
#include "src/sessionservices.hpp"
#include "src/transportsocketcache.hpp"

namespace qi {

  class SessionPrivate
  {
  public:
    SessionPrivate(qi::Session *session);
    virtual ~SessionPrivate();

    qi::FutureSync<bool> connect(const qi::Url &serviceDirectoryURL);
    qi::FutureSync<void> close();
    bool isConnected() const;

    void onConnected();
    void onDisconnected(int error);

  public:
    Session               *_self;

    //session have a transportsocket not belonging to transportsocketcache
    TransportSocketPtr     _sdSocket;
    unsigned int           _sdSocketConnectedLink;
    unsigned int           _sdSocketDisconnectedLink;

    ServiceDirectoryClient _sdClient;
    ObjectRegistrar        _serverObject;
    Session_Service        _serviceHandler;
    Session_Services       _servicesHandler;
    ServiceWatcher         _watcher;
    TransportSocketCache   _socketsCache;
  };
}


#endif  // _SRC_SESSION_P_HPP_
