/*
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2012 Aldebaran Robotics
*/

#include "service_watcher.hpp"
#include <qimessaging/service_info.hpp>
#include <qimessaging/session.hpp>

namespace qi {

  ServiceWatcher::ServiceWatcher(qi::Session *session)
    : _session(session)
  {
  }

  void ServiceWatcher::onServiceRegistered(Session *, const std::string &serviceName)
  {
    {
      boost::mutex::scoped_lock _sl(_watchedServicesMutex);
      std::map< std::string, std::pair< int, qi::Promise<void> > >::iterator it;
      it = _watchedServices.find(serviceName);
      if (it != _watchedServices.end())
        it->second.second.setValue(0);
    }
  }

  void ServiceWatcher::onServiceUnregistered(Session *, const std::string &serviceName)
  {
    {
      boost::mutex::scoped_lock _sl(_watchedServicesMutex);
      std::map< std::string, std::pair< int, qi::Promise<void> > >::iterator it;
      it = _watchedServices.find(serviceName);
      if (it != _watchedServices.end())
        it->second.second.setError(0);
    }
  }

  bool ServiceWatcher::waitForServiceReady(const std::string &service, int msecs) {

    qi::Future< std::vector<ServiceInfo> > svs;
    std::vector<ServiceInfo>::iterator     it;
    qi::Promise<void>                      prom;
    qi::Future<void>                       fut;

    //register a watcher for the service
    {
      boost::mutex::scoped_lock _sl(_watchedServicesMutex);
      std::map< std::string, std::pair< int, qi::Promise<void> > >::iterator it;
      it = _watchedServices.find(service);
      if (it == _watchedServices.end()) {
        fut = prom.future();
        it = _watchedServices.insert(std::make_pair(service, std::make_pair(0, prom))).first;
      } else {
        fut = it->second.second.future();
      }
      it->second.first += 1;
    }

    svs = _session->services();
    svs.wait(msecs);
    if (!svs.isReady()) {
      qiLogVerbose("qi.ServiceWatcher") << "waitForServiceReady failed because session.services did not return.";
      return false;
    }
    for (it = svs.value().begin(); it != svs.value().end(); ++it) {
      if (it->name() == service)
        return true;
    }
    fut.wait(msecs);
    //no error mean service found
    bool found = (fut.isReady() && fut.hasError() == 0);

    {
      boost::mutex::scoped_lock _sl(_watchedServicesMutex);
      std::map< std::string, std::pair< int, qi::Promise<void> > >::iterator it;
      it = _watchedServices.find(service);
      if (it != _watchedServices.end()) {
        it->second.first -= 1;
      }
      if (it->second.first == 0)
        _watchedServices.erase(it);
    }
    return found;
  }

}
