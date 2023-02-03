/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/
#include "servicedirectoryclient.hpp"
#include <qi/type/objecttypebuilder.hpp>
#include "servicedirectory_p.hpp"
#include "messagesocket.hpp"
#include "server.hpp"
#include "authprovider_p.hpp"

qiLogCategory("qimessaging.servicedirectoryclient");

namespace qi {

  ServiceDirectoryClient::StateData::StateData(StateData&& o)
  {
    // Avoid repeating code by using the operator=. The performance impact should be negligible.
    *this = std::move(o);
  }

  ServiceDirectoryClient::StateData&
  ServiceDirectoryClient::StateData::operator=(StateData&& o)
  {
    sdSocket = ka::exchange(o.sdSocket, nullptr);
    sdSocketDisconnectedSignalLink = exchangeInvalidSignalLink(o.sdSocketDisconnectedSignalLink);
    sdSocketSocketEventSignalLink = exchangeInvalidSignalLink(o.sdSocketSocketEventSignalLink);
    addSignalLink = exchangeInvalidSignalLink(o.addSignalLink);
    removeSignalLink = exchangeInvalidSignalLink(o.removeSignalLink);
    localSd = ka::exchange(o.localSd, false);
    return *this;
  }

  ServiceDirectoryClient::ServiceDirectoryClient(
    ssl::ClientConfig sslConfig,
    boost::optional<ClientAuthenticatorFactoryPtr> clientAuthFactory)
    : _remoteObject(RemoteObject::makePtr(qi::Message::Service_ServiceDirectory))
    // Even if authentication is disabled, the factory is used to instantiate an authenticator.
    // We initialize the factory with one that creates authenticators that do nothing.
    , _authFactory(std::move(clientAuthFactory)
                     .value_or_eval(boost::make_shared<NullClientAuthenticatorFactory>))
    , _sslConfig(std::move(sslConfig))
    , _enforceAuth(clientAuthFactory.is_initialized())
  {
    _object = makeDynamicAnyObject(_remoteObject.get(), false);

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
      qi::Future<void> fdc = onSocketFailure(_stateData.sdSocket, future.error());
      fdc.then(std::bind(&qi::Promise<void>::setError, promise, future.error()));
      return;
    }
    bool ready = false;
    {
      boost::mutex::scoped_lock lock(_mutex);
      if (isAdd)
        _stateData.addSignalLink = future.value();
      else
        _stateData.removeSignalLink = future.value();
      ready = isValidSignalLink(_stateData.addSignalLink) &&
              isValidSignalLink(_stateData.removeSignalLink);
    }
    if (ready)
    {
      promise.setValue(0);
      connected();
    }
  }

  bool ServiceDirectoryClient::isPreviousSdSocket(const MessageSocketPtr& socket)
  {
    boost::mutex::scoped_lock lock(_mutex);
    return socket != _stateData.sdSocket;
  }

  void ServiceDirectoryClient::cleanupPreviousSdSocket(const MessageSocketPtr& socket,
                                                qi::Promise<void> connectionPromise) const
  {
    boost::mutex::scoped_lock lock(_mutex);
    if (socket)
      socket->disconnect().async();
    connectionPromise.setError("Socket has been reset");
  }

  Future<void> ServiceDirectoryClient::closeImpl(const std::string& reason,
                                                 bool sendSignalDisconnected)
  {
    // In order to hold the mutex for the shortest time, we swap the member variables with local
    // variables (thus resetting the member variables to 0).
    StateData stateData;
    {
      boost::mutex::scoped_lock lock(_mutex);
      stateData = std::move(_stateData);
    }

    Future<void> fut = futurize(); // if there was nothing asynchronous to do, return a future with
                                   // a value already set

    using std::placeholders::_1;
    if (stateData.sdSocket)
    {
      static const auto logSocketSignalDisc = [](const char* prefix, Future<bool> discFut) {
        if (discFut.hasError())
          qiLogDebug() << prefix << discFut.error();
        else if (!discFut.value())
          qiLogDebug() << prefix << "unknown error";
      };

      // Unlink from socket signal before disconnecting it.
      stateData.sdSocket->disconnected
        .disconnectAsync(exchangeInvalidSignalLink(stateData.sdSocketDisconnectedSignalLink))
        .then(std::bind(logSocketSignalDisc,
                        "Failed to disconnect Socket::disconnected: ", _1));
      stateData.sdSocket->socketEvent
        .disconnectAsync(exchangeInvalidSignalLink(stateData.sdSocketSocketEventSignalLink))
        .then(std::bind(logSocketSignalDisc,
                        "Failed to disconnect Socket::socketEvent: ", _1));
      fut = stateData.sdSocket->disconnect().async();

      if (sendSignalDisconnected)
        disconnected(reason);
    }

    {
      static const auto logObjectSignalDisc = [](const char* prefix, Future<void> discFut) {
        if (discFut.hasError())
          qiLogDebug() << prefix << discFut.error();
      };

      _object.disconnect(exchangeInvalidSignalLink(stateData.addSignalLink)).async()
        .then(std::bind(logObjectSignalDisc, "Failed to disconnect SDC::serviceAdded: ", _1));

      _object.disconnect(exchangeInvalidSignalLink(stateData.removeSignalLink)).async()
        .then(std::bind(logObjectSignalDisc, "Failed to disconnect SDC::serviceRemoved: ", _1));
    }

    if (stateData.localSd)
    {
      boost::mutex::scoped_lock lock(_mutex);
      _object = makeDynamicAnyObject(_remoteObject.get(), false);
    }

    return fut;
  }

  void ServiceDirectoryClient::onMetaObjectFetched(MessageSocketPtr socket,
                                                   qi::Future<void> future,
                                                   qi::Promise<void> promise)
  {
    if (isPreviousSdSocket(socket))
    {
      cleanupPreviousSdSocket(socket, promise);
      return;
    }

    if (future.hasError())
    {
      qi::Future<void> fdc = onSocketFailure(socket, future.error());
      fdc.then(std::bind(&qi::Promise<void>::setError, promise, future.error()));
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

    fut1.then(track(
      boost::bind(&ServiceDirectoryClient::onSDEventConnected, this, _1, promise, true), this));
    fut2.then(track(
      boost::bind(&ServiceDirectoryClient::onSDEventConnected, this, _1, promise, false), this));
  }

  namespace service_directory_client_private
  {
    static void sendCapabilities(MessageSocketPtr sock)
    {
      Message msg;
      msg.setType(Message::Type_Capability);
      msg.setService(Message::Service_Server);
      msg.setValue(sock->localCapabilities(), typeOf<CapabilityMap>()->signature());
      sock->send(std::move(msg));
    }
  } // service_directory_client_private

  void ServiceDirectoryClient::onAuthentication(MessageSocketPtr socket,
                                                const MessageSocket::SocketEventData& data,
                                                qi::Promise<void> prom,
                                                ClientAuthenticatorPtr authenticator)
  {
    static const std::string cmsig = typeOf<CapabilityMap>()->signature().toString();
    if (isPreviousSdSocket(socket))
    {
      cleanupPreviousSdSocket(socket, prom);
      return;
    }

    if (data.which() == MessageSocket::Event_Error)
    {
      if (socket)
        socket->socketEvent.disconnect(
          exchangeInvalidSignalLink(_stateData.sdSocketSocketEventSignalLink));
      const std::string& err = boost::get<std::string>(data);
      qi::Future<void> fdc = onSocketFailure(socket, err);
      fdc.then(std::bind(&qi::Promise<void>::setError, prom, err));
      return;
    }

    const Message& msg = boost::get<const Message&>(data);
    unsigned int function = msg.function();
    bool failure = msg.type() == Message::Type_Error
        || msg.service() != Message::Service_Server
        || function != Message::ServerFunction_Authenticate;

    if (failure)
    {
      if (socket)
        socket->socketEvent.disconnect(
          exchangeInvalidSignalLink(_stateData.sdSocketSocketEventSignalLink));
      if (_enforceAuth)
      {
        std::stringstream error;
        if (msg.type() == Message::Type_Error)
          error << "Authentication failed: " << msg.value("s", socket).to<std::string>();
        else
          error << "Expected a message for function #" << Message::ServerFunction_Authenticate
                << " (authentication), received a message for function " << msg.function();
        qi::Future<void> fdc = onSocketFailure(socket, error.str());
        fdc.then(std::bind(&qi::Promise<void>::setError, prom, error.str()));
      }
      else
      {
        service_directory_client_private::sendCapabilities(socket);
        qi::Future<void> future = _remoteObject->fetchMetaObject();
        future.connect(track(
          boost::bind(&ServiceDirectoryClient::onMetaObjectFetched, this, socket, _1, prom), this));
      }
      return;
    }

    CapabilityMap authData = msg.value(typeOf<CapabilityMap>()->signature(), socket).to<CapabilityMap>();
    CapabilityMap::iterator authStateIt = authData.find(AuthProvider::State_Key);

    if (authStateIt == authData.end() || authStateIt->second.to<unsigned int>() < AuthProvider::State_Error
        || authStateIt->second.to<unsigned int>() > AuthProvider::State_Done)
    {
      if (socket)
        socket->socketEvent.disconnect(
          exchangeInvalidSignalLink(_stateData.sdSocketSocketEventSignalLink));
      std::string error = "Invalid authentication state token.";
      qi::Future<void> fdc = onSocketFailure(socket, error);
      fdc.then(std::bind(&qi::Promise<void>::setError, prom, error));
      qiLogError() << error;
      return;
    }
    if (authData[AuthProvider::State_Key].to<unsigned int>() == AuthProvider::State_Done)
    {
      if (socket)
        socket->socketEvent.disconnect(
          exchangeInvalidSignalLink(_stateData.sdSocketSocketEventSignalLink));
      qi::Future<void> future = _remoteObject->fetchMetaObject();
      future.connect(track(
        boost::bind(&ServiceDirectoryClient::onMetaObjectFetched, this, socket, _1, prom), this));
      return;
    }

    CapabilityMap nextData = authenticator->processAuth(authData);
    Message authMsg;
    authMsg.setService(Message::Service_Server);
    authMsg.setType(Message::Type_Call);
    authMsg.setValue(nextData, cmsig);
    authMsg.setFunction(Message::ServerFunction_Authenticate);
    socket->send(std::move(authMsg));
  }

  void ServiceDirectoryClient::onSocketConnected(MessageSocketPtr socket,
                                                 qi::Future<void> future,
                                                 qi::Promise<void> promise)
  {
    if (isPreviousSdSocket(socket))
    {
      cleanupPreviousSdSocket(socket, promise);
      return;
    }

    if (future.hasError()) {
      qi::Future<void> fdc = onSocketFailure(socket, future.error(), false);
      fdc.then(std::bind(&qi::Promise<void>::setError, promise, future.error()));
      return;
    }
    if (!socket)
      return;

    ClientAuthenticatorPtr authenticator = _authFactory->newAuthenticator();
    CapabilityMap authCaps;
    {
      CapabilityMap tmp = authenticator->initialAuthData();
      for (CapabilityMap::iterator it = tmp.begin(), end = tmp.end(); it != end; ++it)
      {
        authCaps[AuthProvider::UserAuthPrefix + it->first] = it->second;
      }
    }

    _stateData.sdSocketSocketEventSignalLink = socket->socketEvent.connect(track(
        [=](const MessageSocket::SocketEventData& data) {
          onAuthentication(socket, data, promise, authenticator);
        },
        this));

    CapabilityMap socketCaps = socket->localCapabilities();
    socketCaps.insert(authCaps.begin(), authCaps.end());

    Message msgCapabilities;
    msgCapabilities.setFunction(Message::ServerFunction_Authenticate);
    msgCapabilities.setService(Message::Service_Server);
    msgCapabilities.setType(Message::Type_Call);
    msgCapabilities.setValue(socketCaps, typeOf<CapabilityMap>()->signature());
    socket->send(std::move(msgCapabilities));
  }

  //we ensure in that function that connect to all events are already setup when we said we are connect.
  //that way we can't be connected without being fully ready.
  qi::FutureSync<void> ServiceDirectoryClient::connect(const qi::Url &serviceDirectoryURL) {
    if (isConnected()) {
      const char* s = "Session is already connected";
      qiLogVerbose() << s;
      return qi::makeFutureError<void>(s);
    }
    qi::Promise<void> promise;
    qi::Future<void> connecting;
    {
      boost::mutex::scoped_lock lock(_mutex);

      _stateData.url = serviceDirectoryURL;
      if (_stateData.sdSocket)
        _stateData.sdSocket->disconnect().async();
      _stateData.sdSocket = qi::makeMessageSocket(_sslConfig);

      if (!_stateData.sdSocket)
        return qi::makeFutureError<void>(std::string("unrecognized protocol '") + serviceDirectoryURL.protocol() + "' in url '" + serviceDirectoryURL.str() + "'");
      auto socket = _stateData.sdSocket;
      _stateData.sdSocketDisconnectedSignalLink = _stateData.sdSocket->disconnected.connect(track(
          [=](const std::string& reason) mutable {
            auto fdc = onSocketFailure(socket, reason, true).async();
            fdc.andThen([=](void*) mutable {
              if (promise.future().isRunning())
              {
                promise.setError(reason);
              }
            });
            fdc.wait();
          },
          this));
      _remoteObject->setTransportSocket(_stateData.sdSocket);

      connecting = _stateData.sdSocket->connect(serviceDirectoryURL);
    }

    connecting.connect(
      track(boost::bind(&ServiceDirectoryClient::onSocketConnected, this, _stateData.sdSocket,
                              _1, promise), this));
    return promise.future();
  }

  void ServiceDirectoryClient::setServiceDirectory(AnyObject serviceDirectoryService)
  {
    _object = serviceDirectoryService;
    _stateData.localSd = true;

    {
      boost::mutex::scoped_lock lock(_mutex);
      _stateData.addSignalLink = _object.connect(
          "serviceAdded",
          boost::function<void(unsigned int, const std::string&)>(
            qi::bind(&ServiceDirectoryClient::onServiceAdded, this, _1, _2))).value();
      _stateData.removeSignalLink = _object.connect(
          "serviceRemoved",
          boost::function<void(unsigned int, const std::string&)>(
            qi::bind(&ServiceDirectoryClient::onServiceRemoved, this, _1, _2))).value();
    }

    connected();
  }

  qi::FutureSync<void> ServiceDirectoryClient::close() {
    return closeImpl("User closed the connection", true);
  }

  bool ServiceDirectoryClient::isConnected() const
  {
    if (_stateData.localSd)
      return true;
    boost::mutex::scoped_lock lock(_mutex);
    return _stateData.sdSocket == 0 ? false : _stateData.sdSocket->isConnected();
  }

  qi::Url              ServiceDirectoryClient::url() const {
    if (_stateData.localSd)
      throw std::runtime_error("Service directory is local, url() unknown.");
    boost::mutex::scoped_lock lock(_mutex);
    if (!_stateData.sdSocket)
      throw std::runtime_error("Session disconnected");
    return _stateData.url;
  }

  void                  ServiceDirectoryClient::setClientAuthenticatorFactory(ClientAuthenticatorFactoryPtr authFactory)
  {
    _authFactory = authFactory;
  }

  void ServiceDirectoryClient::onServiceRemoved(unsigned int idx, const std::string &name) {
    qiLogVerbose() << "ServiceDirectoryClient: Service Removed #" << idx << ": " << name;
    serviceRemoved(idx, name);
  }

  void ServiceDirectoryClient::onServiceAdded(unsigned int idx, const std::string &name) {
    qiLogVerbose() << "ServiceDirectoryClient: Service Added #" << idx << ": " << name;
    serviceAdded(idx, name);
  }

  qi::FutureSync<void> ServiceDirectoryClient::onSocketFailure(MessageSocketPtr socket,
                                                               std::string error,
                                                               bool sendSignalDisconnected)
  {
    if (isPreviousSdSocket(socket))
    {
      cleanupPreviousSdSocket(socket, qi::Promise<void>{});
      return futurize();
    }

    return closeImpl(error, sendSignalDisconnected);
  }

  MessageSocketPtr ServiceDirectoryClient::socket()
  {
    boost::mutex::scoped_lock lock(_mutex);
    return _stateData.sdSocket;
  }

  bool ServiceDirectoryClient::isLocal()
  {
    return _stateData.localSd;
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

  qi::Future<qi::MessageSocketPtr>   ServiceDirectoryClient::_socketOfService(unsigned int id) {
    return _object.async<MessageSocketPtr>("_socketOfService", id);
  }
}
