/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/
#include "servicedirectoryclient.hpp"
#include <qi/type/objecttypebuilder.hpp>
#include "servicedirectory_p.hpp"
#include "transportsocket.hpp"
#include "server.hpp"
#include "authprovider_p.hpp"

qiLogCategory("qimessaging.servicedirectoryclient");

namespace qi {


  ServiceDirectoryClient::ServiceDirectoryClient(bool enforceAuth)
    : _sdSocketDisconnectedSignalLink(0)
    , _remoteObject(new RemoteObject(qi::Message::Service_ServiceDirectory))
    , _addSignalLink(0)
    , _removeSignalLink(0)
    , _localSd(false)
    , _enforceAuth(enforceAuth)
  {
    _object = makeDynamicAnyObject(_remoteObject, true);

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

    qi::Future<SignalLink> fut1 = _object.connect(
        "serviceAdded",
        boost::function<void(unsigned int, const std::string&)>(
          qi::bind(&ServiceDirectoryClient::onServiceAdded, this, _1, _2)));
    qi::Future<SignalLink> fut2 = _object.connect(
        "serviceRemoved",
        boost::function<void(unsigned int, const std::string&)>(
          qi::bind(&ServiceDirectoryClient::onServiceRemoved, this, _1, _2)));

    fut1.connect(&ServiceDirectoryClient::onSDEventConnected, this, _1, promise, true);
    fut2.connect(&ServiceDirectoryClient::onSDEventConnected, this, _1, promise, false);
  }

  static void sendCapabilities(TransportSocketPtr sock)
  {
    Message msg;
    msg.setType(Message::Type_Capability);
    msg.setService(Message::Service_Server);
    msg.setValue(sock->localCapabilities(), typeOf<CapabilityMap>()->signature());
    sock->send(msg);
  }

  void ServiceDirectoryClient::onAuthentication(const TransportSocket::SocketEventData& data, qi::Promise<void> prom, ClientAuthenticatorPtr authenticator, SignalSubscriberPtr old)
  {
    static const std::string cmsig = typeOf<CapabilityMap>()->signature().toString();
    TransportSocketPtr sdSocket = _sdSocket;
    if (data.which() == TransportSocket::Event_Error)
    {
      if (sdSocket)
        sdSocket->socketEvent.disconnect(*old);
      const std::string& err = boost::get<std::string>(data);
      qi::Future<void> fdc = onSocketDisconnected(err);
      fdc.connect(&qi::Promise<void>::setError, prom, err);
      return;
    }

    const Message& msg = boost::get<const Message&>(data);
    unsigned int function = msg.function();
    bool failure = msg.type() == Message::Type_Error
        || msg.service() != Message::Service_Server
        || function != Message::ServerFunction_Authenticate;

    if (failure)
    {
      if (sdSocket)
        sdSocket->socketEvent.disconnect(*old);
      if (_enforceAuth)
      {
        std::stringstream error;
        if (msg.type() == Message::Type_Error)
          error << "Authentication failed: " << msg.value("s", _sdSocket).to<std::string>();
        else
          error << "Expected a message for function #" << Message::ServerFunction_Authenticate << " (authentication), received a message for function " << msg.function();
        qi::Future<void> fdc = onSocketDisconnected(error.str());
        fdc.connect(&qi::Promise<void>::setError, prom, error.str());
      }
      else
      {
        sendCapabilities(sdSocket);
        qi::Future<void> future = _remoteObject->fetchMetaObject();
        future.connect(&ServiceDirectoryClient::onMetaObjectFetched, this, _1, prom);
      }
      return;
    }

    AnyReference cmref = msg.value(typeOf<CapabilityMap>()->signature(), _sdSocket);
    CapabilityMap authData = cmref.to<CapabilityMap>();
    cmref.destroy();
    CapabilityMap::iterator authStateIt = authData.find(AuthProvider::State_Key);

    if (authStateIt == authData.end() || authStateIt->second.to<unsigned int>() < AuthProvider::State_Error
        || authStateIt->second.to<unsigned int>() > AuthProvider::State_Done)
    {
      if (sdSocket)
        sdSocket->socketEvent.disconnect(*old);
      std::string error = "Invalid authentication state token.";
      qi::Future<void> fdc = onSocketDisconnected(error);
      fdc.connect(&qi::Promise<void>::setError, prom, error);
      qiLogError() << error;
      return;
    }
    if (authData[AuthProvider::State_Key].to<unsigned int>() == AuthProvider::State_Done)
    {
      if (sdSocket)
        sdSocket->socketEvent.disconnect(*old);
      qi::Future<void> future = _remoteObject->fetchMetaObject();
      future.connect(&ServiceDirectoryClient::onMetaObjectFetched, this, _1, prom);
      return;
    }

    CapabilityMap nextData = authenticator->processAuth(authData);
    Message authMsg;
    authMsg.setService(Message::Service_Server);
    authMsg.setType(Message::Type_Call);
    authMsg.setValue(nextData, cmsig);
    authMsg.setFunction(Message::ServerFunction_Authenticate);
    _sdSocket->send(authMsg);
  }

