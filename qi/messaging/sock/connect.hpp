#pragma once
#ifndef _QI_SOCK_CONNECT_HPP
#define _QI_SOCK_CONNECT_HPP
#include <string>
#include <qi/messaging/sock/concept.hpp>
#include <qi/messaging/sock/traits.hpp>
#include <qi/messaging/sock/error.hpp>
#include <qi/messaging/sock/option.hpp>
#include <qi/messaging/sock/resolve.hpp>
#include <qi/messaging/sock/common.hpp>
#include <qi/url.hpp>
#include <qi/future.hpp>
#include <qi/clock.hpp>
#include <qi/macroregular.hpp>
#include <qi/os.hpp>

/// @file
/// Contains functions and types related to socket connection.

namespace qi { namespace sock {

  /// Network N
  template<typename N>
  SocketPtr<N> createSocket(IoService<N>& io, SslEnabled ssl, SslContext<N>& context)
  {
    auto socket = boost::make_shared<SslSocket<N>>(io, context);
    if (*ssl)
    {
      socket->set_verify_mode(N::sslVerifyNone());
    }
    return socket;
  }

  /// Network N,
  /// Readable<SslSocket<N>> S,
  /// Procedure<void (ErrorCode<N>)> Proc,
  /// Procedure<void (SocketPtr<N>)> Proc1
  template<typename N, typename S, typename Proc, typename Proc1 = PolymorphicConstantFunction<void>>
  void sslHandshake(S socket, HandshakeSide<SslSocket<N>> side,
    Proc onComplete, Proc1 setupStop = Proc1{})
  {
    (*socket).async_handshake(side,
      [=](ErrorCode<N> erc) mutable { // onSslHandshakeDone
        if (erc)
        {
          onComplete(erc, socket);
        }
        else
        {
          onComplete(success<ErrorCode<N>>(), socket);
        }
      }
    );
    setupStop(socket);
  }

  /// Connects the given socket.
  /// If present and connection is not ssl, a tcp ping timeout will be set.
  /// For ssl connections, a "no delay" option is set.
  ///
  /// Network N,
  /// Readable<SslSocket<N>> S,
  /// Procedure<void (ErrorCode<N>)> Proc,
  /// Procedure<void (SocketPtr<N>)> Proc1
  template<typename N, typename S, typename Proc, typename Proc1 = PolymorphicConstantFunction<void>>
  void connect(S socket, const Entry<Resolver<N>>& entry, Proc onComplete,
    SslEnabled ssl, HandshakeSide<SslSocket<N>> side,
    const boost::optional<Seconds>& tcpPingTimeout, Proc1 setupStop = Proc1{})
  {
    (*socket).lowest_layer().async_connect(entry,
      [=](ErrorCode<N> erc) mutable { // onConnectDone
        if (erc)
        {
          onComplete(erc, socket);
          return;
        }
        // Options can be set only once the socket is connected.
        setSocketOptions<N>(*socket, tcpPingTimeout);
        if (*ssl)
        {
          sslHandshake<N>(socket, side, onComplete, setupStop);
        }
        else
        {
          onComplete(success<ErrorCode<N>>(), socket);
        }
      }
    );
    setupStop(socket);
  }

  /// Connects to a URL and gives the created socket by calling a handler.
  ///
  /// See `ResolveUrl` for the URL format.
  ///
  /// Usage:
  /// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  /// ConnectSocket<N> connect{ioService};
  /// connect(Url{"tcp://1.2.3.4:9876"}, handler,
  ///   SslEnabled{false}, IpV6Enabled{false});
  /// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  ///
  /// The handler signature must be callable as:
  /// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  /// handler(ErrorCode<N>, SocketPtr<N>);
  /// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  ///
  /// The socket will be null if an error occurs.
  ///
  /// The object must be alive until the connecting process is complete.
  /// That is, you must _not_ write:
  /// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  /// void connectNonSsl(IoService& io, Url url, Handler handler) {
  ///   ConnectSocket connect{io};
  ///   connect(url, handler, SslEnabled{false}, IpV6Enabled{false});
  /// }
  /// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  /// because the connecting process could exceed in time the ConnectSocket
  /// object lifetime.
  ///
  /// Example: stopping connection
  /// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  /// Promise<void> promiseStop;
  /// ConnectSocket<N> connect{ioService};
  /// connect(Url{"tcp://1.2.3.4:9876"}, handler,
  ///   SslEnabled{false}, IpV6Enabled{false},
  ///   SetupConnectionStop<N>{promiseStop.future()});
  /// // ...
  /// promiseStop.setValue(nullptr);
  /// // if not already complete, the completion handler will be called with a
  /// // "operation aborted" error
  /// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  ///
  /// From a more technical point of view, the different connecting steps
  /// are: URL resolving, socket connecting and SSL handshake if needed.
  template<typename N>
  class ConnectSocket
  {
    using OptionalEntry = boost::optional<Entry<Resolver<N>>>;
    using Handshake = HandshakeSide<SslSocket<N>>;
    ResolveUrl<N> _resolve;
  public:
  // QuasiRegular (if ResolveUrl<N> is QuasiRegular):
    QI_GENERATE_FRIEND_REGULAR_OPS_1(ConnectSocket, _resolve)
  // Custom:
    explicit ConnectSocket(IoService<N>& io)
      : _resolve{io}
    {
    }
  // Procedure:
    /// Procedure<void (ErrorCode<N>, SocketPtr)> Proc,
    /// Procedure<void (Resolver<N>& || SocketPtr<N>)> Proc1
    template<typename Proc, typename Proc1 = PolymorphicConstantFunction<void>>
    void operator()(const Url& url, SslEnabled ssl, SslContext<N>& context,
        IpV6Enabled ipV6, Handshake side, Proc onComplete,
        const boost::optional<Seconds>& tcpPingTimeout = boost::optional<Seconds>{},
        Proc1 setupStop = Proc1{})
    {
      auto& io = _resolve.getIoService();
      _resolve(url, ipV6,
        [=, &io, &context](const ErrorCode<N>& erc, const OptionalEntry& entry) mutable { // onResolved
          if (erc)
          {
            onComplete(erc, {});
            return;
          }
          if (!entry)
          {
            onComplete(hostNotFound<ErrorCode<N>>(), {});
            return;
          }
          auto socket = createSocket<N>(io, ssl, context);
          connect<N>(socket, entry.value(), onComplete, ssl, side, tcpPingTimeout, setupStop);
        },
        setupStop
      );
    }

