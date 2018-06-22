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


  ServiceDirectoryClient::ServiceDirectoryClient(bool enforceAuth)
    : _sdSocketDisconnectedSignalLink(0)
    , _sdSocketSocketEventSignalLink(0)
    , _remoteObject(new RemoteObject(qi::Message::Service_ServiceDirectory))
    , _addSignalLink(0)
    , _removeSignalLink(0)
    , _localSd(false)
    , _enforceAuth(enforceAuth)
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
      qi::Future<void> fdc = onSocketFailure(_sdSocket, future.error());
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

  bool ServiceDirectoryClient::isPreviousSdSocket(const MessageSocketPtr& socket)
  {
    boost::mutex::scoped_lock lock(_mutex);
    return socket != _sdSocket;
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
    MessageSocketPtr sdSocket;
    SignalLink addLink = 0, removeLink = 0, socketDisconnectedLink = 0, socketEventLink = 0;
    bool localSd = false;
    {
      boost::mutex::scoped_lock lock(_mutex);
      using std::swap;
      swap(sdSocket, _sdSocket);
      swap(socketDisconnectedLink, _sdSocketDisconnectedSignalLink);
      swap(socketEventLink, _sdSocketSocketEventSignalLink);
      swap(addLink, _addSignalLink);
      swap(removeLink, _removeSignalLink);
      swap(localSd, _localSd);
    }

    Future<void> fut = futurize(); // if there was nothing asynchronous to do, return a future with
                                   // a value already set

    using std::placeholders::_1;
    if (sdSocket)
    {
      static const auto logSocketSignalDisc = [](const char* prefix, Future<bool> discFut) {
        if (discFut.hasError())
          qiLogDebug() << prefix << discFut.error();
        else if (!discFut.value())
          qiLogDebug() << prefix << "unknown error";
      };

      // Unlink from socket signal before disconnecting it.
      if (socketDisconnectedLink != 0)
        sdSocket->disconnected.disconnectAsync(socketDisconnectedLink)
            .then(std::bind(logSocketSignalDisc, "Failed to disconnect Socket::disconnected: ", _1));
      if (socketEventLink != 0)
        sdSocket->socketEvent.disconnectAsync(socketEventLink)
            .then(std::bind(logSocketSignalDisc, "Failed to disconnect Socket::socketEvent: ", _1));
      fut = sdSocket->disconnect().async();

      if (sendSignalDisconnected)
        disconnected(reason);
    }

    {
      static const auto logObjectSignalDisc = [](const char* prefix, Future<void> discFut) {
        if (discFut.hasError())
          qiLogDebug() << prefix << discFut.error();
      };

      if (addLink != 0)
        _object.disconnect(addLink).async().then(
              std::bind(logObjectSignalDisc, "Failed to disconnect SDC::serviceAdded: ", _1));

      if (removeLink != 0)
        _object.disconnect(removeLink).async().then(
              std::bind(logObjectSignalDisc, "Failed to disconnect SDC::serviceRemoved: ", _1));
    }

    if (localSd)
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

  namespace service_directory_client_private
  {
    static void sendCapabilities(MessageSocketPtr sock)
    {
      Message msg;
      msg.setType(Message::Type_Capability);
      msg.setService(Message::Service_Server);
      msg.setValue(sock->localCapabilities(), typeOf<CapabilityMap>()->signature());
      sock->send(msg);
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
        socket->socketEvent.disconnect(_sdSocketSocketEventSignalLink);
      const std::string& err = boost::get<std::string>(data);
      qi::Future<void> fdc = onSocketFailure(socket, err);
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
      if (socket)
        socket->socketEvent.disconnect(_sdSocketSocketEventSignalLink);
      if (_enforceAuth)
      {
        std::stringstream error;
        if (msg.type() == Message::Type_Error)
          error << "Authentication failed: " << msg.value("s", socket).to<std::string>();
        else
          error << "Expected a message for function #" << Message::ServerFunction_Authenticate << " (authentication), received a message for function " << msg.function();
        qi::Future<void> fdc = onSocketFailure(socket, error.str());
        fdc.connect(&qi::Promise<void>::setError, prom, error.str());
      }
      else
      {
        service_directory_client_private::sendCapabilities(socket);
        qi::Future<void> future = _remoteObject->fetchMetaObject();
        future.connect(&ServiceDirectoryClient::onMetaObjectFetched, this, socket, _1, prom);
      }
      return;
    }

    AnyReference cmref = msg.value(typeOf<CapabilityMap>()->signature(), socket);
    CapabilityMap authData = cmref.to<CapabilityMap>();
    cmref.destroy();
    CapabilityMap::iterator authStateIt = authData.find(AuthProvider::State_Key);

    if (authStateIt == authData.end() || authStateIt->second.to<unsigned int>() < AuthProvider::State_Error
        || authStateIt->second.to<unsigned int>() > AuthProvider::State_Done)
    {
      if (socket)
        socket->socketEvent.disconnect(_sdSocketSocketEventSignalLink);
      std::string error = "Invalid authentication state token.";
      qi::Future<void> fdc = onSocketFailure(socket, error);
      fdc.connect(&qi::Promise<void>::setError, prom, error);
      qiLogError() << error;
      return;
    }
    if (authData[AuthProvider::State_Key].to<unsigned int>() == AuthProvider::State_Done)
    {
      if (socket)
        socket->socketEvent.disconnect(_sdSocketSocketEventSignalLink);
      qi::Future<void> future = _remoteObject->fetchMetaObject();
      future.connect(&ServiceDirectoryClient::onMetaObjectFetched, this, socket, _1, prom);
      return;
    }

    CapabilityMap nextData = authenticator->processAuth(authData);
    Message authMsg;
    authMsg.setService(Message::Service_Server);
    authMsg.setType(Message::Type_Call);
    authMsg.setValue(nextData, cmsig);
    authMsg.setFunction(Message::ServerFunction_Authenticate);
    socket->send(authMsg);
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
      fdc.connect(&qi::Promise<void>::setError, promise, future.error());
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

    _sdSocketSocketEventSignalLink = socket->socketEvent.connect(track(
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
    socket->send(msgCapabilities);
  }

  //we ensure in that function that connect to all events are already setup when we said we are connect.
  //that way we can't be connected without being fully ready.
  qi::FutureSync<void> ServiceDirectoryClient::connect(const qi::Url &serviceDirectoryURL) {
    if (isConnected()) {
      const char* s = "Session is already connected";
      qiLogInfo() << s;
      return qi::makeFutureError<void>(s);
    }
    qi::Promise<void> promise;
    qi::Future<void> connecting;
    {
      boost::mutex::scoped_lock lock(_mutex);

      if (_sdSocket)
        _sdSocket->disconnect().async();
      _sdSocket = qi::makeMessageSocket(serviceDirectoryURL.protocol());

      if (!_sdSocket)
        return qi::makeFutureError<void>(std::string("unrecognized protocol '") + serviceDirectoryURL.protocol() + "' in url '" + serviceDirectoryURL.str() + "'");
      auto socket = _sdSocket;
      _sdSocketDisconnectedSignalLink = _sdSocket->disconnected.connect(track(
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
      _remoteObject->setTransportSocket(_sdSocket);

      connecting = _sdSocket->connect(serviceDirectoryURL);
    }

    connecting.connect(&ServiceDirectoryClient::onSocketConnected, this, _sdSocket, _1, promise);
    return promise.future();
  }

  void ServiceDirectoryClient::setServiceDirectory(AnyObject serviceDirectoryService)
  {
    _object = serviceDirectoryService;
    _localSd = true;

    {
      boost::mutex::scoped_lock lock(_mutex);
      _addSignalLink = _object.connect(
          "serviceAdded",
          boost::function<void(unsigned int, const std::string&)>(
            qi::bind(&ServiceDirectoryClient::onServiceAdded, this, _1, _2)));
      _removeSignalLink = _object.connect(
          "serviceRemoved",
          boost::function<void(unsigned int, const std::string&)>(
            qi::bind(&ServiceDirectoryClient::onServiceRemoved, this, _1, _2)));
    }

    connected();
  }

  qi::FutureSync<void> ServiceDirectoryClient::close() {
    return closeImpl("User closed the connection", true);
  }

  bool ServiceDirectoryClient::isConnected() const
  {
    if (_localSd)
      return true;
    boost::mutex::scoped_lock lock(_mutex);
    return _sdSocket == 0 ? false : _sdSocket->isConnected();
  }

  qi::Url              ServiceDirectoryClient::url() const {
    if (_localSd)
      throw std::runtime_error("Service directory is local, url() unknown.");
    boost::mutex::scoped_lock lock(_mutex);
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

  qi::Future<qi::MessageSocketPtr>   ServiceDirectoryClient::_socketOfService(unsigned int id) {
    return _object.async<MessageSocketPtr>("_socketOfService", id);
  }
}
