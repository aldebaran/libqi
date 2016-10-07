#pragma once
#ifndef _QI_NET_CONNECT_HPP
#define _QI_NET_CONNECT_HPP
#include <string>
#include <qi/messaging/net/concept.hpp>
#include <qi/messaging/net/traits.hpp>
#include <qi/messaging/net/error.hpp>
#include <qi/messaging/net/option.hpp>
#include <qi/messaging/net/resolve.hpp>
#include <qi/messaging/net/common.hpp>
#include <qi/url.hpp>
#include <qi/future.hpp>
#include <qi/clock.hpp>
#include <qi/macroregular.hpp>

/// @file
/// Contains functions and types related to socket connection.

namespace qi { namespace net {
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

  namespace detail
  {
    /// Network N, Readable<SslSocket<N>> S, Procedure<void (ErrorCode<N>)> Proc
    template<typename N, typename S, typename Proc>
    void onSslHandshakeDone(const ErrorCode<N>& erc, const S& socket, Proc onComplete)
    {
      if (erc)
      {
        onComplete(erc, socket);
        return;
      }
      onComplete(success<ErrorCode<N>>(), socket);
    }

    /// Network N, Readable<SslSocket<N>> S, Procedure<void (ErrorCode<N>)> Proc
    template<typename N, typename S, typename Proc>
    void sslHandshake(S socket, HandshakeSide<SslSocket<N>> side, const Proc& onComplete)
    {
      (*socket).async_handshake(side,
        [=](ErrorCode<N> erc) {
          onSslHandshakeDone<N>(erc, socket, onComplete);
        }
      );
    }

    /// Network N, Readable<SslSocket<N>> S, Procedure<void (ErrorCode<N>)> Proc
    template<typename N, typename S, typename Proc>
    void onConnectDone(const ErrorCode<N>& erc, const S& socket, Proc onComplete,
        SslEnabled ssl, HandshakeSide<SslSocket<N>> side,
        const boost::optional<Seconds>& tcpPingTimeout)
    {
      if (erc)
      {
        onComplete(erc, socket);
        return;
      }
      // Options can be set only once the socket is connected.
      setSocketOptions<N>(*socket, tcpPingTimeout);
      if (*ssl)
      {
        sslHandshake<N>(socket, side, onComplete);
      }
      else
      {
        onComplete(success<ErrorCode<N>>(), socket);
      }
    }
  } // namespace detail

  /// Connects the given socket.
  /// If present and connection is not ssl, a tcp ping timeout will be set.
  /// For ssl connections, a "no delay" option is set.
  ///
  /// Network N, Readable<SslSocket<N>> S, Procedure<void (ErrorCode<N>)> Proc
  template<typename N, typename S, typename Proc>
  void connect(S socket, const Entry<Resolver<N>>& entry, const Proc& onComplete,
    SslEnabled ssl, HandshakeSide<SslSocket<N>> side,
    const boost::optional<Seconds>& tcpPingTimeout)
  {
    (*socket).lowest_layer().async_connect(entry,
      [=](ErrorCode<N> erc) {
        detail::onConnectDone<N>(erc, socket, onComplete, ssl, side, tcpPingTimeout);
      }
    );
  }

  namespace detail
  {
    /// Network N, Procedure<void (ErrorCode<N>)> Proc
    template<typename N, typename Proc>
    void onResolved(const ErrorCode<N>& erc, const boost::optional<Entry<Resolver<N>>>& entry,
      IoService<N>& io, Proc onComplete, SslEnabled ssl, SslContext<N>& context,
      HandshakeSide<SslSocket<N>> side, const boost::optional<Seconds>& tcpPingTimeout)
    {
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
      connect<N>(createSocket<N>(io, ssl, context), entry.value(),
        onComplete, ssl, side, tcpPingTimeout);
    }
  } // namespace detail


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
  /// From a more technical point of view, the different connecting steps
  /// are: URL resolving, socket connecting and ssl handshake if needed.
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
    /// Procedure<void (ErrorCode<N>, SocketPtr)> Proc
    template<typename Proc>
    void operator()(const Url& url, SslEnabled ssl, SslContext<N>& context,
        IpV6Enabled ipV6, Handshake side, const Proc& onComplete,
        const boost::optional<Seconds>& tcpPingTimeout = {})
    {
      _resolve(url, ipV6,
        [=, &context](const ErrorCode<N>& erc, const OptionalEntry& entry) {
          detail::onResolved<N>(erc, entry, _resolve.getIoService(), onComplete,
            ssl, context, side, tcpPingTimeout);
        });
    }
    /// The socket must already be connected.
    /// Only ssl handshake is done if necessary.
    ///
    /// You have for example an already connected socket when it comes from a
    /// server accepting an incoming connection.
    /// This overload is provided for uniformity purpose.
    ///
    /// Procedure<void (ErrorCode<N>, SocketPtr)> Proc
    template<typename S, typename Proc>
    void operator()(SslEnabled ssl, const S& socket, Handshake side, Proc onComplete)
    {
      if (*ssl)
      {
        detail::sslHandshake<N>(socket, side, onComplete);
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
        _complete.setError(std::to_string(erc.value()) + ": " + erc.message());
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
    void operator()(const Url& url, SslEnabled ssl, SslContext<N>& context, IpV6Enabled ipV6,
      Handshake side, const boost::optional<Seconds>& tcpPingTimeout = {})
    {
      _connect(url, ssl, context, ipV6, side, ConnectHandler<N>{_complete}, tcpPingTimeout);
    }
    void operator()(SslEnabled ssl, const SocketPtr<N>& s, Handshake side)
    {
      _connect(ssl, s, side, ConnectHandler<N>{_complete});
    }
  // Custom:
    Future<SocketPtr<N>> complete()
    {
      return _complete.future();
    }
  };
}} // namespace qi::net

#endif // _QI_NET_CONNECT_HPP
