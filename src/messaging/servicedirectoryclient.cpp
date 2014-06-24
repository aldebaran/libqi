/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/
#include "servicedirectoryclient.hpp"
#include <qi/type/objecttypebuilder.hpp>
#include "servicedirectory_p.hpp"
#include "transportsocket.hpp"

qiLogCategory("qimessaging.servicedirectoryclient");

namespace qi {


  ServiceDirectoryClient::ServiceDirectoryClient()
    : Trackable<ServiceDirectoryClient>(this)
    , _remoteObject(qi::Message::Service_ServiceDirectory)
    , _addSignalLink(0)
    , _removeSignalLink(0)
    , _localSd(false)
  {
    _object = makeDynamicAnyObject(&_remoteObject, false);

    connected.setCallType(MetaCallType_Direct);
    disconnected.setCallType(MetaCallType_Direct);
  }


  ServiceDirectoryClient::~ServiceDirectoryClient()
  {
    destroy();
    close();
  }

  void ServiceDirectoryClient::onSDEventConnected(qi::Future<SignalLink> future,
    qi::Promise<void> promise, bool isAdd)
  {
    if (promise.future().isFinished()) {
      return;
    }
    if (future.hasError())
    {
      qi::Future<void> fdc = onSocketDisconnected(future.error());
      fdc.connect(&qi::Promise<void>::setError, promise, future.error());
      return;
    }
    bool ready = false;
    {
      boost::mutex::scoped_lock lock(_mutex);
      if (isAdd)
        _addSignalLink = future.value();
      else
        _removeSignalLink = future.value();
      ready = _addSignalLink && _removeSignalLink;
    }
    if (ready)
    {
      promise.setValue(0);
      connected();
    }
  }

  void ServiceDirectoryClient::onMetaObjectFetched(qi::Future<void> future, qi::Promise<void> promise) {
    if (future.hasError()) {
      qi::Future<void> fdc = onSocketDisconnected(future.error());
      fdc.connect(&qi::Promise<void>::setError, promise, future.error());
      return;
    }
    boost::function<void (unsigned int, std::string)> f;

    f = boost::bind<void>(&ServiceDirectoryClient::onServiceAdded, this, _1, _2);
    qi::Future<SignalLink> fut1 = _object.connect("serviceAdded", f);

    f = boost::bind<void>(&ServiceDirectoryClient::onServiceRemoved, this, _1, _2);
    qi::Future<SignalLink> fut2 = _object.connect("serviceRemoved", f);

    fut1.connect(&ServiceDirectoryClient::onSDEventConnected, this, _1, promise, true);
    fut2.connect(&ServiceDirectoryClient::onSDEventConnected, this, _1, promise, false);
  }

  void ServiceDirectoryClient::onSocketConnected(qi::FutureSync<void> future, qi::Promise<void> promise) {
    if (future.hasError()) {
      qi::Future<void> fdc = onSocketDisconnected(future.error());
      fdc.connect(&qi::Promise<void>::setError, promise, future.error());
      return;
    }
    qi::Future<void> fut = _remoteObject.fetchMetaObject();
    fut.connect(&ServiceDirectoryClient::onMetaObjectFetched, this, _1, promise);
  }

  //we ensure in that function that connect to all events are already setup when we said we are connect.
  //that way we can't be connected without being fully ready.
  qi::FutureSync<void> ServiceDirectoryClient::connect(const qi::Url &serviceDirectoryURL) {
    if (isConnected()) {
      const char* s = "Session is already connected";
      qiLogInfo() << s;
      return qi::makeFutureError<void>(s);
    }
    _sdSocket = qi::makeTransportSocket(serviceDirectoryURL.protocol());
    if (!_sdSocket)
      return qi::makeFutureError<void>(std::string("unrecognized protocol '") + serviceDirectoryURL.protocol() + "' in url '" + serviceDirectoryURL.str() + "'");
    _sdSocketDisconnectedSignalLink = _sdSocket->disconnected.connect(&ServiceDirectoryClient::onSocketDisconnected, this, _1);
    _remoteObject.setTransportSocket(_sdSocket);

    qi::Promise<void> promise;
    qi::Future<void> fut = _sdSocket->connect(serviceDirectoryURL);
    fut.connect(&ServiceDirectoryClient::onSocketConnected, this, _1, promise);
    return promise.future();
  }

  void ServiceDirectoryClient::setServiceDirectory(AnyObject serviceDirectoryService)
  {
    _object = serviceDirectoryService;
    _localSd = true;
    boost::function<void (unsigned int, std::string)> f;

    f = boost::bind<void>(&ServiceDirectoryClient::onServiceAdded, this, _1, _2);
    _addSignalLink  = _object.connect("serviceAdded", f);

    f = boost::bind<void>(&ServiceDirectoryClient::onServiceRemoved, this, _1, _2);
    _removeSignalLink = _object.connect("serviceRemoved", f);

    connected();
  }

