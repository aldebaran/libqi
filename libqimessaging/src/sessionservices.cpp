/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/
#include "sessionservices.hpp"
#include "servicedirectoryclient.hpp"
#include "sessionserver.hpp"

namespace qi {

  void Session_Services::onFutureFailed(const std::string &error, void *data) {
    ServicesRequest *rq = request(data);
    if (!rq)
      return;
    rq->promise.setError(error);
    removeRequest(data);
  }

  void Session_Services::onFutureFinished(const std::vector<qi::ServiceInfo> &value, void *data) {
    ServicesRequest *rq = request(data);
    if (!rq)
      return;
    std::vector<qi::ServiceInfo> result;
    if (rq->locality == qi::Session::ServiceLocality_All)
    {
      result = _server->registeredServices();
    }
    result.insert(result.end(), value.begin(), value.end());
    rq->promise.setValue(result);
    removeRequest(data);
  }

  ServicesRequest *Session_Services::request(void *data) {
    {
      boost::mutex::scoped_lock sl(_requestMutex);
      std::map<long, ServicesRequest*>::iterator it;
      it = _request.find(reinterpret_cast<long>(data));
      if (it != _request.end())
        return it->second;
    }
    return 0;
  }

  void Session_Services::removeRequest(void *data) {
    {
      boost::mutex::scoped_lock sl(_requestMutex);
      std::map<long, ServicesRequest*>::iterator it;
      it = _request.find(reinterpret_cast<long>(data));
      if (it != _request.end()) {
        delete it->second;
        _request.erase(it);
      }
    }
  }

  qi::Future< std::vector<qi::ServiceInfo> > Session_Services::services(Session::ServiceLocality locality)
  {
    long requestId = ++_requestIndex;
    qi::Promise<std::vector<qi::ServiceInfo> > promise;

    if (locality == Session::ServiceLocality_Local) {
      promise.setValue(_server->registeredServices());
      return promise.future();
    }

    qi::Future< std::vector<qi::ServiceInfo> > fut = _sdClient->services();
    {
      boost::mutex::scoped_lock sl(_requestMutex);
      _request[requestId] = new ServicesRequest(promise, locality);
    }
    fut.addCallbacks(this, reinterpret_cast<void *>(requestId));
    return promise.future();
  }

}
