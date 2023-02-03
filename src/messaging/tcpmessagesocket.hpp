#pragma once

#ifndef _SRC_TCPMESSAGESOCKET_HPP_
#define _SRC_TCPMESSAGESOCKET_HPP_

#include <string>
#include <functional>
#include <memory>
#include <mutex>
#include <boost/make_shared.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/optional.hpp>
#include <boost/predef.h>
#include <boost/thread/synchronized_value.hpp>
#include <ka/typetraits.hpp>
#include <ka/macroregular.hpp>
#include <qi/url.hpp>
#include <qi/assert.hpp>
#include <qi/detail/future_fwd.hpp>
#include <qi/messaging/tcpscheme.hpp>
#include "message.hpp"
#include "messagedispatcher.hpp"
#include "messagesocket.hpp"
#include "sock/disconnectedstate.hpp"
#include "sock/disconnectingstate.hpp"
#include "sock/connectingstate.hpp"
#include "sock/connectedstate.hpp"
#include "sock/macrolog.hpp"
#include "sock/networkasio.hpp"

/// @file
/// Contains a socket to send and receive qi::Messages, and the types representing
/// the states a socket can be in : disconnected, connecting, connected, disconnecting.

namespace qi {

  /// Sets cipher list.
  ///
  /// Throws `std::runtime_error` if the cipher list couldn't be set.
  ///
  /// Network N
  template<typename N>
  void setCipherListTls12AndBelow(sock::SslContext<N>& c, const char* cipherList)
  {
    if (!N::trySetCipherListTls12AndBelow(c, cipherList))
    {
      throw std::runtime_error(
        std::string("SSL context: Could not set cipher list: ") + cipherList);
    }
  }

  boost::optional<Seconds> getTcpPingTimeout(Seconds defaultTimeout);

  template<typename N, typename S>
  class TcpMessageSocket;

  namespace sock
  {
    /// Functor that consumes and forwards the message to a TcpMessageSocket.
    ///
    /// Precondition: The socket pointer must be valid.
    /// You should protect the socket if necessary.
    ///
    /// Network N,
    /// With NetSslSocket S:
    ///   S is compatible with N
    template<typename N, typename S>
    struct HandleMessage
    {
      boost::shared_ptr<TcpMessageSocket<N, S>> _tcpSocket;
      bool operator()(const sock::ErrorCode<N>& erc, Message* msg)
      {
        QI_LOG_DEBUG_SOCKET(_tcpSocket.get()) << "Message received "
                                              << ((!erc && msg) ? msg->id() : 0);
        return !erc && msg && _tcpSocket->handleMessage(std::move(*msg));
      }
    };

    const int defaultTimeoutInSeconds = 30;
  } // namespace sock

