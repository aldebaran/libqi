#pragma once
#ifndef _QI_SOCK_ACCEPT_HPP
#define _QI_SOCK_ACCEPT_HPP
#include <boost/make_shared.hpp>
#include <qi/messaging/sock/concept.hpp>
#include <qi/messaging/sock/traits.hpp>
#include <qi/messaging/sock/error.hpp>
#include <qi/messaging/sock/option.hpp>
#include <qi/messaging/sock/resolve.hpp>
#include <qi/messaging/sock/common.hpp>
#include <qi/url.hpp>
#include <qi/macroregular.hpp>
#include <qi/functional.hpp>

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
  /// acceptConnection(acceptor, sslContext, [](ErrorCode<N> erc, SocketPtr<N> socket) {
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
  ///     SslContext<N> context{SslContext<N>::sslv23};
  ///     acceptConnection(_acceptor, context, [=](ErrorCode<N> erc, SocketPtr<N> socket) {
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
  /// Procedure<bool (ErrorCode<N>, SocketPtr<N>)> Proc,
  /// Transformation<Procedure> F0,
  /// Transformation<Procedure<void (Args...)>> F1
  template<typename N, typename Proc, typename F0 = IdTransfo, typename F1 = IdTransfo>
  void acceptConnection(Acceptor<N>& acceptor, SslContext<N>& context,
    Proc onAccept, F0 lifetimeTransfo = {}, F1 syncTransfo = {})
  {
    SocketPtr<N> socket = boost::make_shared<SslSocket<N>>(acceptor.get_io_service(), context);
    acceptor.async_accept((*socket).next_layer(), syncTransfo(lifetimeTransfo(
      [=, &acceptor, &context](const ErrorCode<N>& erc) mutable {
        if (onAccept(erc, socket)) // true means "must continue"
        {
          acceptConnection<N>(acceptor, context, onAccept, lifetimeTransfo, syncTransfo);
        }
      })));
  }

  /// Set up the acceptor to make it listen on the given endpoint.
  ///
  /// The steps are: opening, binding, setting options and listening.
  /// If any step fails, an exception is thrown.
  ///
  /// Network N
  template<typename N>
  void listen(Acceptor<N>& acceptor, const Endpoint<Lowest<SslSocket<N>>>& endpoint,
    ReuseAddressEnabled reuse)
  {
    acceptor.open(endpoint.protocol());
    acceptor.set_option(AcceptOptionReuseAddress<N>{*reuse});
    acceptor.bind(endpoint);
    acceptor.listen();
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
  /// Promise<SocketPtr<N>> promiseSocket;
  ///
  /// AcceptConnectionContinuous<N> accept{N::defaultIoService(), sslContext,
  ///   endpoint, ReuseAddressEnabled{false},
  ///   [&](ErrorCode<N> erc, SocketPtr<N> socket) {
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
  /// AcceptConnectionContinuous<N> accept{N::defaultIoService()};
  /// accept(sslContext, url, IpV6Enabled{false}, ReuseAddressEnabled{false},
  ///   [&](ErrorCode<N> erc, SocketPtr<N> socket) {
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
  /// Network N
  template<typename N>
  class AcceptConnectionContinuous
  {
    using OptionalEntry = boost::optional<Entry<Resolver<N>>>;
    Acceptor<N> _acceptor;
    ResolveUrl<N> _resolve;

    template<typename Proc, typename F0, typename F1>
    void startAccept(SslContext<N>& context,
        const Endpoint<Lowest<SslSocket<N>>>& endpoint, ReuseAddressEnabled reuse,
        Proc onAccept, const F0& lifetimeTransfo, const F1& syncTransfo)
    {
      listen<N>(_acceptor, endpoint, reuse);
      acceptConnection<N>(_acceptor, context, onAccept, lifetimeTransfo, syncTransfo);
    }
  public:
  // QuasiRegular:
    QI_GENERATE_FRIEND_REGULAR_OPS_2(AcceptConnectionContinuous, _resolve, _acceptor)
    AcceptConnectionContinuous(IoService<N>& io)
      : _acceptor{io}
      , _resolve{io}
    {
    }
  // Procedure:
    /// Procedure<bool (ErrorCode<N>, SocketPtr<N>)> Proc,
    /// Transformation<Procedure> F0,
    /// Transformation<Procedure<void (Args...)>> F1
    template<typename Proc, typename F0 = IdTransfo, typename F1 = IdTransfo>
    void operator()(SslContext<N>& context,
        const Endpoint<Lowest<SslSocket<N>>>& endpoint, ReuseAddressEnabled reuse,
        const Proc& onAccept, const F0& lifetimeTransfo = {}, const F1& syncTransfo = {})
    {
      startAccept(context, endpoint, reuse, onAccept, lifetimeTransfo, syncTransfo);
    }

    /// Procedure<bool (ErrorCode<N>, SocketPtr<N>)> Proc,
    /// Transformation<Procedure> F0,
    /// Transformation<Procedure<void (Args...)>> F1
    template<typename Proc, typename F0 = IdTransfo, typename F1 = IdTransfo>
    void operator()(SslContext<N>& context, const Url& url,
      IpV6Enabled ipV6, ReuseAddressEnabled reuse,
      Proc onAccept, F0 lifetimeTransfo = {}, F1 syncTransfo = {})
    {
      _resolve(url, ipV6, syncTransfo(lifetimeTransfo(
        [=, &context](const ErrorCode<N>& erc, const OptionalEntry& entry) mutable {
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
          startAccept(context, (*entry).endpoint(), reuse, onAccept,
            lifetimeTransfo, syncTransfo);
        }))
      );
    }

    ~AcceptConnectionContinuous()
    {
      if (_acceptor.is_open())
      {
        ErrorCode<N> erc;
        _acceptor.close(erc);
        if (erc)
        {
          qiLogWarning(logCategory()) << "Error while closing acceptor " << this
            << ": " << erc.message();
        }
      }
    }
  };

  template<typename N>
  class AcceptConnectionContinuousTrack : public Trackable<AcceptConnectionContinuousTrack<N>>
  {
    using Trackable<AcceptConnectionContinuousTrack>::destroy;
    AcceptConnectionContinuous<N> _accept;
  public:
  // QuasiRegular:
    QI_GENERATE_FRIEND_REGULAR_OPS_1(AcceptConnectionContinuousTrack, _accept)
    AcceptConnectionContinuousTrack(IoService<N>& io)
      : _accept{io}
    {
    }
  // Procedure:
    /// Procedure<bool (ErrorCode<N>, SocketPtr<N>)> Proc,
    /// Transformation<Procedure<void (Args...)>> F
    template<typename Proc, typename F = IdTransfo>
    void operator()(SslContext<N>& context,
      const Endpoint<Lowest<SslSocket<N>>>& endpoint, ReuseAddressEnabled reuse,
      const Proc& onAccept, const F& syncTransfo = {})
    {
      _accept(context, endpoint, reuse, onAccept, trackSilentTransfo(this), syncTransfo);
    }

    /// Procedure<bool (ErrorCode<N>, SocketPtr<N>)> Proc,
    /// Transformation<Procedure> F0,
    /// Transformation<Procedure<void (Args...)>> F1
    template<typename Proc, typename F = IdTransfo>
    void operator()(SslContext<N>& context,
      const Url& url, IpV6Enabled ipV6, ReuseAddressEnabled reuse,
      const Proc& onAccept, const F& syncTransfo = {})
    {
      _accept(context, url, ipV6, reuse, onAccept, trackSilentTransfo(this), syncTransfo);
    }

    ~AcceptConnectionContinuousTrack()
    {
      destroy();
    }
  };

}} // namespace qi::sock

#endif // _QI_SOCK_ACCEPT_HPP
