#pragma once
#ifndef _QI_SOCK_ACCEPT_HPP
#define _QI_SOCK_ACCEPT_HPP
#include <boost/make_shared.hpp>
#include <boost/optional.hpp>
#include <ka/src.hpp>
#include <ka/macroregular.hpp>
#include <ka/functional.hpp>
#include <qi/url.hpp>
#include "concept.hpp"
#include "traits.hpp"
#include "error.hpp"
#include "option.hpp"
#include "resolve.hpp"
#include "common.hpp"

/// @file
/// Contains functions and types related to socket acception, in the context of
/// a server accepting incoming connections.

namespace qi { namespace sock {

  /// Asynchronously accept connections and pass the associated socket via a
  /// handler.
  ///
  /// The socket is valid only if there is no error.
  /// If the handler return `true`, another connection is asynchronously waited for.
  ///
  /// Example: accepting connections until an error occurs
  /// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  /// acceptConnection(acceptor, [&] {
  ///     return makeSocketWithContextPtr<N>(
  ///       io_service, makeSslContextPtr<N>(SslContext<N>::tlsv12));
  ///   },
  ///   [](ErrorCode<N> erc, SocketWithContextPtr<N> socket) {
  ///   if (erc)
  ///   {
  ///     // handle error...
  ///     return false; // stop accepting connections
  ///   }
  ///   sockets.push_back(socket);
  ///   return true; // continue accepting connections
  /// })
  /// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  ///
  /// Example: accepting one connection and protecting against object destruction
  /// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  /// /// Network N
  /// template<typename N>
  /// struct SocketAccepter : Trackable<SocketAccepter<N>>
  /// {
  ///   // members, contructors, destructor...
  ///   void acceptOne()
  ///   {
  ///     acceptConnection(_acceptor, [&] {
  ///           return makeSocketWithContextPtr<N>(
  ///             io_service, makeSslContextPtr<N>(SslContext<N>::tlsv12));
  ///         },
  ///         [=](ErrorCode<N> erc, SocketWithContextPtr<N> socket) {
  ///         if (erc)
  ///         {
  ///           // handle error...
  ///           return false; // stop accepting connections
  ///         }
  ///         _socket = socket;
  ///         return false; // stop accepting connections
  ///       },
  ///       trackSilentTransfo(this) // lifetime transformation: don't call the
  ///                                // handler if this objet was destroyed
  ///     );
  ///   }
  /// };
  /// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  ///
  /// Network N,
  /// With NetSslSocket S and Mutable<NetSslSocket> M:
  ///   S is compatible with N,
  ///   Procedure<M ()> Proc0,
  ///   Procedure<bool (ErrorCode<N>, M)> Proc1,
  ///   Transformation<Procedure> F0,
  ///   Transformation<Procedure<void (Args...)>> F1
  template<typename N, typename Proc0, typename Proc1,
           typename F0 = ka::id_transfo_t, typename F1 = ka::id_transfo_t>
  void acceptConnection(Acceptor<N>& acceptor, Proc0 makeSocket,
    Proc1 onAccept, F0 lifetimeTransfo = {}, F1 syncTransfo = {})
  {
    auto socket = makeSocket();
    acceptor.async_accept((*socket).next_layer(), syncTransfo(lifetimeTransfo(
      [=, &acceptor](const ErrorCode<N>& erc) mutable {
        if (onAccept(erc, socket)) // true means "must continue"
        {
          acceptConnection<N>(acceptor, makeSocket, onAccept,
                              lifetimeTransfo, syncTransfo);
        }
      })));
  }

  /// Set up the acceptor to make it listen on the given endpoint.
  ///
  /// The steps are: opening, binding, setting options and listening.
  /// If any step but listen fails, an exception is thrown.
  /// The error code resulting of the listen is passed to onListen.
  ///
  /// Network N,
  /// With NetSslSocket S:
  ///   S is compatible with N,
  /// Procedure<void (ErrorCode<N>, boost::optional<Endpoint<Lowest<SslSocket<N>>>>)> Proc
  template<typename N, typename S, typename Proc = ka::constant_function_t<void>>
  void listen(Acceptor<N>& acceptor, const Endpoint<Lowest<S>>& endpoint,
    ReuseAddressEnabled reuse, Proc onListen = {})
  {
    acceptor.open(endpoint.protocol());
    acceptor.set_option(AcceptOptionReuseAddress<N>{*reuse});
    acceptor.bind(endpoint);
    ErrorCode<N> erc;
    acceptor.listen(Lowest<SslSocket<N>>::max_connections, erc);
    ErrorCode<N> localEpErc;
    const auto localEp = acceptor.local_endpoint(localEpErc);
    onListen(erc, localEpErc ? boost::none : boost::make_optional(localEp));
  }