  /// A socket to send and receive messages.
  ///
  /// # General kinematics
  ///
  /// The socket has four states : disconnected, connecting, connected, disconnecting.
  /// At any moment, the socket is in only one of these states.
  ///
  /// The typical progression is :
  /// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  /// -> disconnected -> connecting -> connected -> disconnecting
  ///         ^                                           |
  ///         |___________________________________________|
  /// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  ///
  /// # Possible actions depending on the state
  ///
  /// - disconnected: connect() (client-side), status()
  /// - connecting: disconnect(), status(), ensureReading() (server-side)
  /// - connected: disconnect(), send(), status(), remoteEndpoint(), url()
  /// - disconnecting: status()
  ///
  /// Each state is represented by a specific type, that handles the available
  /// actions and the associated data.
  ///
  /// # Client-side and server-side usage
  ///
  /// The socket has two usages : the client-side one and the server-side one.
  ///
  /// ## Client-side cinematic:
  ///
  /// On client-side, you construct the socket without passing an underlying socket.
  /// The socket is then in the disconnected state.
  /// Then you call connect() to enter the connecting state. connect() returns
  /// a future that is set when the socket is in the connected state.
  /// Also the `connected` signal is emitted when the socket enter the connected
  /// state.
  /// You can then send messages and receive them with the `messageReady` signal.
  /// You can finally disconnect. The socket then enter the disconnecting state
  /// and you are notified when entering the disconnect state, either by the future
  /// returned by disconnect() or by the `disconnected` signal.
  /// Note that if an error happens in the connected state while sending or receiving
  /// messages, the socket enter the disconnecting and disconnected states.
  ///
  /// ## Server-side cinematic:
  ///
  /// On server-side, you construct the socket by passing an underlying socket.
  /// This underlying socket is typically obtained by the `accept` system call.
  /// The underlying socket is technically connected by the message socket is
  /// nonetheless set in the connecting state.
  /// This is because we generally don't want to receive and send messages immediately.
  /// We'd rather first connect signals and perform other tasks.
  /// Then, you call ensureReading() to enter the connected state.
  /// Note that if you specified `SSL` at construction, the SSL handshake already
  /// happened at construction, so that ensureReading() is synchronous.
  ///
  /// # The Network template parameter
  ///
  /// ## Concept
  ///
  /// `Network` is a concept defined in traits.hpp.
  /// It defines all the low-level types and methods related to the underlying
  /// socket.
  ///
  /// The types are for example:
  ///   resolver, underlying socket, error code, io service, etc.
  ///
  /// Most types have themselves associated types :
  ///   query for resolver, handshake side for underlying socket, etc.
  ///
  /// The methods are for example:
  ///   asynchrounous read, asynchrounous write, buffer create, etc.
  ///
  /// ## Models
  ///
  /// For production code, the Network type used really performs network operations.
  /// `NetworkAsio` is one such model that used Boost.Asio.
  ///
  /// For unit tests, another type is used, for example `NetworkMock` that allows
  /// to cause network errors on demand.
  ///
  /// ## Traits
  ///
  /// You can get the types associated to the Network type with traits.
  ///
  /// For example :
  /// - Resolver<N> is the type of the resolver for a Network type named N.
  /// - Query<Resolver<N>> is the type of query of the resolver.
  ///
  /// # Usage examples
  ///
  /// Example: client-side
  /// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  /// auto socket = makeMessageSocket();
  /// socket.messageReady.connect([](const Message& msg) {
  ///   // treat message
  /// });
  /// socket->connect(Url{"tcps://10.11.12.13:9876"}).then([](Future<void> fut) {
  ///   if (fut.hasError()) {
  ///     // treat error
  ///   }
  ///   socket->send(msg);
  ///   socket->disconnect();
  /// });
  /// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  ///
  /// Example: server-side
  /// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  /// auto socket = boost::make_shared<TcpMessageSocket<>>(underlyingSocket, TcpScheme::Tls, io);
  /// // connect signals
  /// socket->ensureReading();
  /// socket->disconnect();
  /// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  ///
  /// # Notes
  ///
  /// If the socket has not been disconnected, it is synchronously disconnected
  /// on destruction.
  ///
  /// Network N,
  /// With NetSslSocket S:
  ///   S is compatible with N
  template<typename N = sock::NetworkAsio, typename S = sock::SocketWithContext<N>>
  class TcpMessageSocket
    : public MessageSocket
    , public boost::enable_shared_from_this<TcpMessageSocket<N, S>>
  {
  public:
    using SocketPtr = sock::SocketPtr<S>;
    using Handshake = sock::HandshakeSide<S>;
    using Method = sock::Method<sock::SslContext<N>>;
    using boost::enable_shared_from_this<TcpMessageSocket<N, S>>::shared_from_this;
    friend struct sock::HandleMessage<N, S>;

    /// Creates a client side socket. The connection is done by calling `connect(Url)`.
    explicit TcpMessageSocket(sock::IoService<N>& io = N::defaultIoService(),
                              ssl::ClientConfig sslConfig = {});

    /// Creates a server side socket. If the TCP scheme denotes a TLS scheme,
    /// then a SSL handshake is immediately performed.
    /// @throws `std::runtime_error` if the socket pointer is null.
    explicit TcpMessageSocket(SocketPtr socket,
                              TcpScheme scheme = TcpScheme::Raw,
                              sock::IoService<N>& io = N::defaultIoService());

    virtual ~TcpMessageSocket();

    /// Connects the socket as a client and ensures that it is reading.
    FutureSync<void> connect(const Url &url) override;

    /// Stops reading the socket and disconnects it.
    FutureSync<void> disconnect() override
    {
      return doDisconnect();
    }

    /// Returns `true` if we could ask to send the message.
    /// One failure case (return `false`) is when the socket is not connected.
    bool send(Message msg) override;

    Status status() const override
    {
      boost::recursive_mutex::scoped_lock lock(_stateMutex);
      return getStatus();
    }

    /// The remote endpoint is known only if the socket is connected.
    /// Otherwise, an empty optional is returned.
    boost::optional<Url> remoteEndpoint() const override
    {
      boost::recursive_mutex::scoped_lock lock(_stateMutex);
      if (getStatus() == Status::Connected)
      {
        return asConnected(_state).remoteEndpoint(_scheme);
      }
      return {};
    }
    bool ensureReading() override;
  private:
    /// Handler called when we transition outside the connected state.
    /// It is the responsibility of the caller to ensure the socket pointer is
    /// valid.
    struct OnConnectedComplete
    {
      boost::shared_ptr<TcpMessageSocket> _socket;
      Future<void> _futureConnected;
    // Regular:
      KA_GENERATE_FRIEND_REGULAR_OPS_1(OnConnectedComplete, _socket)
    // Procedure:
      void operator()(Future<sock::SyncConnectedResultPtr<N, S>> f)
      {
        _futureConnected.wait();
        // Here, connected state is over, successfully (user asked for) or not.
        QI_LOG_DEBUG_SOCKET(_socket.get()) << "Exiting connected state.";

        const sock::ConnectedResult<N, S> res = f.value()->get(); // copy the result
        if (res.hasError)
        {
          QI_LOG_DEBUG_SOCKET(_socket.get()) << "socket exited connected state: " << res.errorMessage;
        }
        _socket->enterDisconnectedState(res.socket, res.disconnectedPromise);
      }
    };

    TcpScheme _scheme = TcpScheme::Raw;
    mutable boost::recursive_mutex _stateMutex;
    sock::IoService<N>& _ioService;
    const boost::optional<ssl::ClientConfig> _clientSslConfig;

    void enterDisconnectedState(const SocketPtr& socket = {},
      Promise<void> promiseDisconnected = Promise<void>{});

    using DisconnectedState = sock::Disconnected<N>;
    using ConnectingState = sock::Connecting<N, S>;
    using ConnectedState = sock::Connected<N, S>;
    using DisconnectingState = sock::Disconnecting<N, S>;

    // Do not change the order : it must match the Status enumeration order.
    using State = boost::variant<DisconnectedState, ConnectingState, ConnectedState, DisconnectingState>;
    State _state;

    bool mustTreatAsServerAuthentication(const Message& msg) const;
    bool handleCapabilityMessage(const Message& msg);
    bool handleNormalMessage(Message msg);
    bool handleMessage(Message msg);

    Future<void> dispatchOrSendError(Message msg);

    ConnectedState& asConnected(State& s)
    {
      return boost::get<ConnectedState>(s);
    }
    const ConnectedState& asConnected(const State& s) const
    {
      return boost::get<ConnectedState>(s);
    }
    ConnectingState& asConnecting(State& s)
    {
      return boost::get<ConnectingState>(s);
    }
    const ConnectingState& asConnecting(const State& s) const
    {
      return boost::get<ConnectingState>(s);
    }
    const DisconnectingState& asDisconnecting(State& s) const
    {
      return boost::get<DisconnectingState>(s);
    }
    /// _state must be synchronized before calling this method.
    Status getStatus() const
    {
      return static_cast<Status>(_state.which());
    }
    FutureSync<void> doDisconnect();
  };

