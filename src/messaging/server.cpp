/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/
#include <qi/anyobject.hpp>
#include "transportserver.hpp"
#include <qi/messaging/serviceinfo.hpp>
#include <qi/type/objecttypebuilder.hpp>
#include "objectregistrar.hpp"
#include <qi/os.hpp>
#include <boost/thread/mutex.hpp>
#include <exception>
#include "servicedirectoryclient.hpp"
#include "authprovider_p.hpp"

qiLogCategory("qimessaging.server");

// Helper for debug logs of the class. Can only be called from non static member functions.
#define QI_LOG_DEBUG_SERVER() qiLogDebug() << this << " - "

namespace qi
{

  namespace detail
  {
    namespace server
    {
      SocketInfo::SocketInfo(const MessageSocketPtr& socket,
                             SignalSubscriber onDisconnected,
                             AuthProviderPtr authProvider) noexcept
        : _socket(socket)
        , _disconnected(socket->disconnected.connect(std::move(onDisconnected)))
        , _authProvider(std::move(authProvider))
      {
      }

      SocketInfo::~SocketInfo()
      {
        tracker.destroy();

        auto sock = _socket.lock();
        if (!sock)
          return;

        if (isValidSignalLink(_disconnected))
        {
          const auto logExceptMsgWarning =
            ka::compose([](const std::string& msg) {
              qiLogWarning() << "Failed to disconnect from socket `disconnected` signal: " << msg;
            }, ka::exception_message_t{});
          ka::invoke_catch(logExceptMsgWarning, [&]{
            sock->disconnected.disconnectAsync(_disconnected);
          });
        }

        _msgDispatchConnection.reset();

        // The server is responsible for disconnecting its sockets.
        qiLogDebug() << "Disconnecting socket " << sock << ".";
        sock->disconnect().async();
      }

      CapabilityMap SocketInfo::extractCapabilities()
      {
        if (ka::exchange(_capabilitiesTaken, true))
          return {};
        const auto socket = _socket.lock();
        QI_ASSERT_NOT_NULL(socket);
        return socket->localCapabilities();
      }

      void SocketInfo::setServerMessageHandler(MessageDispatcher::MessageHandler handler)
      {
        if (_msgDispatchConnection)
          // TODO: Throw a domain specific error.
          throw std::logic_error(
            "Cannot set a socket message handler for the server: one is already set.");

        _msgDispatchConnection.emplace(
          _socket.lock(),
          MessageDispatcher::RecipientId{ Message::Service_Server, Message::GenericObject_None },
          std::move(handler));
      }

      BoundObjectSocketBinder::BoundObjectSocketBinder() noexcept
      {}

      SocketInfo& BoundObjectSocketBinder::addSocketPendingValidation(
        const MessageSocketPtr& socket,
        SignalSubscriber disconnectionHandlerAnyFunc)
      {
        auto end = _socketsInfo.cend();
        auto sockIt = std::find_if(_socketsInfo.cbegin(), end, [&](const SocketInfoPtr& sockInfo) {
          return sockInfo->socket() == socket;
        });
        if (sockIt != end)
          // TODO: Throw a domain specific error.
          throw std::logic_error("This socket already exists in this server.");

        _socketsInfo.emplace_back(new SocketInfo(socket, std::move(disconnectionHandlerAnyFunc)));
        // TODO: In C++17, `emplace_back` returns a reference to the inserted element, but for now
        // we have to get it manually with a call to `back`.
        return *_socketsInfo.back();
      }

      bool BoundObjectSocketBinder::removeSocket(const MessageSocketPtr& socket) noexcept
      {
        qiLogDebug() << "Removing socket " << socket << ".";
        const auto end = _socketsInfo.end();
        const auto socketIt =
          std::remove_if(_socketsInfo.begin(), end, [&](const SocketInfoPtr& sockInfo) {
            return sockInfo->socket() == socket;
          });
        if (socketIt == end)
          return false;

        _socketsInfo.erase(socketIt, end);

        unbindSocket(socket);
        return true;
      }

      std::size_t BoundObjectSocketBinder::validateSocket(const MessageSocketPtr& socket) noexcept
      {
        return bindSocket(socket);
      }