    /// The socket must already be connected.
    /// Only ssl handshake is done if necessary.
    ///
    /// You have for example an already connected socket when it comes from a
    /// server accepting an incoming connection.
    /// This overload is provided for uniformity purpose.
    ///
    /// Procedure<void (ErrorCode<N>, SocketPtr)> Proc,
    /// Procedure<void (SocketPtr<N>)> Proc1
    template<typename S, typename Proc, typename Proc1 = PolymorphicConstantFunction<void>>
    void operator()(SslEnabled ssl, const S& socket, Handshake side,
      Proc onComplete, Proc1 setupStop = Proc1{})
    {
      if (*ssl)
      {
        sslHandshake<N>(socket, side, onComplete, setupStop);
      }
      else
      {
        onComplete(success<ErrorCode<N>>(), socket);
      }
    }
  };

  /// A functor that sets a promise containing a pointer to a socket.
  /// It can be used as a connect handler.
  ///
  /// In case of error, the promise is set in error with a string of the form:
  /// "error_code: error_message"
  ///
  /// The code is specified because it is portable, but the message is not.
  ///
  /// Network N
  template<typename N>
  struct ConnectHandler
  {
    Promise<SocketPtr<N>> _complete;
  // Procedure:
    void operator()(const ErrorCode<N>& erc, const SocketPtr<N>& socket)
    {
      if (erc)
      {
        _complete.setError(os::to_string(erc.value()) + ": " + erc.message());
      }
      else
      {
        _complete.setValue(socket);
      }
    }
  };

  /// Connects to a URL and gives the created socket via a future.
  ///
  /// The only difference relatively to ConnectSocket is that the connected
  /// socket is presented via a future instead of a handler.
  ///
  /// Usage:
  /// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  /// ConnectSocketFuture<N> connect{ioService};
  /// connect(Url{"tcp://1.2.3.4:9876"}, SslEnabled{false}, IpV6Enabled{false});
  /// auto socketPtr = connect.complete().value();
  /// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  ///
  /// Warning: in the same manner as ConnectSocket, the ConnectSocketFuture
  /// object must remain alive until the connecting process is complete.
  ///
  /// Remark: See `ConnectSocket` for an explanation on the stop.
  ///
  /// Network N
  template<typename N>
  class ConnectSocketFuture
  {
    using Handshake = HandshakeSide<SslSocket<N>>;
    // Member declaration order must not be changed!
    ConnectSocket<N> _connect;
    Promise<SocketPtr<N>> _complete;
  public:
    explicit ConnectSocketFuture(IoService<N>& io)
      : _connect{io}
    {
    }
  // Procedure:
    /// Procedure<void (Resolver<N>& || SocketPtr<N>)> Proc
    template<typename Proc = PolymorphicConstantFunction<void>>
    void operator()(const Url& url, SslEnabled ssl, SslContext<N>& context, IpV6Enabled ipV6,
      Handshake side, const boost::optional<Seconds>& tcpPingTimeout = boost::optional<Seconds>{},
      Proc setupStop = {})
    {
      _connect(url, ssl, context, ipV6, side, ConnectHandler<N>{_complete}, tcpPingTimeout,
        setupStop);
    }

    /// Use this overload if the socket is available _and_ already connected (this
    /// happens on server side, after an "accept" has completed).
    ///
    /// This is to keep a unique state flow:
    /// connecting -> connected -> disconnecting -> disconnected
    ///
    /// Procedure<void (SocketPtr<N>)> Proc
    template<typename Proc = PolymorphicConstantFunction<void>>
    void operator()(SslEnabled ssl, const SocketPtr<N>& s, Handshake side, Proc setupStop = {})
    {
      _connect(ssl, s, side, ConnectHandler<N>{_complete}, setupStop);
    }
  // Custom:
    Future<SocketPtr<N>> complete()
    {
      return _complete.future();
    }
  };
}} // namespace qi::sock

#endif // _QI_SOCK_CONNECT_HPP
