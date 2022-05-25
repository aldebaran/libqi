#pragma once
#ifndef _QI_SOCK_COMMON_HPP
#define _QI_SOCK_COMMON_HPP
#include <mutex>
#include <boost/shared_ptr.hpp>
#include <boost/optional.hpp>
#include <ka/functional.hpp>
#include <ka/typetraits.hpp>
#include <ka/macroregular.hpp>
#include <qi/trackable.hpp>
#include <qi/future.hpp>
#include <qi/url.hpp>
#include <qi/messaging/tcpscheme.hpp>
#include "concept.hpp"
#include "traits.hpp"
#include "option.hpp"
#include "socketptr.hpp"


/// @file
/// Contains procedure transformations (to transform a procedure into a "stranded"
/// equivalent for example) and lockable adapters around a socket.

namespace qi { namespace sock {

  /// The URL of the endpoint
  /// NetEndpoint E
  template<typename E>
  Url url(const E& ep, TcpScheme scheme)
  {
    return Url{
      ep.address().to_string(),
      to_string(scheme),
      ep.port()};
  }

  /// Returns the verify mode for SSL according to the type of peer and the target TCP scheme.
  /// Verifications are disabled in raw TCP and TCP with TLS, but enabled on both sides
  /// when targeting TCP with mutual authentication TLS.
  ///
  /// Network N,
  /// With NetSslSocket S and Mutable<S> M:
  ///   S is compatible with N,
  ///   HandshakeSide<S> H,
  template<typename N, typename H>
  SslVerifyMode<N> sslVerifyMode(H side, TcpScheme tcpScheme)
  {
    switch (side)
    {
      // Even though server-side and client-side semantics are identical for now, the client-side
      // semantics may diverge in the future. The client case is unusual (in TLS, it's unusual to not
      // at least verify the server certificate) and should not be thought as identical to the server
      // case.
      // Currently, both the server and the client only verify the certificate of the peer in mutual
      // authentication TLS. The client could also check the certificate of the server in standard
      // TLS, but in order to keep a backward compatibility with our current tools, we disable it.
      case H::server:
      case H::client:
        switch (tcpScheme)
        {
          case TcpScheme::Raw:
          case TcpScheme::Tls:
            return N::sslVerifyNone();
          case TcpScheme::MutualTls:
            return N::sslVerifyPeer() | N::sslVerifyFailIfNoPeerCert();
          default:
            QI_ASSERT_UNREACHABLE();
            throw std::domain_error("unexpected TcpScheme value");
        }
        break;
      default:
        QI_ASSERT_UNREACHABLE();
        throw std::domain_error("unexpected HandshakeSide value");
    }
  }

  /// A polymorphic transformation that takes a procedure and returns a
  /// "stranded" equivalent.
  ///
  /// Network N
  template<typename N>
  struct StrandTransfo
  {
    IoService<N>* _io;
  // Regular:
    KA_GENERATE_FRIEND_REGULAR_OPS_1(StrandTransfo, _io)
  // PolymorphicTransformation:
    /// Procedure<void (Args...)> Proc
    template<typename Proc>
    auto operator()(Proc&& p) -> decltype(_io->wrap(std::forward<Proc>(p)))
    {
      return _io->wrap(std::forward<Proc>(p));
    }
  };

  /// Gracefully closes the socket.
  ///
  /// Following the Asio documentation for a portable behavior, first shutdowns
  /// the socket, then closes it.
  ///
  /// This function ignores errors, and is therefore reentrant.
  ///
  /// Network N,
  /// With NetSslSocket S:
  ///   S is compatible with N
  template<typename N, typename S>
  void close(SocketPtr<S> socket)
  {
    using namespace sock;
    if (socket)
    {
      ErrorCode<N> erc;
      socket->lowest_layer().shutdown(ShutdownMode<Lowest<SslSocket<N>>>::shutdown_both, erc);
      socket->lowest_layer().close(erc);
    }
  }

  /// Polymorphic procedure to setup the connection stop.
  ///
  /// There are three possible steps that can be stopped:
  ///
  /// - endpoint resolve
  ///
  /// - socket connection
  ///
  /// - socket handshake
  ///
  /// The two socket steps (connection and handshake) result in the same
  /// operation: socket close.
  ///
  /// See `Connecting` for a detailed explanation of the stopping process.
  ///
  /// You can provide a procedure transformation (`lifetimeTransfo`) that will
  /// wrap the handler. It can be used for example to guarantee that the handler
  /// is only called when the Resolver to cancel is still alive.
  ///
  /// A sync procedure transformation can also be provided to wrap any
  /// handler passed to the network. A typical use is to strand the handler.
  ///
  /// Network N,
  /// With NetSslSocket S:
  ///   S is compatible with N
  /// Transformation<Procedure> F0
  /// Transformation<Procedure> F1
  template<typename N, typename S, typename F0 = ka::id_transfo_t, typename F1 = ka::id_transfo_t>
  class SetupConnectionStop
  {
    Future<void> futStop;
    F0 lifetimeTransfo;
    F1 syncTransfo;
    bool connectAlreadySetup = false;
  public:
    explicit SetupConnectionStop(const Future<void>& f,
                                 F0 lifetimeTransfo = {},
                                 F1 syncTransfo = {})
      : futStop(f)
      , lifetimeTransfo(std::move(lifetimeTransfo))
      , syncTransfo(std::move(syncTransfo))
    {
      QI_ASSERT(futStop.isValid());
    }
  // Procedure<void (Resolver<N>&)>:
    /// Overload used to stop resolving.
    void operator()(Resolver<N>& r)
    {
      futStop.andThen(syncTransfo(lifetimeTransfo(([&](void*) mutable {
        r.cancel();
      }))));
    }
  // Procedure<void (SocketPtr<S>)>:
    /// Overload used to stop connecting and handshaking.
    void operator()(const SocketPtr<S>& s)
    {
      // Can be called in the connection step and in the handshake step.
      // The stop action being the same, we setup it only once.
      if (connectAlreadySetup) return;
      futStop.andThen(syncTransfo(lifetimeTransfo(([=](void*) mutable {
        close<N>(s);
      }))));
      connectAlreadySetup = true;
    }
  };

  /// Helper function to perform type deduction for constructing a SetupConnectionStop.
  template <typename N, typename S, typename F0 = ka::id_transfo_t, typename F1 = ka::id_transfo_t>
  SetupConnectionStop<N, S, F0, F1> makeSetupConnectionStop(const Future<void>& f,
                                                            F0 lifetimeTransfo = {},
                                                            F1 syncTransfo = {})
  {
    return SetupConnectionStop<N, S, F0, F1>{ f, std::move(lifetimeTransfo),
                                              std::move(syncTransfo) };
  }
}} // namespace qi::sock

#endif // _QI_SOCK_COMMON_HPP