      bool BoundObjectSocketBinder::addObject(unsigned int serviceId,
                                              BoundObjectPtr object) noexcept
      {
        qiLogDebug() << "Adding a service object (service=" << serviceId << ").";
        auto it = _boundObjects.find(serviceId);
        if (it != _boundObjects.end()) {
          // An object already exists for this id, we won't add a new one.
          return false;
        }
        _boundObjects[serviceId] = object;

        bindObject(object);
        return true;
      }

      bool BoundObjectSocketBinder::removeObject(unsigned int serviceId) noexcept
      {
        qiLogDebug() << "Trying to remove object (id=" << serviceId << ").";
        auto it = _boundObjects.find(serviceId);
        if (it == _boundObjects.end()) {
          return false;
        }

        auto removedObject = std::move(it->second);
        qiLogDebug() << "Object to remove found (" << removedObject << "), removing it.";
        _boundObjects.erase(it);

        unbindObject(removedObject);
        return true;
      }

      BoundObjectSocketBinder::ClearSocketsResult BoundObjectSocketBinder::clearSockets() noexcept
      {
        const auto socketCount = _socketsInfo.size();
        _socketsInfo.clear();
        const auto bindingCount = _bindings.size();
        _bindings.clear();
        return { socketCount, bindingCount };
      }

      std::size_t BoundObjectSocketBinder::bindSocket(const MessageSocketPtr& socket) noexcept
      {
        qiLogDebug() << "Binding objects (count=" << _boundObjects.size() << ") to socket " << socket
                     << ".";

        for (const auto& boundObject : _boundObjects)
          _bindings.emplace_back(boundObject.second, socket);
        return _boundObjects.size();
      }

      std::size_t BoundObjectSocketBinder::bindObject(const BoundObjectPtr& object) noexcept
      {
        qiLogDebug() << "Binding sockets (count=" << _socketsInfo.size() << ") to object " << object
                     << ".";

        for (const auto& socketInfo : _socketsInfo)
          _bindings.emplace_back(object, socketInfo->socket());
        return _socketsInfo.size();
      }

      std::size_t BoundObjectSocketBinder::unbindSocket(const MessageSocketPtr& socket) noexcept
      {
        qiLogDebug() << "Unbinding objects from socket " << socket << ".";

        const auto end = _bindings.end();
        const auto newEnd =
          std::remove_if(_bindings.begin(), end, [&](const boundObject::SocketBinding& binding) {
            return binding.socket() == socket;
          });
        const auto count = std::distance(newEnd, end);
        QI_ASSERT_TRUE(count >= 0);
        _bindings.erase(newEnd, end);
        return static_cast<std::size_t>(count);
      }

      std::size_t BoundObjectSocketBinder::unbindObject(const BoundObjectPtr& object) noexcept
      {
        qiLogDebug() << "Unbinding sockets from object " << object << ".";

        const auto end = _bindings.end();
        const auto newEnd =
          std::remove_if(_bindings.begin(), end, [&](const boundObject::SocketBinding& binding) {
            return binding.object() == object;
          });
        const auto count = std::distance(newEnd, end);
        QI_ASSERT_TRUE(count >= 0);
        _bindings.erase(newEnd, end);
        return static_cast<std::size_t>(count);
      }
    }
  }

  namespace
  {

    bool isAuthRequest(const Message& msg)
    {
      return msg.type() == Message::Type_Call &&
             msg.function() == Message::ServerFunction_Authenticate;
    }

    Message makeReply(const MessageAddress& address)
    {
      return Message{ Message::Type_Reply, address };
    };

  }

  using namespace detail::server;

  Server::Server(ssl::ServerConfig sslConfig,
                 boost::optional<AuthProviderFactoryPtr> authProviderFactory)
    // Even if authentication is disabled, the factory is used to instantiate an authentication provider.
    // We initialize the factory with one that creates authentication providers that do nothing.
    : _state{ authProviderFactory.value_or_eval(boost::make_shared<NullAuthProviderFactory>), {} }
    , _enforceAuth(authProviderFactory.has_value())
    , _server(std::move(sslConfig))
    , _defaultCallType(qi::MetaCallType_Queued)
  {
    _server.newConnection.connect(track(
      [=](const std::pair<MessageSocketPtr, Url>& socketUrl) {
        safeCall([=] { this->addIncomingSocket(socketUrl.first); });
      },
      &_tracker));
  }

