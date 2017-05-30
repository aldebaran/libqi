#pragma once

#ifndef _SRC_TCPMESSAGESOCKET_HPP_
#define _SRC_TCPMESSAGESOCKET_HPP_

#include <string>
#include <functional>
#include <memory>
#include <mutex>
#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/make_shared.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/optional.hpp>
#include <boost/predef.h>
#include <boost/thread/synchronized_value.hpp>
#include <qi/api.hpp>
#include "message.hpp"
#include <qi/url.hpp>
#include <qi/trackable.hpp>
#include <qi/signal.hpp>
#include <qi/type/traits.hpp>
#include <qi/macroregular.hpp>
#include "messagedispatcher.hpp"
#include "messagesocket.hpp"
#include <qi/messaging/net/networkasio.hpp>
#include <qi/messaging/net/traits.hpp>
#include <qi/messaging/net/common.hpp>
#include <qi/messaging/net/resolve.hpp>
#include <qi/messaging/net/connect.hpp>
#include <qi/messaging/net/receive.hpp>
#include <qi/messaging/net/send.hpp>

/// @file
/// Contains a socket to send and receive qi::Messages, and the types representing
/// the states a socket can be in : disconnected, connecting, connected, disconnecting.

namespace qi {

#define QI_LOG_ERROR_SOCKET(SOCKET_PTR) qiLogError(net::logCategory()) << (SOCKET_PTR) << ": "
#define QI_LOG_WARNING_SOCKET(SOCKET_PTR) qiLogWarning(net::logCategory()) << (SOCKET_PTR) << ": "
#define QI_LOG_INFO_SOCKET(SOCKET_PTR) qiLogInfo(net::logCategory()) << (SOCKET_PTR) << ": "
#define QI_LOG_DEBUG_SOCKET(SOCKET_PTR) qiLogDebug(net::logCategory()) << (SOCKET_PTR) << ": "

  boost::optional<Seconds> getTcpPingTimeout(Seconds defaultTimeout);

  template<typename N>
  class TcpMessageSocket;

  /// Functor that forwards the message to a TcpMessageSocket.
  ///
  /// Precondition: The socket pointer must be valid.
  /// You should protect the socket if necessary.
  ///
  /// Network N
  template<typename N>
  struct HandleMessage
  {
    boost::shared_ptr<TcpMessageSocket<N>> _tcpSocket;
    bool operator()(const net::ErrorCode<N>& erc, const Message* msg)
    {
      QI_LOG_DEBUG_SOCKET(_tcpSocket.get()) << "Message received "
                                            << ((!erc && msg) ? msg->id() : 0);
      return !erc && msg && _tcpSocket->handleMessage(*msg);
    }
  };

  boost::optional<qi::int64_t> getSocketTimeWarnThresholdFromEnv();

  namespace net
  {
    /// Result of the connecting state.
    /// The socket is valid if there was no error and no disconnection was requested.
    ///
    /// Example:
    /// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    /// // res is a ConnectingResult<N>
    /// const bool connectingFailed = hasError(res);
    /// if (connectingFailed || res.disconnectionRequested)
    /// {
    ///   connectedPromise.setError(connectingFailed
    ///     ? "Connect error: " + res.errorMessage
    ///     : "Abort: disconnection requested while connecting");
    ///
    ///   // res.socket is null but it is handled by enterDisconnectedState().
    ///   enterDisconnectedState(res.socket, res.disconnectedPromise);
    ///   return;
    /// }
    /// // res.socket is valid and can be used.
    /// // ...
    /// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    ///
    /// Network N
    template<typename N>
    struct ConnectingResult
    {
      std::string errorMessage;
      SocketPtr<N> socket;
      bool disconnectionRequested = false;
      Promise<void> disconnectedPromise;
      friend void setDisconnectionRequested(ConnectingResult& res, const Promise<void>& p)
      {
        res.disconnectionRequested = true;
        res.disconnectedPromise = p;
      }
      friend bool hasError(const ConnectingResult& x)
      {
        return !x.errorMessage.empty();
      }
    };