  template<typename N, typename S>
  using TcpMessageSocketPtr = boost::shared_ptr<TcpMessageSocket<N, S>>;

  template<typename N, typename S>
  TcpMessageSocket<N, S>::TcpMessageSocket(sock::IoService<N>& io,
                                           ssl::ClientConfig sslConfig)
    : MessageSocket()
    , _ioService(io)
    , _clientSslConfig(std::move(sslConfig))
    , _state{ DisconnectedState{} }
  {
  }

  template<typename N, typename S>
  TcpMessageSocket<N, S>::TcpMessageSocket(SocketPtr socket,
                                           TcpScheme scheme,
                                           sock::IoService<N>& io)
    : MessageSocket()
    , _scheme(scheme)
    , _ioService(io)
    , _state{ DisconnectedState{} }
  {
    if (!socket)
      throw std::runtime_error("null server socket");

    sock::setSocketOptions<N>(socket, getTcpPingTimeout(Seconds{sock::defaultTimeoutInSeconds}));
    _state = ConnectingState{io, isWithTls(scheme), socket, Handshake::server};
  }

  template<typename N, typename S>
  TcpMessageSocket<N, S>::~TcpMessageSocket()
  {
    // We are in the destructor, so no concurrency problem.
    if (getStatus() == Status::Connected)
    {
      doDisconnect().async().wait();
      QI_LOG_VERBOSE_SOCKET(this) << "deleted";
    }
  }