  Server::~Server()
  {
    _tracker.destroy();
    closeImpl();
  }

  void Server::closeImpl()
  {
    auto strand = boost::atomic_exchange(&_strand, {});
    if (!strand)
      return;

    strand->join();

    qiLogVerbose() << "Closing server...";

    // Objects are not removed from the server, they will become available again if the server
    // is reopen.
    _state.binder.clearSockets();

    _server.close();
  }

  Future<bool> Server::addObject(unsigned int serviceId, qi::AnyObject obj)
  {
    if (!obj)
      return futurize(false);
    return addObject(serviceId, makeServiceBoundObjectPtr(serviceId, obj, _defaultCallType));
  }

  Future<bool> Server::addObject(unsigned int serviceId, qi::BoundObjectPtr obj)
  {
    if (!obj)
      return futurize(false);
    return safeCall([=]{
      return _state.binder.addObject(serviceId, obj);
    });
  }

  Future<bool> Server::removeObject(unsigned int idx)
  {
    return safeCall([=]{
      return _state.binder.removeObject(idx);
    });
  }

  Future<void> Server::setAuthProviderFactory(AuthProviderFactoryPtr factory)
  {
    return safeCall([=]{
      _state.authProviderFactory = factory;
    });
  }

  SocketInfo& Server::addSocket(MessageSocketPtr socket)
  {
    if (!socket)
      // TODO: Throw a domain specific error.
      throw std::invalid_argument("The socket that was added to the server is null.");

    qiLogVerbose() << this << " - New socket " << socket << " added to the server.";

    boost::function<void(const std::string&)> disconnectionHandler =
      track([=](const std::string&) { safeCall([=] { removeSocket(socket); }); }, &_tracker);
    auto disconnectionHandlerAnyFunc = AnyFunction::from(std::move(disconnectionHandler));

    return _state.binder.addSocketPendingValidation(socket, std::move(disconnectionHandlerAnyFunc));
  }

  Future<bool> Server::addOutgoingSocket(MessageSocketPtr socket)
  {
    return safeCall([=]{
      addSocket(socket);

      // Nothing more to do to start this socket, as it is outgoing, it's already receiving message
      // and there's no need to authenticate it.
      return true;
    });
  }

  namespace
  {
    struct Track
    {
      template<typename F, typename T>
      auto operator()(F&& f, T&& t) const
        -> decltype(track(ka::fwd<F>(f), ka::fwd<T>(t)))
      {
        return track(ka::fwd<F>(f), ka::fwd<T>(t));
      }
    };
  }

  bool Server::addIncomingSocket(MessageSocketPtr incomingSocket)
  {
    // The server is responsible for the socket that was given to him, so in case of error
    // (exception) while adding it, it must disconnect it.
    bool success = false;
    auto scopedSocket = ka::scoped(incomingSocket, [&](MessageSocketPtr socket){
      if (!success && socket)
        socket->disconnect().async();
    });
    auto& socket = scopedSocket.value;

    auto& socketInfo = addSocket(socket);

    socketInfo.setAuthProvider(_state.authProviderFactory->newProvider());

    auto serverLifetimeGuarded = std::bind(Track{}, std::placeholders::_1, &_tracker);
    auto socketInfoLifetimeGuarded = std::bind(Track{}, std::placeholders::_1, &socketInfo.tracker);
    auto asyncServerMessageHandler =
      // Track the socketInfo to be able to retrack it in the callback, as tracking an object
      // requires that it is alive.
      serverLifetimeGuarded(socketInfoLifetimeGuarded(
        [=, &socketInfo](const Message& msg) mutable {
          return safeCall(socketInfoLifetimeGuarded([=, &socketInfo] {
            handleServerMessage(msg, socketInfo);
            return DispatchStatus::MessageHandled;
          }));
        }));

    // MessageDispatcher does not accept yet asynchronous handlers, so we have to explicitly wait
    // for the result.
    socketInfo.setServerMessageHandler([=](const Message& msg) mutable noexcept {
      auto logWarningReturnError = [](const std::string& errorMsg) {
        qiLogWarning() << "Server message handler raised an exception: " << errorMsg;
        return DispatchStatus::MessageHandled_WithError;
      };
      return ka::invoke_catch(
        ka::compose(logWarningReturnError, ka::exception_message_t{}),
        [&] {
          return asyncServerMessageHandler(msg).value();
        });
    });

    // Start reading messages from this socket.
    const auto started = socket->ensureReading();
    success = true;
    return started;
  }