  static void sharedPtrHolder(TransportSocketPtr* ptr)
  {
    delete ptr;
  }

  qi::FutureSync<void> ServiceDirectoryClient::close() {
    return onSocketDisconnected("User closed the connection");
  }

  bool                 ServiceDirectoryClient::isConnected() const {
    if (_localSd)
      return true;
    return _sdSocket == 0 ? false : _sdSocket->isConnected();
  }

  qi::Url              ServiceDirectoryClient::url() const {
    if (_localSd)
      throw std::runtime_error("Service directory is local, url() unknown.");
    if (!_sdSocket)
      throw std::runtime_error("Session disconnected");
    return _sdSocket->url();
  }

  void ServiceDirectoryClient::onServiceRemoved(unsigned int idx, const std::string &name) {
    qiLogVerbose() << "ServiceDirectoryClient: Service Removed #" << idx << ": " << name << std::endl;
    serviceRemoved(idx, name);
  }

  void ServiceDirectoryClient::onServiceAdded(unsigned int idx, const std::string &name) {
    qiLogVerbose() << "ServiceDirectoryClient: Service Added #" << idx << ": " << name << std::endl;
    serviceAdded(idx, name);
  }

  qi::FutureSync<void> ServiceDirectoryClient::onSocketDisconnected(std::string error) {
    qi::Future<void> fut;
    {
      qi::TransportSocketPtr socket;
      { // can't hold lock while disconnecting signals, so swap _sdSocket.
        boost::mutex::scoped_lock lock(_mutex);
        std::swap(socket, _sdSocket);
      }

      if (!socket)
        return qi::Future<void>(0);
      // We just manually triggered onSocketDisconnected, so unlink
      // from socket signal before disconnecting it.
      socket->disconnected.disconnect(_sdSocketDisconnectedSignalLink);
      // Manually trigger close on our remoteobject or it will be called
      // asynchronously from socket.disconnected signal, and we would need to
      // wait fo it.
      _remoteObject.close();
      fut = socket->disconnect();

      // Hold the socket shared ptr alive until the future returns.
      // otherwise, the destructor will block us until disconnect terminates
      // Nasty glitch: socket is reusing promises, so this future hook will stay
      // So pass shared pointer by pointer: that way a single delete statement
      // will end all copies.
      fut.connect(&sharedPtrHolder, new TransportSocketPtr(socket));
    }

    qi::SignalLink add=0, remove=0;
    qi::AnyObject object;
    {
      boost::mutex::scoped_lock lock(_mutex);
      std::swap(add, _addSignalLink);
      std::swap(remove, _removeSignalLink);
    }
    try {
      if (add != 0)
      {
        _object.disconnect(add);
      }
    } catch (std::runtime_error &e) {
      qiLogDebug() << "Cannot disconnect SDC::serviceAdded: " << e.what();
    }
    try {
      if (remove != 0)
      {
        _object.disconnect(remove);
      }
    } catch (std::runtime_error &e) {
        qiLogDebug() << "Cannot disconnect SDC::serviceRemoved: " << e.what();
    }
    disconnected(error);

    return fut;
  }

  TransportSocketPtr ServiceDirectoryClient::socket()
  {
    return _sdSocket;
  }

  bool ServiceDirectoryClient::isLocal()
  {
    return _localSd;
  }

  qi::Future< std::vector<ServiceInfo> > ServiceDirectoryClient::services() {
    return _object.async< std::vector<ServiceInfo> >("services");
  }

  qi::Future<ServiceInfo>              ServiceDirectoryClient::service(const std::string &name) {
    return _object.async< ServiceInfo >("service", name);
  }

  qi::Future<unsigned int>             ServiceDirectoryClient::registerService(const ServiceInfo &svcinfo) {
    return _object.async< unsigned int >("registerService", svcinfo);
  }

  qi::Future<void>                     ServiceDirectoryClient::unregisterService(const unsigned int &idx) {
    return _object.async<void>("unregisterService", idx);
  }

  qi::Future<void>                     ServiceDirectoryClient::serviceReady(const unsigned int &idx) {
    return _object.async<void>("serviceReady", idx);
  }

  qi::Future<void>                     ServiceDirectoryClient::updateServiceInfo(const ServiceInfo &svcinfo) {
    return _object.async<void>("updateServiceInfo", svcinfo);
  }

  qi::Future<std::string>              ServiceDirectoryClient::machineId() {
    return _object.async<std::string>("machineId");
  }

  qi::Future<qi::TransportSocketPtr>   ServiceDirectoryClient::_socketOfService(unsigned int id) {
    return _object.async<TransportSocketPtr>("_socketOfService", id);
  }
}