  std::uint32_t getMaxPayloadFromEnv(std::uint32_t defaultValue = std::numeric_limits<std::uint32_t>::max());

  /// Start receiving messages. Also allows to send messages.
  ///
  /// The returned value indicates if the operation succeeded.
  /// To succeed, the object must be in the `connecting` state.
  /// This is typically obtained by first constructing the object with a
  /// connected socket,
  template<typename N, typename S>
  bool TcpMessageSocket<N, S>::ensureReading()
  {
    static const auto maxPayload = getMaxPayloadFromEnv();
    {
      boost::recursive_mutex::scoped_lock lock(_stateMutex);
      if (getStatus() != Status::Connecting)
      {
        QI_LOG_VERBOSE_SOCKET(this) << "ensureReading: socket must be in connecting state.";
        return false;
      }

      const auto res = [&]{ // copy the result
        auto syncRes = asConnecting(_state).complete().value()->defer_synchronize();
        lock.unlock();
        std::lock(lock, syncRes);
        return *syncRes;
      }();
      if (hasError(res))
      {
        QI_LOG_WARNING_SOCKET(this) << "ensureReading: connection error: " << res.errorMessage;
        enterDisconnectedState(res.socket, res.disconnectedPromise);
        lock.unlock();
        res.disconnectedPromise.future().wait();
        return false;
      }
      auto self = shared_from_this();
      _state = ConnectedState(res.socket, isWithTls(_scheme), maxPayload, sock::HandleMessage<N, S>{self});
      auto& connected = asConnected(_state);
      connected.complete().then(connected.ioServiceStranded(
        OnConnectedComplete{self, Future<void>{nullptr}}
      ));
    }
    connected();
    return true;
  }

  /// Connect the socket to start receiving and sending messages.
  ///
  /// The operation completes when the returned future is set.
  ///
  /// The object must be in the `disconnected` state.
  ///
  /// Returns a future in error if the URL is invalid or if the scheme
  /// of the URL is not one of our supported TCP schemes (see the
  /// `qi::TcpScheme` type).
  template<typename N, typename S>
  FutureSync<void> TcpMessageSocket<N, S>::connect(const Url& url)
  {
    static const bool disableIpV6 = os::getenv("QIMESSAGING_ENABLE_IPV6").empty();
    Promise<void> connectedPromise;
    boost::recursive_mutex::scoped_lock lock(_stateMutex);

    if (getStatus() != Status::Disconnected)
    {
      QI_LOG_WARNING_SOCKET(this) << "connect() but status is " << static_cast<int>(getStatus());
      return ConnectingState::connectError("Must be disconnected to connect().");
    }

    // Return an error if the URL is invalid.
    // Both the host and the port fields are required to attempt a connection.
    // The resolver is guaranteed to fail later if the URL lacks a host and
    // the connection will fail if the URL lacks a port. The scheme is
    // theoretically optional but it is later checked if the URL is valid (meaning
    // it includes a scheme) and an error is returned if it is not.
    if (!url.isValid())
    {
      const auto error = std::string("invalid URL '") + url.str() + "' used to connect, "
                           "this connection will result in an error";
      return ConnectingState::connectError(error);
    }

    // Return an error if the scheme of the URL is not one of our TCP schemes.
    const auto optTcpScheme = tcpScheme(url);
    if (!optTcpScheme)
      return ConnectingState::connectError("unsupported URL TCP scheme '" + url.str() + "'");

    const auto scheme = *optTcpScheme;

    using Side = sock::HandshakeSide<S>;

    auto makeSocket = std::bind(
      InvokeCatchLogRethrowError{},
      sock::logCategory(),       // log category
      "could not create socket", // log prefix
      [this, url, scheme]() {
        // The lowest authorized protocol is TLS1.2. This means SSL protocols
        // and TLS 1.1 are forbidden.
        auto contextPtr = sock::makeSslContextPtr<N>(Method::tlsv12);
        setCipherListTls12AndBelow<N>(*contextPtr, N::clientCipherList());
        contextPtr->set_options(
            sock::SslContext<N>::no_sslv2
          | sock::SslContext<N>::no_sslv3
          | sock::SslContext<N>::no_tlsv1
          | sock::SslContext<N>::no_tlsv1_1
        );
        const auto mode = sock::sslVerifyMode<N>(Side::client, scheme);
        contextPtr->set_verify_mode(mode);
        auto hostName = url.host();
        QI_ASSERT(!hostName.empty());
        N::applyConfig(*contextPtr, *_clientSslConfig, std::move(hostName));
        return sock::makeSocketWithContextPtr<N>(_ioService, contextPtr);
      }
    );

    _state =
        ConnectingState{ _ioService, url, std::move(makeSocket), !disableIpV6, Side::client,
          getTcpPingTimeout(Seconds{ sock::defaultTimeoutInSeconds }) };
    _scheme = scheme;
    auto self = shared_from_this();

    asConnecting(_state).complete().then([=](
        Future<sock::SyncConnectingResultPtr<N, S>> fut) mutable {
      // Here, connecting is over, successfully or not.
      {
        boost::recursive_mutex::scoped_lock lock(_stateMutex, boost::defer_lock);
        const auto res = [&]{ // copy the result
          auto syncRes = fut.value()->defer_synchronize();
          std::lock(lock, syncRes);
          return *syncRes;
        }();
        QI_ASSERT_TRUE(getStatus() == Status::Connecting);
        const bool connectingFailed = hasError(res);
        if (res.disconnectionRequested || connectingFailed)
        {
          connectedPromise.setError(res.disconnectionRequested
            ? "Connect abort: disconnection requested while connecting"
            : "Connect error: " + res.errorMessage);
          enterDisconnectedState(res.socket, res.disconnectedPromise);
          return;
        }
        // Connecting was successful, so we enter the connected state (to be able
        // send and receive messages).
        static const auto maxPayload = getMaxPayloadFromEnv();
        _state = ConnectedState(res.socket, isWithTls(_scheme), maxPayload, sock::HandleMessage<N, S>{self});
        auto& connected = asConnected(_state);
        connected.complete().then(connected.ioServiceStranded(
          OnConnectedComplete{self, connectedPromise.future()}
        ));
      }
      // If we exit the connected state before arriving here (because of an
      // error or because an explicit disconnection) the user might receive the
      // `connected` signal after the `disconnected` one. Since signals are not
      // ordered anyway, it may not be a real problem.
      // We emit `connected` here to avoid doing it while locking.
      QI_LOG_DEBUG_SOCKET(self.get()) << "Emitting `connected` signal";
      connected();
      QI_LOG_DEBUG_SOCKET(self.get()) << "Setting `connected` promise";
      connectedPromise.setValue(nullptr);
      QI_LOG_DEBUG_SOCKET(self.get()) << "socket connected to " << url.str();
    });
    return connectedPromise.future();
  }