  /// Accept new connections until told to stop.
  ///
  /// You must provide a handler that will be called for each accept.
  /// An error code and a shared pointer to socket are passed to the handler on accept.
  /// The socket pointer is valid only if there is no error (i.e. error code is false).
  ///
  /// The accept process is automatically started in the constructors.
  ///
  /// # Kinematics
  ///
  /// Here is the cinematic without error handling:
  /// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  /// -> URL resolve (optional)
  ///         |
  ///         v                                  accept complete
  /// -> acceptor set up -> wait for connection -----------------> handler(error code, socket)
  ///                               ^                                   |
  ///                               |          must continue            |
  ///                               ------------------------------------|
  ///                                                                   | must not continue
  ///                                                                   v
  ///                                                                  end
  /// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  ///
  /// ## 0) URL resolve (optional)
  /// If you pass a URL, it is first resolved.
  /// If the resolve failed, the error is passed as-is to the handler.
  /// Otherwise, if no valid endpoint was found, a "host not found" error is
  /// passed to the handler.
  ///
  /// ## 1) Acceptor set up
  /// Then, the acceptor is set up to listen on the endpoint.
  /// The steps are: opening, binding, setting options and finally listening.
  ///
  /// ## 2) Accept
  /// Then, the acceptor waits for an incoming connection.
  /// When that happens, it calls the handler with the error code and the new socket.
  /// The socket is valid only if the error code is "success".
  /// If the handler returns true, the acceptor waits again for an incoming connection.
  ///
  /// # Examples
  ///
  /// Example: accepting one connection on an endpoint
  /// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  /// Promise<SocketWithContextPtr<N>> promiseSocket;
  ///
  /// auto& io = N::defaultIoService();
  /// AcceptConnectionContinuous<N> accept{io, [&] {
  ///     return makeSocketWithContextPtr<N>(
  ///       io, makeSslContextPtr<N>(SslContext<N>::tlsv12));
  ///   },
  ///   endpoint, ReuseAddressEnabled{false},
  ///   [&](ErrorCode<N> erc, SocketWithContextPtr<N> socket) {
  ///     if (erc)
  ///     {
  ///       promiseSocket.setError("accept error");
  ///     }
  ///     else
  ///     {
  ///       promiseSocket.setValue(socket);
  ///     }
  ///     return false; // don't accept any further connections
  ///   }
  /// };
  /// auto futureSocket = promiseSocket.future();
  /// // block if necessary to avoid `accept` to go out of scope before the end
  /// // of the async operation.
  /// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  ///
  /// Example: accepting connections on a URL until an error occurs
  /// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  /// auto& io = N::defaultIoService();
  /// AcceptConnectionContinuous<N> accept{io};
  /// accept([&] {
  ///     return makeSocketWithContextPtr<N>(
  ///       io, makeSslContextPtr<N>(SslContext<N>::tlsv12));
  ///   },
  ///   url, IpV6Enabled{false}, ReuseAddressEnabled{false},
  ///   [&](ErrorCode<N> erc, SocketWithContextPtr<N> socket) {
  ///     if (erc)
  ///     {
  ///       qiLogError() << "Accept error occurred. url = " << url;
  ///       return false;
  ///     }
  ///     std::lock_guard<std::mutex> lock{socketsMutex};
  ///     sockets.push_back(socket);
  ///     return true; // accept further connections
  ///   }
  /// );
  /// // use `sockets` elsewhere.
  /// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  ///
  /// If the instance is destroyed while an async accept is pending, the handler
  /// is called with an error.
  ///
  /// Network N,
  /// With NetSslSocket S:
  ///   S is compatible with N
  template<typename N, typename S>
  class AcceptConnectionContinuous
  {
    using OptionalEntry = boost::optional<Entry<Resolver<N>>>;
    Acceptor<N> _acceptor;
    ResolveUrl<N> _resolve;

    template<typename Proc0, typename Proc1, typename Proc2, typename F0, typename F1>
    void startAccept(Proc0 makeSocket,
        const Endpoint<Lowest<S>>& endpoint, ReuseAddressEnabled reuse,
        Proc1 onAccept, Proc2 onListen, const F0& lifetimeTransfo, const F1& syncTransfo)
    {
      listen<N, S>(_acceptor, endpoint, reuse, onListen);
      acceptConnection<N>(_acceptor, makeSocket, onAccept, lifetimeTransfo, syncTransfo);
    }
  public:
  // QuasiRegular:
    KA_GENERATE_FRIEND_REGULAR_OPS_2(AcceptConnectionContinuous, _resolve, _acceptor)
    AcceptConnectionContinuous(IoService<N>& io)
      : _acceptor{io}
      , _resolve{io}
    {
    }

    ~AcceptConnectionContinuous()
    {
      ErrorCode<N> erc;
      _acceptor.close(erc);
      if (erc)
      {
        qiLogVerbose(logCategory()) << "Error while closing acceptor " << this
          << ": " << erc.message();
      }
    }

