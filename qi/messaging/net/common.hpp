#pragma once
#ifndef _QI_NET_COMMON_HPP
#define _QI_NET_COMMON_HPP
#include <mutex>
#include <boost/shared_ptr.hpp>
#include <boost/optional.hpp>
#include <qi/functional.hpp>
#include <qi/trackable.hpp>
#include <qi/type/traits.hpp>
#include <qi/messaging/net/concept.hpp>
#include <qi/messaging/net/traits.hpp>
#include <qi/macroregular.hpp>

/// @file
/// Contains procedure transformations (to transform a procedure into a "stranded"
/// equivalent for example) and lockable adapters around a socket.

namespace qi { namespace net {

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

}} // namespace qi::net

#endif // _QI_NET_COMMON_HPP