  /// You must wait for this method to complete before
  /// destructing the socket.
  template<typename N, typename S>
  FutureSync<void> TcpMessageSocket<N, S>::doDisconnect()
  {
    Promise<void> promiseDisconnected;
    boost::recursive_mutex::scoped_lock lock(_stateMutex);
    switch (getStatus())
    {
    case Status::Disconnected:
      // libqi's code heavily relies on socket disconnection being reentrant...
      promiseDisconnected.setValue(nullptr);
      break;
    case Status::Connecting:
      asConnecting(_state).stop(promiseDisconnected);
      break;
    case Status::Connected:
      QI_LOG_DEBUG_SOCKET(this) << "Requesting connected socket to disconnect.";
      asConnected(_state).stop(promiseDisconnected);
      break;
    case Status::Disconnecting:
      return asDisconnecting(_state).complete();
    }
    return promiseDisconnected.future();
  }

  template<typename N, typename S>
  void TcpMessageSocket<N, S>::enterDisconnectedState(const SocketPtr& socket, Promise<void> promiseDisconnected)
  {
    bool wasConnected = false;
    {
      DisconnectingState disconnect{socket, promiseDisconnected};
      {
        boost::recursive_mutex::scoped_lock lock(_stateMutex);
        wasConnected = (getStatus() == Status::Connected);
        QI_LOG_DEBUG_SOCKET(socket.get()) << "Entering Disconnecting state";
        _state = disconnect;
      }
      disconnect();
      auto self = shared_from_this();
      disconnect.complete().then([=](Future<void> fut) mutable {
        if (fut.hasError())
        {
          QI_LOG_WARNING_SOCKET(socket.get()) << "Error while disconnecting: " << fut.error();
        }
        {
          boost::recursive_mutex::scoped_lock lock(self->_stateMutex);
          self->_state = DisconnectedState{};
          QI_LOG_DEBUG_SOCKET(socket.get()) << "Socket disconnected.";
        }
        static const std::string data{"disconnected"};
        if (wasConnected)
        {
          QI_LOG_DEBUG_SOCKET(socket.get()) << "Emitting `disconnected` signal.";
          self->disconnected(data);
        }
        QI_LOG_DEBUG_SOCKET(socket.get()) << "Emitting `socketEvent` signal.";
        self->socketEvent(SocketEventData(data));
        QI_LOG_DEBUG_SOCKET(socket.get()) << "Setting disconnected promise.";
        promiseDisconnected.setValue(nullptr);
      });
    }
  }