  void ServiceDirectoryClient::onSocketConnected(qi::FutureSync<void> future, qi::Promise<void> promise) {
    TransportSocketPtr sdSocket = _sdSocket;

    if (future.hasError()) {
      qi::Future<void> fdc = onSocketDisconnected(future.error());
      fdc.connect(&qi::Promise<void>::setError, promise, future.error());
      return;
    }
    if (!sdSocket)
      return;

    ClientAuthenticatorPtr authenticator = _authFactory->newAuthenticator();
    CapabilityMap authCaps;
    {
      CapabilityMap tmp = authenticator->initialAuthData();
      for (CapabilityMap::iterator it = tmp.begin(), end = tmp.end(); it != end; ++it)
        authCaps[AuthProvider::UserAuthPrefix + it->first] = it->second;
    }
    SignalSubscriberPtr protocolSubscriber(new SignalSubscriber);
    *protocolSubscriber = sdSocket->socketEvent.connect(&ServiceDirectoryClient::onAuthentication, this, _1, promise, authenticator, protocolSubscriber);

    CapabilityMap socketCaps = sdSocket->localCapabilities();
    socketCaps.insert(authCaps.begin(), authCaps.end());

    Message msgCapabilities;
    msgCapabilities.setFunction(Message::ServerFunction_Authenticate);
    msgCapabilities.setService(Message::Service_Server);
    msgCapabilities.setType(Message::Type_Call);
    msgCapabilities.setValue(socketCaps, typeOf<CapabilityMap>()->signature());
    sdSocket->send(msgCapabilities);
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
    _remoteObject->setTransportSocket(_sdSocket);

    qi::Promise<void> promise;
    qi::Future<void> fut = _sdSocket->connect(serviceDirectoryURL);
    fut.connect(&ServiceDirectoryClient::onSocketConnected, this, _1, promise);
    return promise.future();
  }

  void ServiceDirectoryClient::setServiceDirectory(AnyObject serviceDirectoryService)
  {
    _object = serviceDirectoryService;
    _localSd = true;

    _addSignalLink = _object.connect(
        "serviceAdded",
        boost::function<void(unsigned int, const std::string&)>(
          qi::bind(&ServiceDirectoryClient::onServiceAdded, this, _1, _2)));
    _removeSignalLink = _object.connect(
        "serviceRemoved",
        boost::function<void(unsigned int, const std::string&)>(
          qi::bind(&ServiceDirectoryClient::onServiceRemoved, this, _1, _2)));

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

  void                  ServiceDirectoryClient::setClientAuthenticatorFactory(ClientAuthenticatorFactoryPtr authFactory)
  {
    _authFactory = authFactory;
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
      _remoteObject->close("Socket disconnected");
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