    /// Connecting state of the socket.
    /// Connects to a URL and give back the created socket.
    ///
    /// Usage:
    /// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    /// Connecting c{ioService, Url("tcp://1.2.3.4:9876"), SslEnabled{true},
    ///   IpV6Enabled{true}, HandshakeSide::client};
    /// auto socketPtr = c.complete().value();
    /// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    ///
    /// The constructor immediately starts the connecting process, in the
    /// the same manner as std::thread. This way, there is no concurrency
    /// issue and no locking is done.
    /// If any error occurs, the `complete` future is set in error.
    ///
    /// # Lifetime considerations
    ///
    /// The object must be alive until the connecting process is complete.
    /// That is, you must not write:
    /// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    /// Future<SocketPtr<N>> connectNonSsl(IoService& io, Url url) {
    ///   return Connecting(io, url, SslEnabled{false}, IpV6Enabled{true}).complete();
    /// }
    /// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    /// because the connecting process could exceed the ConnectSocket object lifetime.
    /// If the ConnectSocket dies before the end of the connecting process, the future
    /// is set in error.
    ///
    /// # Server-side
    ///
    /// It is also possible to give an already connected socket. In this case,
    /// if needed, only the SSL handshake is done. This is the typical use case
    /// for servers, because the `accept` pattern yields an already connected socket.
    ///
    /// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    /// Connecting c{ioService, SslEnabled{true}, IpV6Enabled{true}, HandshakeSide::server};
    /// auto socketPtr = c.complete().value();
    /// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    ///
    /// # Connecting steps
    ///
    /// From a more technical point of view, the different connecting steps
    /// are:
    /// - URL resolving
    /// - socket connecting
    /// - SSL handshake if needed.
    ///
    /// # Stopping the connection
    ///
    /// Example: stopping the connection
    /// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    /// Promise<void> disconnectedPromise;
    /// connecting.stop(disconnectedPromise);
    /// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    /// The promise will be passed as-is in the "complete" future. The purpose is to
    /// pass it to the next socket state, that will pass it again to the next state
    /// until it is finally set by the disconnected state.
    /// The stop is expected to happen almost immediately.
    ///
    /// # Technicalities of stopping
    ///
    /// The stop can happen in any step: resolving, connecting or handshaking.
    ///
    /// Stopping while resolving results in cancelling the resolver.
    /// Stopping while connecting or while handshaking results in the same
    /// action: closing the socket.
    ///
    /// These stopping actions are performed by continuations on a future. The stop is
    /// effectively triggered when the associated promise is set. This promise is
    /// owned by the `Connecting` object. A procedure is passed to the object that
    /// connects the socket to setup the stop, that is to set the continuations
    /// on the future.
    ///
    /// The promise must be destroyed before the object that connects the socket
    /// to avoid continuations to be called after the destruction of the resolver
    /// or the socket.
    ///
    /// Stopping kinematics:
    /// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    /// -> `Connecting` object construction
    ///     | (1) creates
    ///     |
    ///     |           (3) calls when necessary
    ///     |           ------------------------
    ///     |          |                        |
    ///     v          v     (2) passed to      |
    /// setup-stop procedure -----------> connect-socket object
    ///        |
    ///        | (4) sets
    ///    -------------------------
    ///   |                         |
    ///   v                         v
    ///  cancelling resolver       closing socket
    ///  continuation              continuation (for connect & handshake steps)
    ///   ^                         ^
    ///   |                         |
    ///    -------------------------
    ///                        | (2') triggers
    ///           (1') sets    |
    /// -> stop() ---------> stop promise
    /// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    ///
    /// Network N
    template<typename N>
    struct Connecting
    {
      using Handshake = HandshakeSide<SslSocket<N>>;
      using Result = ConnectingResult<N>;