  template<typename N, typename S>
  bool TcpMessageSocket<N, S>::mustTreatAsServerAuthentication(const Message& msg) const
  {
    return !hasReceivedRemoteCapabilities()
        && msg.service() == Message::Service_Server
        && msg.function() == Message::ServerFunction_Authenticate;
  }

  template<typename N, typename S>
  bool TcpMessageSocket<N, S>::handleCapabilityMessage(const Message& msg)
  {
    try
    {
      CapabilityMap cm;
      {
        AnyValue v{msg.value(typeOf<CapabilityMap>()->signature(), shared_from_this())};
        cm = v.to<CapabilityMap>();
      }
      boost::mutex::scoped_lock lock(_contextMutex);
      _remoteCapabilityMap.insert(cm.begin(), cm.end());
    }
    catch (const std::runtime_error& e)
    {
      QI_LOG_ERROR_SOCKET(this) << "Ill-formed capabilities message: " << e.what();
      return false;
    }
    return true;
  }

  template<typename N, typename S>
  bool TcpMessageSocket<N, S>::handleNormalMessage(Message msg)
  {
    messageReady(msg);
    socketEvent(SocketEventData(msg));
    dispatchOrSendError(std::move(msg));
    return true;
  }

  template<typename N, typename S>
  bool TcpMessageSocket<N, S>::handleMessage(Message msg)
  {
    bool success = false;
    if (mustTreatAsServerAuthentication(msg) || msg.type() == Message::Type_Capability)
    {
      if (msg.type() != Message::Type_Error)
      {
        success = handleCapabilityMessage(msg);
      }
      if (success && msg.type() != Message::Type_Capability)
      {
        success = handleNormalMessage(std::move(msg));
      }
    }
    else
    {
      success = handleNormalMessage(std::move(msg));
    }
    return success;
  }

  template<typename N, typename S>
  Future<void> TcpMessageSocket<N, S>::dispatchOrSendError(Message msg)
  {
    const auto header = msg.header();
    auto fut = _dispatcher.dispatch(std::move(msg));

    // Call request expects a reply and if the client doesn't get it, it might block. To avoid that,
    // if no handler was able to process the message, send back an error informing the client.
    if (header.messageType() == Message::Type_Call)
    {
      const MessageAddress msgAddress = header.address();
      return fut
        .andThen(ka::scope_lock_proc(
          [msgAddress, this](bool handled) {
            if (!handled)
            {
              Message errorMsg(Message::Type_Error, msgAddress);
              errorMsg.setError("The call request could not be handled.");
              send(errorMsg);
            }
          },
          ka::mutable_store(this->weak_from_this())))
        .andThen([](ka::opt_t<void>) {});
    }

    return fut.andThen([](bool){});
  }

  template<typename N, typename S>
  bool TcpMessageSocket<N, S>::send(Message msg)
  {
    boost::recursive_mutex::scoped_lock lock(_stateMutex);
    if (getStatus() != Status::Connected)
    {
      QI_LOG_DEBUG_SOCKET(this) << "Socket must be connected to send().";
      return false;
    }
    // NOTE: Should we specify an `onSent` callback and stop sending if an error
    // occurred?
    asConnected(_state).send(std::move(msg), isWithTls(_scheme));
    return true;
  }

  /// Network N,
  /// With NetSslSocket S:
  ///   S is compatible with N
  template <typename N = sock::NetworkAsio, typename S = sock::SocketWithContext<N>>
  TcpMessageSocketPtr<N, S> makeTcpMessageSocket(ssl::ClientConfig sslConfig = {},
                                                 EventLoop* eventLoop = getNetworkEventLoop())
  {
    using Socket = TcpMessageSocket<N, S>;
    return boost::make_shared<Socket>(*asIoServicePtr(eventLoop), std::move(sslConfig));
  }

} // namespace qi

#endif  // _SRC_TCPMESSAGESOCKET_HPP_