  bool Server::handleServerMessage(const Message& msg, SocketInfo& socketInfo)
  {
    QI_LOG_DEBUG_SERVER() << "Handling message " << msg.address() << " from socket "
                          << socketInfo.socket() << ".";

    // If the socket is already authenticated, simply ignore its messages directed to the server.
    if (socketInfo.isAuthenticated())
    {
      // Still reply if it was a call to avoid having a hanging client.
      if (isAuthRequest(msg))
        return sendSuccessfulAuthReply(socketInfo, makeReply(msg.address()));
      return false;
    }

    QI_LOG_DEBUG_SERVER() << std::boolalpha
                 << "The message is an authentication request: " << isAuthRequest(msg)
                 << ", the server enforces authentication: " << _enforceAuth << ".";

    return _enforceAuth ? handleServerMessageAuth(msg, socketInfo) :
                          handleServerMessageNoAuth(msg, socketInfo);
  }

  bool Server::handleServerMessageAuth(const Message& msg, Server::SocketInfo& socketInfo)
  {
    QI_ASSERT_TRUE(_enforceAuth);

    if (!isAuthRequest(msg))
    {
      // If we enforce authentication, then the received message is unexpected (we expected an
      // authentication request but we received something else), send an error back to the
      // caller.
      std::stringstream err;
      err << "Expected authentication (service #" << Message::Service_Server << ", type #"
          << Message::Type_Call << ", action #" << Message::ServerFunction_Authenticate
          << "), received service #" << msg.service() << ", type #" << msg.type()
          << ", action #" << msg.function();
      return sendAuthError(err.str(), *socketInfo.socket(), makeReply(msg.address())).errorSent;
    }

    return authenticateSocket(msg, socketInfo, makeReply(msg.address()));
  }

  bool Server::handleServerMessageNoAuth(const Message& msg, Server::SocketInfo& socketInfo)
  {
    QI_ASSERT_FALSE(_enforceAuth);

    // Since we do not enforce authentication, consider the socket as authenticated.
    finalizeSocketAuthentication(socketInfo);

    // The message is an authentication request but we do not enforce it, just act as if the
    // authentication was successful.
    if (isAuthRequest(msg))
      return sendSuccessfulAuthReply(socketInfo, makeReply(msg.address()));

    // Otherwise, the current message is unexpected (it's not an authentication message), we do
    // not know what to do with it, so just send back the socket capabilities.
    else
      return sendCapabilitiesMessage(socketInfo);
  }

  bool Server::authenticateSocket(const qi::Message& msg,
                                  SocketInfo& socketInfo,
                                  Message reply)
  {
    const auto socketPtr = socketInfo.socket();
    QI_ASSERT_NOT_NULL(socketPtr);

    auto authData = msg.value(typeOf<CapabilityMap>()->signature(), socketPtr).to<CapabilityMap>();

    // Insert the IP of the remote endpoint.
    auto maybeRemoteEndpoint = socketPtr->remoteEndpoint();
    if(maybeRemoteEndpoint)
    {
      authData[AuthProvider::UserAuthPrefix + "remoteEndpoint"] =
        AnyValue::from<Url>(*maybeRemoteEndpoint);
    }

    auto authResult = socketInfo.authProvider()->processAuth(std::move(authData));
    const auto sendAuthResult = [&] {
      return sendAuthReply(std::move(authResult), socketInfo, std::move(reply));
    };

    const unsigned int state = authResult[AuthProvider::State_Key].to<unsigned int>();
    switch (state)
    {
      case AuthProvider::State_Cont:
        return sendAuthResult();
      case AuthProvider::State_Done:
        qiLogVerbose() << "Client " << socketPtr->remoteEndpoint().value().str()
                       << " successfully authenticated.";
        finalizeSocketAuthentication(socketInfo);
        return sendAuthResult();
      case AuthProvider::State_Error:
      default:
      {
        std::stringstream builder;
        builder << "Authentication failed";
        if (authResult.find(AuthProvider::Error_Reason_Key) != authResult.end())
        {
          builder << ": " << authResult[AuthProvider::Error_Reason_Key].to<std::string>();
          auto authProviderFactory = _state.authProviderFactory;
          builder << " [" << authProviderFactory->authVersionMajor() << "."
                  << authProviderFactory->authVersionMinor() << "]";
        }
        return sendAuthError(builder.str(), *socketPtr, std::move(reply)).errorSent;
      }
    }
  }