      struct Impl : std::enable_shared_from_this<Impl>
      {
        using std::enable_shared_from_this<Impl>::shared_from_this;
        Promise<Result> _promiseComplete;
        ConnectSocketFuture<N> _connect;
        Promise<void> _promiseStop; // Must be declared after `_connect`, to be destroyed first
        ConnectingResult<N> _result;
        std::atomic<bool> _stopping;

        void setContinuation()
        {
          auto self = shared_from_this();
          _connect.complete().then(
            [=](const Future<SocketPtr<N>>& fut) { // continuation
              if (fut.hasError())
              {
                self->_result.errorMessage = fut.error();
              }
              else
              {
                self->_result.socket = fut.value();
              }
              self->_promiseComplete.setValue(_result);
            }
          );
        }
        Impl(IoService<N>& io)
          : _connect{io}
          , _stopping{false}
        {
        }

        template<typename Proc = PolymorphicConstantFunction<void>>
        void start(const Url& url, SslEnabled ssl, SslContext<N>& context,
          IpV6Enabled ipV6, Handshake side, const boost::optional<Seconds>& tcpPingTimeout = {},
          Proc setupCancel = Proc{})
        {
          setContinuation();
          _connect(url, ssl, context, ipV6, side, tcpPingTimeout, setupCancel);
        }

        template<typename Proc = PolymorphicConstantFunction<void>>
        void start(SslEnabled ssl, const SocketPtr<N>& s, Handshake side, Proc setupCancel = Proc{})
        {
          setContinuation();
          _connect(ssl, s, side, setupCancel);
        }
      };

      Connecting(IoService<N>& io, const Url& url, SslEnabled ssl, SslContext<N>& context,
          IpV6Enabled ipV6, Handshake side, const boost::optional<Seconds>& tcpPingTimeout = {})
        : _impl(std::make_shared<Impl>(io))
      {
        _impl->start(url, ssl, context, ipV6, side, tcpPingTimeout,
          SetupConnectionStop<N>{_impl->_promiseStop.future()});
      }
      Connecting(IoService<N>& io, SslEnabled ssl, const SocketPtr<N>& s, Handshake side)
        : _impl(std::make_shared<Impl>(io))
      {
        _impl->start(ssl, s, side, SetupConnectionStop<N>{_impl->_promiseStop.future()});
      }
      Future<Result> complete() const
      {
        return _impl->_promiseComplete.future();
      }
      bool stop(const Promise<void>& disconnectedPromise)
      {
        const bool mustStop = tryRaiseAtomicFlag(_impl->_stopping);
        if (mustStop)
        {
          setDisconnectionRequested(_impl->_result, disconnectedPromise);
          _impl->_promiseStop.setValue(nullptr); // triggers the stop
        }
        return mustStop;
      }
      static Future<void> connectError(const std::string& error)
      {
        return makeFutureError<void>(std::string("socket connection: ") + error);
      }
    private:
      std::shared_ptr<Impl> _impl;
    };

    /// Disconnected state of the socket.
    /// It does nothing but is provided for the sake of consistency.
    template<typename N>
    struct Disconnected
    {
    };

    /// The URL of the endpoint of the given socket.
    /// Precondition: The socket must be connected.
    ///
    /// NetSslSocket S
    template<typename S>
    Url remoteEndpoint(S& socket, bool /*ssl*/)
    {
      auto endpoint = socket.lowest_layer().remote_endpoint();
      // Forcing TCP is the legacy behavior.
      // TODO: Change this with `ssl ? "tcps" : "tcp"` when sure of the impact.
      return Url{
        endpoint.address().to_string(),
        "tcp",
        endpoint.port()};
    }

