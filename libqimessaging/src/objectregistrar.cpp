/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/
#include <set>
#include <qimessaging/genericobject.hpp>
#include <qimessaging/transportserver.hpp>
#include <qimessaging/serviceinfo.hpp>
#include <qimessaging/objecttypebuilder.hpp>
#include "server.hpp"
#include "objectregistrar.hpp"
#include "serverresult.hpp"
#include "transportserver_p.hpp"
#include <qi/os.hpp>
#include <boost/thread/mutex.hpp>
#include "servicedirectoryclient.hpp"
#include "signal_p.hpp"

namespace qi {


  ObjectRegistrar::ObjectRegistrar(ServiceDirectoryClient *sdClient)
    : Server()
    , _dying(false)
    , _sdClient(sdClient)
  {
  }

  ObjectRegistrar::~ObjectRegistrar()
  {
    _dying = true;
  }

  void serviceReady(qi::Future<void> fut, qi::Promise<unsigned int> result, unsigned int idx) {
    if (fut.hasError()) {
      result.setError(fut.error());
      return;
    }
    result.setValue(idx);
  }

  void ObjectRegistrar::onFutureFinished(qi::Future<unsigned int> fut, long id, qi::Promise<unsigned int> result)
  {
    if (fut.hasError()) {
      result.setError(fut.error());
      return;
    }
    qi::ServiceInfo              si;
    RegisterServiceMap::iterator it;

    {
      boost::mutex::scoped_lock sl(_registerServiceRequestMutex);
      it = _registerServiceRequest.find(id);
      if (it != _registerServiceRequest.end())
        si = it->second.second;
      if (fut.hasError()) {
        _registerServiceRequest.erase(id);
        result.setError(fut.error());
        return;
      }
    }
    unsigned int idx = fut.value();
    si.setServiceId(idx);
    {
      boost::mutex::scoped_lock sl(_servicesMutex);
      BoundService bs;
      bs.id          = idx;
      bs.object      = it->second.first;
      bs.serviceInfo = si;
      bs.name        = si.name();
      BoundServiceMap::iterator it;
      it = _services.find(idx);
      if (it != _services.end()) {
        qiLogError("qi.server") << "A service is already registered with that id:" << idx;
        result.setError("Service already registered.");
        return;
      }
      _services[idx] = bs;
      //todo register the object on the server (find a better way)
      Server::addObject(idx, bs.object);
    }

    // ack the Service directory to tell that we are ready
    //TODO: async handle.
    qi::Future<void> fut2 = _sdClient->serviceReady(idx);
    fut2.connect(boost::bind(&serviceReady, _1, result, idx));

    {
      boost::mutex::scoped_lock sl(_serviceNameToIndexMutex);
      _serviceNameToIndex[si.name()] = idx;
    }
    {
      boost::mutex::scoped_lock sl(_registerServiceRequestMutex);
      _registerServiceRequest.erase(it);
    }

  }

  qi::Future<unsigned int> ObjectRegistrar::registerService(const std::string &name, qi::ObjectPtr obj)
  {
    if (Server::endpoints().empty()) {
      qiLogError("qimessaging.Server") << "Could not register service: " << name << " because the current server has not endpoint";
      return qi::Future<unsigned int>();
    }
    qi::ServiceInfo si;
    si.setName(name);
    si.setProcessId(qi::os::getpid());
    si.setMachineId(qi::os::getMachineId());

    std::vector<qi::Url>                 epsUrl = Server::endpoints();
    std::vector<qi::Url>::const_iterator epsUrlIt;
    std::vector<std::string> eps;
    for (epsUrlIt = epsUrl.begin(); epsUrlIt != epsUrl.end(); epsUrlIt++) {
      eps.push_back((*epsUrlIt).str());
    }
    si.setEndpoints(eps);

    long id = ++_registerServiceRequestIndex;
    {
      boost::mutex::scoped_lock sl(_registerServiceRequestMutex);
      _registerServiceRequest[id] = std::make_pair(obj, si);
    }

    qi::Promise<unsigned int> prom;
    qi::Future<unsigned int>  future;
    future = _sdClient->registerService(si);
    future.connect(boost::bind<void>(&ObjectRegistrar::onFutureFinished, this, _1, id, prom));

    return prom.future();
  };

  qi::Future<void> ObjectRegistrar::unregisterService(unsigned int idx)
  {
    qi::Future<void> future = _sdClient->unregisterService(idx);

    std::string name;
    {
      boost::mutex::scoped_lock sl(_servicesMutex);
      BoundServiceMap::iterator it = _services.find(idx);
      if (it != _services.end()) {
        name = it->second.name;
        _services.erase(it);
      } else {
        qiLogVerbose("qimessaging.Server") << "Can't find name associated to id:" << idx;
      }
      Server::removeObject(idx);
    }
    if (!name.empty())
    {
      boost::mutex::scoped_lock sl(_serviceNameToIndexMutex);
      ServiceNameToIndexMap::iterator it = _serviceNameToIndex.find(name);
      if (it != _serviceNameToIndex.end())
        _serviceNameToIndex.erase(it);
      else
        qiLogVerbose("qimessaging.Server") << "Can't find idx associated to name :" << name;
    }
    return future;
  }

  std::vector<qi::ServiceInfo> ObjectRegistrar::registeredServices() {
    std::vector<qi::ServiceInfo> ssi;
    {
      boost::mutex::scoped_lock sl(_servicesMutex);
      BoundServiceMap::iterator it = _services.begin();
      for (; it != _services.end(); ++it) {
        //drop 0 => it's the server itself.
        if (it->first == 0)
          continue;
        ssi.push_back(it->second.serviceInfo);
      }
    }
    return ssi;
  }

  //return 0 on error (0 is the server which have no name)
  unsigned int ObjectRegistrar::objectId(const std::string &name)
  {
    {
      boost::mutex::scoped_lock sl(_serviceNameToIndexMutex);
      ServiceNameToIndexMap::iterator it;
      it = _serviceNameToIndex.find(name);
      if (it != _serviceNameToIndex.end())
        return it->second;
    }
    return 0;
  }

  qi::ServiceInfo ObjectRegistrar::registeredService(const std::string &service) {
    unsigned int serviceId = objectId(service);

    if (!serviceId)
      return qi::ServiceInfo();

    {
      boost::mutex::scoped_lock sl(_servicesMutex);
      BoundServiceMap::iterator it = _services.find(serviceId);
      if (it != _services.end())
        return it->second.serviceInfo;
    }
    return qi::ServiceInfo();
  }

  qi::ObjectPtr ObjectRegistrar::registeredServiceObject(const std::string &service) {
    unsigned int serviceId = objectId(service);
    if (!serviceId)
      return qi::ObjectPtr();

    {
      boost::mutex::scoped_lock sl(_servicesMutex);
      BoundServiceMap::iterator it = _services.find(serviceId);
      if (it != _services.end())
        return it->second.object;
    }
    return ObjectPtr();
  }

}
