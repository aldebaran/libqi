/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/
#include "servicedirectoryclient.hpp"
#include <qitype/objecttypebuilder.hpp>
#include "servicedirectory_p.hpp"
#include "transportsocket.hpp"

qiLogCategory("qimessaging.servicedirectoryclient");

namespace qi {


  ServiceDirectoryClient::ServiceDirectoryClient()
    : Trackable<ServiceDirectoryClient>(this)
    , _remoteObject(qi::Message::Service_ServiceDirectory)
    , _addSignalLink(0)
    , _removeSignalLink(0)
  {
    _object = makeDynamicAnyObject(&_remoteObject, false);
  }

  ServiceDirectoryClient::~ServiceDirectoryClient()
  {
    destroy();
    close();
  }

 void ServiceDirectoryClient::onSDEventConnected(qi::Future<SignalLink> ret,
   qi::Promise<void> fco, bool isAdd)
 {
   if (fco.future().isFinished()) {
     return;
   }
   if (ret.hasError())
   {
     fco.setError(ret.error());
     onSocketDisconnected(ret.error());
     return;
   }
   bool ready = false;
   {
     boost::mutex::scoped_lock lock(_mutex);
     if (isAdd)
       _addSignalLink = ret.value();
     else
       _removeSignalLink = ret.value();
     ready = _addSignalLink && _removeSignalLink;
   }
   if (ready)
   {
     fco.setValue(0);
     connected();
   }
 }

  void ServiceDirectoryClient::onMetaObjectFetched(qi::Future<void> future, qi::Promise<void> promise) {
    if (future.hasError()) {
      promise.setError(future.error());
      onSocketDisconnected(future.error());
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
      promise.setError(future.error());
      onSocketDisconnected(future.error());
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
      return qi::makeFutureError<void>("!sdSocket");
    _sdSocketDisconnectedSignalLink = _sdSocket->disconnected.connect(boost::bind<void>(&ServiceDirectoryClient::onSocketDisconnected, this, _1));
    _remoteObject.setTransportSocket(_sdSocket);

    qi::Promise<void> promise(qi::FutureCallbackType_Sync);
    qi::Future<void> fut = _sdSocket->connect(serviceDirectoryURL);
    fut.connect(&ServiceDirectoryClient::onSocketConnected, this, _1, promise);
    return promise.future();
  }

  static void sharedPtrHolder(TransportSocketPtr* ptr)
  {
    delete ptr;
  }

  qi::FutureSync<void> ServiceDirectoryClient::close() {
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
    qi::Future<void> fut = socket->disconnect();

    onSocketDisconnected("User closed the connection");

    // Hold the socket shared ptr alive until the future returns.
    // otherwise, the destructor will block us until disconnect terminates
    // Nasty glitch: socket is reusing promises, so this future hook will stay
    // So pass shared pointer by pointer: that way a single delete statement
    // will end all copies.
    fut.connect(&sharedPtrHolder, new TransportSocketPtr(socket));

    socket.reset();
    return fut;
  }

  bool                 ServiceDirectoryClient::isConnected() const {
    return _sdSocket == 0 ? false : _sdSocket->isConnected();
  }

  qi::Url              ServiceDirectoryClient::url() const {
    return _sdSocket ? _sdSocket->url() : qi::Url();
  }

  void ServiceDirectoryClient::onServiceRemoved(unsigned int idx, const std::string &name) {
    qiLogVerbose() << "ServiceDirectoryClient: Service Removed #" << idx << ": " << name << std::endl;
    serviceRemoved(idx, name);
  }

  void ServiceDirectoryClient::onServiceAdded(unsigned int idx, const std::string &name) {
    qiLogVerbose() << "ServiceDirectoryClient: Service Added #" << idx << ": " << name << std::endl;
    serviceAdded(idx, name);
  }

  void ServiceDirectoryClient::onSocketDisconnected(std::string error) {
    disconnected(error);
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
  }

  qi::Future< std::vector<ServiceInfo> > ServiceDirectoryClient::services() {
    return _object.call< std::vector<ServiceInfo> >("services");
  }

  qi::Future<ServiceInfo>              ServiceDirectoryClient::service(const std::string &name) {
    return _object.call< ServiceInfo >("service", name);
  }

  qi::Future<unsigned int>             ServiceDirectoryClient::registerService(const ServiceInfo &svcinfo) {
    return _object.call< unsigned int >("registerService", svcinfo);
  }

  qi::Future<void>                     ServiceDirectoryClient::unregisterService(const unsigned int &idx) {
    return _object.call<void>("unregisterService", idx);
  }

  qi::Future<void>                     ServiceDirectoryClient::serviceReady(const unsigned int &idx) {
    return _object.call<void>("serviceReady", idx);
  }

  qi::Future<void>                     ServiceDirectoryClient::updateServiceInfo(const ServiceInfo &svcinfo) {
    return _object.call<void>("updateServiceInfo", svcinfo);
  }

}