    /// Connected state of the socket.
    /// Allow to send and receive messages.
    ///
    /// Received messages are exposed via a callback specified at construction.
    ///
    /// You can send messages continuously : they will be enqueued if needed.
    /// By default, enqueued messages are sent as soon as possible, but
    /// you can explicitly ask to stop the queue processing.
    ///
    /// You can request this state to stop, by optionally passing it a Promise
    /// to be set on disconnection.
    ///
    /// A `complete` future is set when the connected state is over.
    /// It contains the socket and a Promise to be set when disconnection is over.
    ///
    /// If an error occurs while sending or receiving, the `complete` future is
    /// set in error.
    ///
    /// Warning: You must call `stop()` before closing the socket, because
    /// closing the socket cause the send and receive handlers to be called with
    /// an 'abort' error, and we don't want to take it into account.
    ///
    /// Usage:
    /// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    /// auto onReceive = [](ErrorCode e, const Message* msg) {
    ///   // `msg` is valid only if there is no error.
    ///   // Do something with `msg`.
    ///   return true; // Or return false to stop listening to message.
    /// };
    /// Connected<N> c{socketPtr, SslEnabled{true}, maxPayload, onReceive};
    /// // Get a Message msg;
    /// c.send(msg); // Move the message if you can, to avoid a copy.
    /// // Send more messages.
    /// Future<std::pair<SocketPtr<N>, Promise<void>>> futureComplete = c.complete();
    /// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    ///
    /// Network N
    template<typename N>
    struct Connected
    {
    private:
      struct Impl : std::enable_shared_from_this<Impl>
      {
        using std::enable_shared_from_this<Impl>::shared_from_this;
        using ReadableMessage = typename SendMessageEnqueue<N, SocketPtr<N>>::ReadableMessage;
        using PairSocketPromise = std::pair<SocketPtr<N>, Promise<void>>;

        Impl(const SocketPtr<N>& socket);
        ~Impl();

        template<typename Proc>
        void start(SslEnabled, size_t maxPayload, Proc onReceive, qi::int64_t messageHandlingTimeoutInMus);

        template<typename Msg, typename Proc>
        void send(Msg&& msg, SslEnabled, Proc onSent);

        Promise<PairSocketPromise> _completePromise;
        SocketPtr<N> _socket;
        std::atomic<bool> _stopRequested;
        ReceiveMessageContinuous<N> _receiveMsg;
        SendMessageEnqueue<N, SocketPtr<N>> _sendMsg;
      };
      std::shared_ptr<Impl> _impl;
    public:
      /// If `onReceive` returns `false`, this stops the message receiving.
      ///
      /// Procedure<bool (ErrorCode<N>, const Message*)> Proc
      template<typename Proc>
      Connected(const SocketPtr<N>&, SslEnabled ssl, size_t maxPayload, const Proc& onReceive,
        qi::int64_t messageHandlingTimeoutInMus = getSocketTimeWarnThresholdFromEnv().value_or(0));

      /// If `onSent` returns false, the processing of enqueued messages stops.
      ///
      /// Procedure<bool (ErrorCode<N>, std::list<Message>::const_iterator)>
      template<typename Msg, typename Proc = NoOpProcedure<bool (ErrorCode<N>, std::list<Message>::const_iterator)>>
      void send(Msg&& msg, SslEnabled ssl, const Proc& onSent = {true})
      {
        return _impl->send(std::forward<Msg>(msg), ssl, onSent);
      }
      Future<std::pair<SocketPtr<N>, Promise<void>>> complete() const
      {
        return _impl->_completePromise.future();
      }
      Url remoteEndpoint(SslEnabled ssl) const
      {
        return net::remoteEndpoint(*(_impl->_socket), *ssl);
      }
      void requestStop(Promise<void> promiseDisconnected = Promise<void>{})
      {
        using P = typename Impl::PairSocketPromise;
        auto impl = _impl->shared_from_this();
        ioServiceStranded([=]() mutable {
          auto& promise = impl->_completePromise;
          if (promise.future().isRunning())
          {
            promise.setValue(P{impl->_socket, promiseDisconnected});
          }
          else
          {
            // A verifier.
            promiseDisconnected.setValue(nullptr);
          }
        })();
      }
      template<typename Proc>
      auto ioServiceStranded(Proc&& p)
        -> decltype(StrandTransfo<N>{&_impl->_socket->get_io_service()}(std::forward<Proc>(p)))
      {
        return StrandTransfo<N>{&_impl->_socket->get_io_service()}(std::forward<Proc>(p));
      }
    };

