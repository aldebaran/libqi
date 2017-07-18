#pragma once
#ifndef _QI_SOCK_COMMON_HPP
#define _QI_SOCK_COMMON_HPP
#include <mutex>
#include <boost/shared_ptr.hpp>
#include <boost/optional.hpp>
#include <qi/functional.hpp>
#include <qi/trackable.hpp>
#include <qi/type/traits.hpp>
#include <qi/messaging/sock/concept.hpp>
#include <qi/messaging/sock/traits.hpp>
#include <qi/messaging/sock/option.hpp>
#include <qi/future.hpp>
#include <qi/macroregular.hpp>

/// @file
/// Contains procedure transformations (to transform a procedure into a "stranded"
/// equivalent for example) and lockable adapters around a socket.

namespace qi { namespace sock {

  /// A polymorphic transformation that takes a procedure and returns a
  /// "stranded" equivalent.
  ///
  /// Network N
  template<typename N>
  struct StrandTransfo
  {
    IoService<N>* _io;
  // Regular:
    QI_GENERATE_FRIEND_REGULAR_OPS_1(StrandTransfo, _io)
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
  /// Network N
  template<typename N>
  void close(SocketPtr<N> socket)
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
  /// Network N
  template<typename N>
  class SetupConnectionStop
  {
    Future<void> futStop;
    bool connectAlreadySetup = false;
  public:
    SetupConnectionStop(const Future<void>& f) : futStop(f)
    {
      QI_ASSERT(futStop.isValid());
    }
  // Procedure<void (Resolver<N>&)>:
    /// Overload used to stop resolving.
    /// Caller is reponsible to ensure that the resolver is still alive when the
    /// future is set.
    void operator()(Resolver<N>& r)
    {
      futStop.andThen([&](void*) mutable {
        r.cancel();
      });
    }
  // Procedure<void (SocketPtr<N>)>:
    /// Overload used to stop connecting and handshaking.
    void operator()(const SocketPtr<N>& s)
    {
      // Can be called in the connection step and in the handshake step.
      // The stop action being the same, we setup it only once.
      if (connectAlreadySetup) return;
      futStop.andThen([=](void*) mutable {
        close<N>(s);
      });
      connectAlreadySetup = true;
    }
  };

}} // namespace qi::sock

#endif // _QI_SOCK_COMMON_HPP
