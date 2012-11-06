/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/
#include "servicedirectoryclient.hpp"
#include <qitype/objecttypebuilder.hpp>
#include "servicedirectory_p.hpp"
#include "tcptransportsocket.hpp"

namespace qi {


  ServiceDirectoryClient::ServiceDirectoryClient()
    : _remoteObject(qi::Message::Service_ServiceDirectory)
    , _addLink(0)
    , _removeLink(0)
  {
    _object = makeDynamicObjectPtr(&_remoteObject, false);
  }

  ServiceDirectoryClient::~ServiceDirectoryClient()
  {
    close();
  }

 void ServiceDirectoryClient::onSDEventConnected(qi::Future<unsigned int> ret,
                                                 qi::Future<unsigned int> fadd,
                                                 qi::Future<unsigned int> frem,
                                                 qi::Promise<bool> fco)
  {
    if (!fadd.isReady() || !frem.isReady())
      return;
    if (fadd.hasError() || frem.hasError()) {
      std::string err;
      err = fadd.error();
      if (!err.empty() && frem.hasError())
        err += ". ";
      err += frem.error();
      fco.setError(err);
      return;
    }
    _addLink = fadd.value();
    _removeLink = frem.value();
    fco.setValue(true);
    connected();
  }

  void ServiceDirectoryClient::onMetaObjectFetched(qi::Future<void> future, qi::Promise<bool> promise) {
    if (future.hasError()) {
      promise.setError(future.error());
      return;
    }
    boost::function<void (unsigned int, std::string)> f;

    f = boost::bind<void>(&ServiceDirectoryClient::onServiceAdded, this, _1, _2);
    qi::Future<unsigned int> fut1 = _object->connect("serviceAdded", f);

    f = boost::bind<void>(&ServiceDirectoryClient::onServiceRemoved, this, _1, _2);
    qi::Future<unsigned int> fut2 = _object->connect("serviceRemoved", f);

    fut1.connect(boost::bind<void>(&ServiceDirectoryClient::onSDEventConnected, this, _1, fut1, fut2, promise));
    fut2.connect(boost::bind<void>(&ServiceDirectoryClient::onSDEventConnected, this, _1, fut1, fut2, promise));
  }

  void ServiceDirectoryClient::onSocketConnected(qi::FutureSync<bool> future, qi::Promise<bool> promise) {
    if (future.hasError()) {
      promise.setError(future.error());
      return;
    }
    if (future.value() == false) {
      promise.setValue(false);
      return;
    }
    qi::Future<void> fut = _remoteObject.fetchMetaObject();
    fut.connect(boost::bind<void>(&ServiceDirectoryClient::onMetaObjectFetched, this, _1, promise));
  }

  //we ensure in that function that connect to all events are already setup when we said we are connect.
  //that way we cant be connected without being fully ready.
  qi::FutureSync<bool> ServiceDirectoryClient::connect(const qi::Url &serviceDirectoryURL) {
    if (isConnected()) {
      qiLogInfo("qi.Session") << "Session is already connected";
      return qi::Future<bool>(false);
    }
    _sdSocket = qi::makeTransportSocket(serviceDirectoryURL.protocol());
    if (!_sdSocket)
      return qi::Future<bool>(false);
    _sdSocketDisconnectedLink = _sdSocket->disconnected.connect(boost::bind<void>(&ServiceDirectoryClient::onSocketDisconnected, this, _1));
    _remoteObject.setTransportSocket(_sdSocket);

    qi::Promise<bool> promise;
    qi::Future<bool> fut = _sdSocket->connect(serviceDirectoryURL);
    fut.connect(boost::bind<void>(&ServiceDirectoryClient::onSocketConnected, this, _1, promise));
    return promise.future();
  }

  static void sharedPtrHolder(TransportSocketPtr ptr)
  {
  }

  qi::FutureSync<void> ServiceDirectoryClient::close() {
    if (!_sdSocket)
      return qi::Future<void>(0);
    qi::Future<void> fut = _sdSocket->disconnect();
    // Hold the socket shared ptr alive until the future returns.
    // otherwise, the destructor will block us until disconnect terminates
    fut.connect(boost::bind(&sharedPtrHolder, _sdSocket));
    _sdSocket->disconnected.disconnect(_sdSocketDisconnectedLink);
    _sdSocket.reset();
    return fut;
  }

  bool                 ServiceDirectoryClient::isConnected() const {
    return _sdSocket == 0 ? false : _sdSocket->isConnected();
  }

  qi::Url              ServiceDirectoryClient::url() const {
    return _sdSocket->url();
  }

  void ServiceDirectoryClient::onServiceRemoved(unsigned int idx, const std::string &name) {
    qiLogVerbose("qi.ServiceDirectoryClient") << "ServiceDirectory: Service Removed #" << idx << ": " << name << std::endl;
    serviceRemoved(idx, name);
  }

  void ServiceDirectoryClient::onServiceAdded(unsigned int idx, const std::string &name) {
    qiLogVerbose("qi.ServiceDirectoryClient") << "ServiceDirectory: Service Added #" << idx << ": " << name << std::endl;
    serviceAdded(idx, name);
  }

  void ServiceDirectoryClient::onSocketDisconnected(int error) {
    disconnected(error);
    _object->disconnect(_addLink);
    _object->disconnect(_removeLink);
  }

  qi::Future< std::vector<ServiceInfo> > ServiceDirectoryClient::services() {
    return _object->call< std::vector<ServiceInfo> >("services");
  }

  qi::Future<ServiceInfo>              ServiceDirectoryClient::service(const std::string &name) {
    return _object->call< ServiceInfo >("service", name);
  }

  qi::Future<unsigned int>             ServiceDirectoryClient::registerService(const ServiceInfo &svcinfo) {
    return _object->call< unsigned int >("registerService", svcinfo);
  }

  qi::Future<void>                     ServiceDirectoryClient::unregisterService(const unsigned int &idx) {
    return _object->call<void>("unregisterService", idx);
  }

  qi::Future<void>                     ServiceDirectoryClient::serviceReady(const unsigned int &idx) {
    return _object->call<void>("serviceReady", idx);
  }


}