    template<typename N>
    template<typename Proc>
    Connected<N>::Connected(const SocketPtr<N>& socket, SslEnabled ssl, size_t maxPayload,
        const Proc& onReceive, qi::int64_t messageHandlingTimeoutInMus)
      : _impl(std::make_shared<Impl>(socket))
    {
      _impl->start(ssl, maxPayload, onReceive, messageHandlingTimeoutInMus);
    }

    template<typename N>
    Connected<N>::Impl::Impl(const SocketPtr<N>& s)
      : _socket(s)
      , _stopRequested(false)
      , _sendMsg{s}
    {
    }

    template<typename N>
    template<typename Proc>
    void Connected<N>::Impl::start(SslEnabled ssl, size_t maxPayload, Proc onReceive,
        qi::int64_t messageHandlingTimeoutInMus)
    {
      auto self = shared_from_this();
      _receiveMsg(_socket, ssl, maxPayload,
        [=](net::ErrorCode<N> e, const Message* msg) mutable {
          // If a stop is already requested, we must not call the handler.
          if (self->_stopRequested) return false;
          const bool mustContinue = onReceive(e, msg);
          if (!mustContinue)
          {
            self->_stopRequested = true;
            if (self->_completePromise.future().isRunning())
            {
              if (e || !msg) // has error
              {
                self->_completePromise.setError(e.message());
              }
              else
              {
                self->_completePromise.setValue(PairSocketPromise{self->_socket, Promise<void>{}});
              }
            }
            // We must not continue to receive messages.
            return false;
          }
          // Otherwise, we continue to receive messages.
          return true;
        },
        dataBoundTransfo(shared_from_this()), // dataTransfo
        StrandTransfo<N>{&(*_socket).get_io_service()} // netTransfo
      );
    }

    template<typename N>
    Connected<N>::Impl::~Impl()
    {
    }

    template<typename N>
    template<typename Msg, typename Proc>
    void Connected<N>::Impl::send(Msg&& msg, SslEnabled ssl, Proc onSent)
    {
      using SendMessage = decltype(_sendMsg);
      using ReadableMessage = typename SendMessage::ReadableMessage;
      auto self = shared_from_this();
      _sendMsg(std::forward<Msg>(msg), ssl,
        [=](const ErrorCode<N>& e, const ReadableMessage& ptrMsg) mutable {
          // If a stop is already requested, we must not call the handler.
          if (self->_stopRequested) return false;
          const bool mustContinue = onSent(e, ptrMsg);
          if (!mustContinue)
          {
            self->_stopRequested = true;
            if (self->_completePromise.future().isRunning())
            {
              if (e) // has error
              {
                self->_completePromise.setError(e.message());
              }
              else
              {
                self->_completePromise.setValue(PairSocketPromise{_socket, Promise<void>{}});
              }
            }
            // Stop sending pending messages.
            return false;
          }
          // Continue sending pending messages.
          return true;
        },
        dataBoundTransfo(shared_from_this()), // dataTransfo
        StrandTransfo<N>{&(*_socket).get_io_service()} // netTransfo
       );
    }

