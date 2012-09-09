/*
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2012 Aldebaran Robotics
*/

#pragma once
#ifndef _SRC_SESSION_P_HPP_
#define _SRC_SESSION_P_HPP_

#include <map>
#include <set>
#include <vector>
#include <boost/thread.hpp>
#include <qimessaging/transport_socket.hpp>
#include <qimessaging/transport_server.hpp>
#include <qimessaging/object.hpp>
#include <qimessaging/service_info.hpp>
#include <qimessaging/session.hpp>
#include "src/server_result.hpp"
#include "src/service_watcher.hpp"
#include "src/service_directory_client.hpp"
#include "src/session_server.hpp"
#include "src/session_service.hpp"
#include "src/session_services.hpp"

namespace qi {

  class SessionPrivate
  {
  public:
    SessionPrivate(qi::Session *session);
    virtual ~SessionPrivate();

  public:
    ServiceDirectoryClient _sdClient;
    Session_Server         _server;
    Session_Service        _serviceHandler;
    Session_Services       _servicesHandler;
    //Session_Client         _client;
    ServiceWatcher         _watcher;
  };
}


#endif  // _SRC_SESSION_P_HPP_
