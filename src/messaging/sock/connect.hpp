#pragma once
#ifndef _QI_SOCK_CONNECT_HPP
#define _QI_SOCK_CONNECT_HPP
#include <string>
#include <ka/src.hpp>
#include <ka/moveoncopy.hpp>
#include <ka/utility.hpp>
#include <ka/macroregular.hpp>
#include <ka/opt.hpp>
#include <ka/empty.hpp>
#include <qi/url.hpp>
#include <qi/future.hpp>
#include <qi/clock.hpp>
#include <qi/os.hpp>
#include "concept.hpp"
#include "traits.hpp"
#include "error.hpp"
#include "option.hpp"
#include "resolve.hpp"
#include "common.hpp"

/// @file
/// Contains functions and types related to socket connection.

namespace qi { namespace sock {

  /// Network N,
  /// With NetSslSocket S and Mutable<S> M:
  ///   S is compatible with N,
  ///   HandshakeSide<S> H,
  ///   Procedure<void (ErrorCode<N>, M)> Proc,
  ///   Procedure<void (M)> Proc1
  template<typename N, typename M, typename H,
           typename Proc, typename Proc1 = ka::constant_function_t<void>>
  void sslHandshake(M socket, H side, Proc onComplete, Proc1 setupStop = Proc1{})
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
  /// With NetSslSocket S and Mutable<S> M:
  ///   S is compatible with N,
  ///   HandshakeSide<S> H,
  ///   Procedure<void (ErrorCode<N>, M)> Proc,
  ///   Procedure<void (M)> Proc1
  template<typename N, typename M, typename H,
           typename Proc, typename Proc1 = ka::constant_function_t<void>>
  void connect(M socket, const Entry<Resolver<N>>& entry, Proc onComplete, SslEnabled ssl, H side,
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
        setSocketOptions<N>(socket, tcpPingTimeout);
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
  /// ConnectSocket<N, S> connect{ioService};
  /// connect(Url{"tcp://1.2.3.4:9876"}, makeSocket,
  ///   IpV6Enabled{false}, HandshakeSide<S>::Client, handler);
  /// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  ///
  /// The handler signature must be callable as:
  /// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  /// handler(ErrorCode<N>, Mutable<S>);
  /// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  ///
  /// The socket will be null if an error occurs.
  ///
  /// The object must be alive until the connecting process is complete.
  /// That is, you must _not_ write:
  /// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  /// void connect(IoService& io, Url url, M makeSocket) {
  ///   ConnectSocket connect{io};
  ///   connect(url, makeSocket, ...);
  /// }
  /// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  /// because the connecting process could exceed in time the ConnectSocket
  /// object lifetime.
  ///
  /// Example: stopping connection
  /// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  /// Promise<void> promiseStop;
  /// ConnectSocket<N, S> connect{ioService};
  /// connect(Url{"tcp://1.2.3.4:9876"}, makeSocket,
  ///   IpV6Enabled{false}, HandshakeSide<S>::Client,
  ///   handler, qi::Seconds{ 2 },
  ///   makeSetupConnectionStop<N>{promiseStop.future());
  /// // ...
  /// promiseStop.setValue(nullptr);
  /// // if not already complete, the completion handler will be called with a
  /// // "operation aborted" error
  /// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  ///
  /// From a more technical point of view, the different connecting steps
  /// are: URL resolving, socket connecting and SSL handshake if needed.
  ///
  /// Network N,
  /// With NetSslSocket S:
  ///   S is compatible with N,
  template<typename N, typename S>
  class ConnectSocket
  {
    using OptionalEntry = boost::optional<Entry<Resolver<N>>>;
    using Handshake = HandshakeSide<S>;
    ResolveUrl<N> _resolve;
  public:
  // QuasiRegular (if ResolveUrl<N> is QuasiRegular):
    KA_GENERATE_FRIEND_REGULAR_OPS_1(ConnectSocket, _resolve)
  // Custom:
    explicit ConnectSocket(IoService<N>& io)
      : _resolve{io}
    {
    }
  // Procedure:
    /// With Mutable<S> M:
    ///   Procedure<M ()> Proc0,
    ///   Procedure<void (ErrorCode<N>, M)> Proc1,
    ///   Procedure<void (Resolver<N>& || M)> Proc2
    template<typename Proc0, typename Proc1, typename Proc2 = ka::constant_function_t<void>>
    void operator()(const Url& url, Proc0 makeSocket,
        IpV6Enabled ipV6, Handshake side, Proc1 onComplete,
        const boost::optional<Seconds>& tcpPingTimeout = boost::optional<Seconds>{},
        Proc2 setupStop = Proc2{})
    {
      using namespace ka;
      SslEnabled ssl = isWithTls(tcpScheme(url).value_or(TcpScheme::Raw));
      _resolve(url, ipV6,
        [=](const ErrorCode<N>& erc, const OptionalEntry& entry) mutable { // onResolved
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
          auto createAndConnectSocket = [&]() {
            auto socket = makeSocket();
            connect<N>(socket, entry.value(), onComplete, ssl, side, tcpPingTimeout, setupStop);
          };

          // This code layer doesn't log, but communicate results through error
          // codes. These do not allow for custom error messages, so we must
          // discard any thrown exception, and replace it with a unique error
          // code. No information is lost though, if upper-layer code such as
          // `makeSocket` does log.
          auto notifyError = ka::constant_procedure([&]() {
            onComplete(socketCreationFailed<ErrorCode<N>>(), {});
          });
          ka::invoke_catch(notifyError, createAndConnectSocket);
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
    /// With Mutable<S> M:
    ///   Procedure<void (ErrorCode<N>, M> Proc1,
    ///   Procedure<void (M)> Proc2
    template<typename M, typename Proc1, typename Proc2 = ka::constant_function_t<void>>
    void operator()(SslEnabled ssl, M socket, Handshake side,
      Proc1 onComplete, Proc2 setupStop = Proc2{})
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
  /// Network N,
  /// With NetSslSocket S:
  ///   S is compatible with N
  template<typename N, typename S>
  struct ConnectHandler
  {
    Promise<SocketPtr<S>> _complete;
  // Procedure:
    void operator()(const ErrorCode<N>& erc, const SocketPtr<S>& socket)
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
  /// Network N,
  /// With NetSslSocket S:
  ///   S is compatible with N
  template<typename N, typename S>
  class ConnectSocketFuture
  {
    using Handshake = HandshakeSide<S>;
    // Member declaration order must not be changed!
    ConnectSocket<N, S> _connect;
    Promise<SocketPtr<S>> _complete;
  public:
    explicit ConnectSocketFuture(IoService<N>& io)
      : _connect{io}
    {
    }
  // Procedure:
    /// With Mutable<S> M:
    ///   Procedure<M ()> Proc0,
    ///   Procedure<void (Resolver<N>& || M)> Proc1
    template<typename Proc0, typename Proc1 = ka::constant_function_t<void>>
    void operator()(const Url& url, Proc0 makeSocket, IpV6Enabled ipV6,
      Handshake side, const boost::optional<Seconds>& tcpPingTimeout = boost::optional<Seconds>{},
      Proc1 setupStop = {})
    {
      _connect(url, makeSocket, ipV6, side, ConnectHandler<N, S>{_complete}, tcpPingTimeout, setupStop);
    }

    /// Use this overload if the socket is available _and_ already connected (this
    /// happens on server side, after an "accept" has completed).
    ///
    /// This is to keep a unique state flow:
    /// connecting -> connected -> disconnecting -> disconnected
    ///
    /// With Mutable<S> M:
    ///   Procedure<void (M)> Proc
    template<typename M, typename Proc = ka::constant_function_t<void>>
    void operator()(SslEnabled ssl, M s, Handshake side, Proc setupStop = {})
    {
      _connect(ssl, s, side, ConnectHandler<N, S>{_complete}, setupStop);
    }
  // Custom:
    Future<SocketPtr<S>> complete()
    {
      return _complete.future();
    }
  };
}} // namespace qi::sock

#endif // _QI_SOCK_CONNECT_HPP