    /// Disconnecting state of the socket.
    /// Close the socket immediately.
    ///
    /// The disconnected promise is stored only to be passed to the next state.
    /// It is not the responsibility of Disconnecting to set it.
    template<typename N>
    class Disconnecting
    {
      Future<void> _completeFuture;
      Promise<void> _disconnectedPromise;
    public:
      Disconnecting(const SocketPtr<N>& socket, Promise<void> disconnectedPromise)
        : _completeFuture(0)
        , _disconnectedPromise(disconnectedPromise)
      {
        QI_LOG_DEBUG_SOCKET(socket.get()) << "Entering Disconnecting state";
        if (socket)
        {
          _completeFuture = qi::async([=] {
            close<N>(socket);
          });
        }
      }
      // TODO: replace by "= default" when get rid of VS2013.
      Disconnecting(Disconnecting&& x)
        : _completeFuture(std::move(x._completeFuture))
        , _disconnectedPromise(std::move(x._disconnectedPromise))
      {
      }
      // TODO: replace by "= default" when get rid of VS2013.
      Disconnecting& operator=(Disconnecting&& x)
      {
        _completeFuture = std::move(x._completeFuture);
        _disconnectedPromise = std::move(x._disconnectedPromise);
        return *this;
      }
      Future<void> complete() const
      {
        return _completeFuture;
      }
      Future<void> disconnectedPromise() const
      {
        return _disconnectedPromise.future();
      }
    };

    const int defaultTimeoutInSeconds = 30;

  } // namespace net