  // Procedure:
    /// With Mutable<S> M:
    ///   Procedure<M ()> Proc0,
    ///   Procedure<bool (ErrorCode<N>, M)> Proc1,
    ///   Procedure<void (ErrorCode<N>, boost::optional<Endpoint<Lowest<S>>>)> Proc2,
    ///   Transformation<Procedure> F0,
    ///   Transformation<Procedure<void (Args...)>> F1
    template<typename Proc0, typename Proc1, typename Proc2 = ka::constant_function_t<void>,
             typename F0 = ka::id_transfo_t, typename F1 = ka::id_transfo_t>
    void operator()(Proc0 makeSocket,
        const Endpoint<Lowest<S>>& endpoint, ReuseAddressEnabled reuse,
        const Proc1& onAccept, const Proc2& onListen = {}, const F0& lifetimeTransfo = {},
        const F1& syncTransfo = {})
    {
      startAccept(makeSocket, endpoint, reuse, onAccept, onListen, lifetimeTransfo, syncTransfo);
    }

    /// With Mutable<S> M:
    ///   Procedure<M ()> Proc0,
    ///   Procedure<bool (ErrorCode<N>, M)> Proc1,
    ///   Procedure<void (ErrorCode<N>, boost::optional<Endpoint<Lowest<S>>>)> Proc2,
    ///   Transformation<Procedure> F0,
    ///   Transformation<Procedure<void (Args...)>> F1
    template<typename Proc0, typename Proc1, typename Proc2 = ka::constant_function_t<void>,
             typename F0 = ka::id_transfo_t, typename F1 = ka::id_transfo_t>
    void operator()(Proc0 makeSocket, const Url& url,
        IpV6Enabled ipV6, ReuseAddressEnabled reuse,
        Proc1 onAccept, Proc2 onListen = {},
        F0 lifetimeTransfo = {}, F1 syncTransfo = {})
    {
      _resolve(url, ipV6, syncTransfo(lifetimeTransfo(
        [=](const ErrorCode<N>& erc, const OptionalEntry& entry) mutable {
          if (erc)
          {
            onAccept(erc, {});
            return;
          }
          if (!entry)
          {
            onAccept(hostNotFound<ErrorCode<N>>(), {});
            return;
          }
          startAccept(makeSocket, (*entry).endpoint(), reuse, onAccept, onListen,
            lifetimeTransfo, syncTransfo);
        }))
      );
    }
  };

  template<typename N, typename S>
  class AcceptConnectionContinuousTrack : public Trackable<AcceptConnectionContinuousTrack<N, S>>
  {
    using Trackable<AcceptConnectionContinuousTrack>::destroy;
    AcceptConnectionContinuous<N, S> _accept;
  public:
  // QuasiRegular:
    KA_GENERATE_FRIEND_REGULAR_OPS_1(AcceptConnectionContinuousTrack, _accept)
    AcceptConnectionContinuousTrack(IoService<N>& io)
      : _accept{io}
    {
    }
  // Procedure:
    /// With Mutable<S> M:
    ///   Procedure<M ()> Proc0,
    ///   Procedure<bool (ErrorCode<N>, M)> Proc1,
    ///   Procedure<void (ErrorCode<N>, boost::optional<Endpoint<Lowest<S>>>)> Proc2,
    ///   Transformation<Procedure<void (Args...)>> F
    template<typename Proc0, typename Proc1, typename Proc2 = ka::constant_function_t<void>,
             typename F = ka::id_transfo_t>
    void operator()(Proc0 makeSocket,
      const Endpoint<Lowest<S>>& endpoint, ReuseAddressEnabled reuse,
      const Proc1& onAccept, const Proc2& onListen = {}, const F& syncTransfo = {})
    {
      _accept(makeSocket, endpoint, reuse, onAccept, onListen,
              trackSilentTransfo(this), syncTransfo);
    }

    /// With Mutable<S> M:
    ///   Procedure<M ()> Proc0,
    ///   Procedure<bool (ErrorCode<N>, M)> Proc1,
    ///   Procedure<void (ErrorCode<N>, boost::optional<Endpoint<Lowest<S>>>)> Proc2,
    ///   Transformation<Procedure<void (Args...)>> F
    template<typename Proc0, typename Proc1, typename Proc2 = ka::constant_function_t<void>,
             typename F = ka::id_transfo_t>
    void operator()(Proc0 makeSocket,
      const Url& url, IpV6Enabled ipV6, ReuseAddressEnabled reuse,
      const Proc1& onAccept, const Proc2& onListen = {}, const F& syncTransfo = {})
    {
      _accept(makeSocket, url, ipV6, reuse, onAccept, onListen,
              trackSilentTransfo(this), syncTransfo);
    }

    ~AcceptConnectionContinuousTrack()
    {
      destroy();
    }
  };

}} // namespace qi::sock

#endif // _QI_SOCK_ACCEPT_HPP