  void Server::finalizeSocketAuthentication(SocketInfo& socketInfo) noexcept
  {
    QI_LOG_DEBUG_SERVER() << "Finalizing authentication for socket " << socketInfo.socket() << ".";

    // We must validate the socket (and bind the objects to the socket) before returning the success
    // of the authentication to the client, otherwise it might send us a request for the object
    // before we could bind it, which would make it a dangling request.
    _state.binder.validateSocket(socketInfo.socket());
    socketInfo.setAuthenticated();
  }

  bool Server::sendAuthReply(CapabilityMap authResult, SocketInfo& socketInfo, Message reply)
  {
    auto socket = socketInfo.socket();
    QI_ASSERT_NOT_NULL(socket);

    qiLogDebug() << "Sending authentication reply ("
                 << authResult.at(AuthProvider::State_Key).to<unsigned int>() << ") to socket "
                 << socket << ".";

    const auto capa = socketInfo.extractCapabilities();
    authResult.insert(capa.begin(), capa.end());
    std::string cmsig = typeOf<CapabilityMap>()->signature().toString();
    reply.setValue(authResult, cmsig);
    return socket->send(std::move(reply));
  }


  bool Server::sendSuccessfulAuthReply(SocketInfo& socketInfo, Message reply)
  {
    CapabilityMap authResult;
    authResult[AuthProvider::State_Key] = AnyValue::from<unsigned int>(AuthProvider::State_Done);
    return sendAuthReply(std::move(authResult), socketInfo, std::move(reply));
  }

  Server::SendAuthErrorResult Server::sendAuthError(std::string error,
                                                    MessageSocket& socket,
                                                    Message reply)
  {
    qiLogVerbose() << "Sending an authentication error '" << error << "' to socket " << &socket << ".";
    reply.setType(Message::Type_Error);
    reply.setError(std::move(error));
    auto errorSent = socket.send(std::move(reply));
    auto disconnected = socket.disconnect().async();
    return { errorSent, disconnected };
  }

  bool Server::sendCapabilitiesMessage(SocketInfo& socketInfo)
  {
    const auto socket = socketInfo.socket();
    QI_ASSERT_NOT_NULL(socket);

    qiLogDebug() << "Sending local capabilities to " << &socket << ".";
    Message msg;
    msg.setType(Message::Type_Capability);
    msg.setService(Message::Service_Server);
    msg.setValue(socketInfo.extractCapabilities(), typeOf<CapabilityMap>()->signature());
    return socket->send(std::move(msg));
  }

  Server::RemoveSocketResult Server::removeSocket(const MessageSocketPtr& socket)
  {
    QI_ASSERT_NOT_NULL(socket);
    const auto removed = _state.binder.removeSocket(socket);
    const auto disconnected = socket->disconnect().async();
    return { removed, disconnected };
  }

  Future<void> Server::close()
  {
    closeImpl();
    return futurize();
  }

  qi::Future<void> Server::listen(const qi::Url& address)
  {
    // Assume this means we are back on-line.
    return open()
        .andThen(FutureCallbackType_Sync,
                 [=](bool) {
                   return _server.listen(address);
                 })
        .unwrap();
  }

  Future<std::vector<qi::Uri>> Server::endpoints() const
  {
    return safeCall([=] {
      return _server.endpoints();
    });
  }

  Future<bool> Server::open()
  {
    boost::shared_ptr<Strand> empty;
    return futurize(boost::atomic_compare_exchange(&_strand, &empty, boost::make_shared<Strand>()));
  }
}