  /// A socket to send and receive messages.
  ///
  /// # General cinematic
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
  /// auto socket = makeMessageSocket("tcps");
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
  /// auto socket = boost::make_shared<TcpMessageSocket<>>(io, SslEnabled{true}, underlyingSocket);
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
  /// Network N
  template<typename N = net::NetworkAsio>
  class TcpMessageSocket
    : public MessageSocket
    , public boost::enable_shared_from_this<TcpMessageSocket<N>>
  {
  public:
    using SocketPtr = net::SocketPtr<N>;
    using Handshake = net::HandshakeSide<net::SslSocket<N>>;
    using Method = net::Method<net::SslContext<N>>;
    using boost::enable_shared_from_this<TcpMessageSocket<N>>::shared_from_this;
    friend struct HandleMessage<N>;

    /// If the socket is not null, we consider we are on server side.
    /// On server side, if SSL is enabled the connection only consist of the handshake.
    /// On client side (null socket), the connection is done by calling `connect(Url)`.
    explicit TcpMessageSocket(net::IoService<N>& io = N::defaultIoService(),
      net::SslEnabled ssl = {false}, SocketPtr = {});

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
    bool send(const Message &msg) override;

    Status status() const override
    {
      boost::recursive_mutex::scoped_lock lock(_stateMutex);
      return getStatus();
    }

    Url url() const override
    {
      return *_url;
    }

    /// The remote endpoint is known only if the socket is connected.
    /// Otherwise, an empty optional is returned.
    boost::optional<Url> remoteEndpoint() const override
    {
      boost::recursive_mutex::scoped_lock lock(_stateMutex);
      if (getStatus() == Status::Connected)
      {
        return asConnected(_state).remoteEndpoint(_ssl);
      }
      return {};
    }
    bool isConnected() const override
    {
      boost::recursive_mutex::scoped_lock lock(_stateMutex);
      return getStatus() == Status::Connected;
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
      QI_GENERATE_FRIEND_REGULAR_OPS_1(OnConnectedComplete, _socket)
    // Procedure:
      void operator()(Future<std::pair<SocketPtr, Promise<void>>> f)
      {
        _futureConnected.wait();
        // Here, connected state is over, successfully (user asked for) or not.
        QI_LOG_DEBUG_SOCKET(_socket.get()) << "Exiting connected state.";
        if (f.hasError())
        {
          QI_LOG_DEBUG_SOCKET(_socket.get()) << "socket exited connected state: " << f.error();
          _socket->enterDisconnectedState();
        }
        else
        {
          auto socket = f.value().first;
          auto promiseDisconnected = f.value().second;
          _socket->enterDisconnectedState(socket, promiseDisconnected);
        }
      }
    };

    const net::SslEnabled _ssl;
    net::SslContext<N> _sslContext;
    mutable boost::recursive_mutex _stateMutex;
    net::IoService<N>& _ioService;

    void enterDisconnectedState(const SocketPtr& socket = {},
      Promise<void> promiseDisconnected = Promise<void>{});

    using DisconnectedState = net::Disconnected<N>;
    using ConnectingState = net::Connecting<N>;
    using ConnectedState = net::Connected<N>;
    using DisconnectingState = net::Disconnecting<N>;

    // Do not change the order : it must match the Status enumeration order.
    using State = boost::variant<DisconnectedState, ConnectingState, ConnectedState, DisconnectingState>;
    State _state;
    boost::synchronized_value<Url> _url;

    bool mustTreatAsServerAuthentication(const Message& msg) const;
    bool handleCapabilityMessage(const Message& msg);
    bool handleNormalMessage(const Message& msg);
    bool handleMessage(const Message& msg);

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

  using TcpMessageSocketPtr = boost::shared_ptr<TcpMessageSocket<net::NetworkAsio>>;

  template<typename N>
  TcpMessageSocket<N>::TcpMessageSocket(net::IoService<N>& io, net::SslEnabled ssl,
        SocketPtr socket)
    : MessageSocket()
    , _ssl(ssl)
    , _sslContext{Method::sslv23}
    , _ioService(io)
    , _state{DisconnectedState{}}
  {
    if (socket)
    {
      net::setSocketOptions<N>(*socket, getTcpPingTimeout(Seconds{net::defaultTimeoutInSeconds}));
      _state = ConnectingState{io, ssl, socket, Handshake::server};
    }
  }

  template<typename N>
  TcpMessageSocket<N>::~TcpMessageSocket()
  {
    // We are in the destructor, so no concurrency problem.
    if (getStatus() == Status::Connected)
    {
      doDisconnect().async().wait();
      QI_LOG_INFO_SOCKET(this) << "deleted";
    }
  }

  size_t getMaxPayloadFromEnv(size_t defaultValue = 50000000);

  /// Start receiving messages. Also allows to send messages.
  ///
  /// The returned value indicates if the operation succeeded.
  /// To succeed, the object must be in the `connecting` state.
  /// This is typically obtained by first constructing the object with a
  /// connected socket,
  template<typename N>
  bool TcpMessageSocket<N>::ensureReading()
  {
    static const auto maxPayload = getMaxPayloadFromEnv();
    {
      boost::recursive_mutex::scoped_lock lock(_stateMutex);
      if (getStatus() != Status::Connecting)
      {
        QI_LOG_INFO_SOCKET(this) << "ensureReading: socket must be in connecting state.";
        return false;
      }
      auto res = asConnecting(_state).complete().value();
      if (hasError(res))
      {
        enterDisconnectedState(res.socket, res.disconnectedPromise);
        res.disconnectedPromise.future().wait();
        return false;
      }
      auto self = shared_from_this();
      _state = ConnectedState(res.socket, _ssl, maxPayload, HandleMessage<N>{self});
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
  template<typename N>
  FutureSync<void> TcpMessageSocket<N>::connect(const Url& url)
  {
    static const bool disableIpV6 = os::getenv("QIMESSAGING_ENABLE_IPV6").empty();
    Promise<void> connectedPromise;
    boost::recursive_mutex::scoped_lock lock(_stateMutex);

    if (getStatus() != Status::Disconnected)
    {
      QI_LOG_WARNING_SOCKET(this) << "connect() but status is " << static_cast<int>(getStatus());
      return ConnectingState::connectError("Must be disconnected to connect().");
    }
    // This changes the status so that concurrent calls will return in error.
    using Side = net::HandshakeSide<net::SslSocket<N>>;
    _state = ConnectingState{_ioService, url, _ssl, _sslContext, !disableIpV6, Side::client,
      getTcpPingTimeout(Seconds{net::defaultTimeoutInSeconds})};
    _url = url;
    auto self = shared_from_this();

    asConnecting(_state).complete().then([=](Future<net::ConnectingResult<N>> fut) mutable {
      // Here, connecting is over, successfully or not.
      {
        boost::recursive_mutex::scoped_lock lock(_stateMutex);
        QI_ASSERT_TRUE(getStatus() == Status::Connecting);
        const auto res = fut.value();
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
        _state = ConnectedState(res.socket, _ssl, maxPayload, HandleMessage<N>{self});
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
  template<typename N>
  FutureSync<void> TcpMessageSocket<N>::doDisconnect()
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
      asConnected(_state).requestStop(promiseDisconnected);
      break;
    case Status::Disconnecting:
      return asDisconnecting(_state).complete();
    }
    return promiseDisconnected.future();
  }

  template<typename N>
  void TcpMessageSocket<N>::enterDisconnectedState(
    const SocketPtr& socket, Promise<void> promiseDisconnected)
  {
    bool wasConnected = false;
    {
      boost::recursive_mutex::scoped_lock lock(_stateMutex);
      wasConnected = (getStatus() == Status::Connected);
      _state = DisconnectingState{socket, promiseDisconnected};
      auto self = shared_from_this();
      asDisconnecting(_state).complete().then([=](Future<void> fut) mutable {
        if (fut.hasError())
        {
          QI_LOG_WARNING_SOCKET(self.get()) << "Error while disconnecting: " << fut.error();
        }
        {
          boost::recursive_mutex::scoped_lock lock(self->_stateMutex);
          self->_state = DisconnectedState{};
          QI_LOG_DEBUG_SOCKET(self.get()) << "Socket disconnected.";
        }
        static const std::string data{"disconnected"};
        if (wasConnected)
        {
          QI_LOG_DEBUG_SOCKET(self.get()) << "Emitting `disconnected` signal.";
          self->disconnected(data);
        }
        QI_LOG_DEBUG_SOCKET(self.get()) << "Emitting `socketEvent` signal.";
        self->socketEvent(SocketEventData(data));
        QI_LOG_DEBUG_SOCKET(self.get()) << "Setting disconnected promise.";
        promiseDisconnected.setValue(nullptr);
      });
    }
  }

  template<typename N>
  bool TcpMessageSocket<N>::mustTreatAsServerAuthentication(const Message& msg) const
  {
    return !hasReceivedRemoteCapabilities()
        && msg.service() == Message::Service_Server
        && msg.function() == Message::ServerFunction_Authenticate;
  }

  template<typename N>
  bool TcpMessageSocket<N>::handleCapabilityMessage(const Message& msg)
  {
    AnyReference cmRef;
    try
    {
      cmRef = msg.value(typeOf<CapabilityMap>()->signature(), shared_from_this());
      CapabilityMap cm = cmRef.to<CapabilityMap>();
      cmRef.destroy();
      boost::mutex::scoped_lock lock(_contextMutex);
      _remoteCapabilityMap.insert(cm.begin(), cm.end());
    }
    catch (const std::runtime_error& e)
    {
      cmRef.destroy();
      QI_LOG_ERROR_SOCKET(this) << "Ill-formed capabilities message: " << e.what();
      return false;
    }
    return true;
  }

  template<typename N>
  bool TcpMessageSocket<N>::handleNormalMessage(const Message& msg)
  {
    messageReady(msg);
    socketEvent(SocketEventData(msg));
    _dispatcher.dispatch(msg);
    return true;
  }

  template<typename N>
  bool TcpMessageSocket<N>::handleMessage(const Message& msg)
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
        success = handleNormalMessage(msg);
      }
    }
    else
    {
      success = handleNormalMessage(msg);
    }
    return success;
  }

  template<typename N>
  bool TcpMessageSocket<N>::send(const Message& msg)
  {
    boost::recursive_mutex::scoped_lock lock(_stateMutex);
    if (getStatus() != Status::Connected)
    {
      QI_LOG_WARNING_SOCKET(this) << "Socket must be connected to send().";
      return false;
    }
    asConnected(_state).send(msg, _ssl);
    return true;
  }

#undef QI_LOG_ERROR_SOCKET
#undef QI_LOG_WARNING_SOCKET
#undef QI_LOG_INFO_SOCKET
#undef QI_LOG_DEBUG_SOCKET

} // namespace qi

#endif  // _SRC_TCPMESSAGESOCKET_HPP_
