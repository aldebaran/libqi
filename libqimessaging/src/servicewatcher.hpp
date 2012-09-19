#pragma once
/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/

#ifndef _SRC_SERVICEWATCHER_HPP_
#define _SRC_SERVICEWATCHER_HPP_

#include <boost/thread/mutex.hpp>
#include <map>
#include <string>
#include <qimessaging/future.hpp>

namespace qi {


  class Session;
  /** Wait for a service to be ready
   */
  class ServiceWatcher {
  public:
    ServiceWatcher(qi::Session *session);

    bool waitForServiceReady(const std::string &service, int msecs);

    void onServiceRegistered(qi::Session *, const std::string &serviceName);
    void onServiceUnregistered(qi::Session *, const std::string &serviceName);

  protected:
    boost::mutex                                                _watchedServicesMutex;
    std::map< std::string, std::pair<int, qi::Promise<void> > > _watchedServices;
    qi::Session*                                                _session;
  };

}

#endif  // _SRC_SERVICEWATCHER_HPP_
