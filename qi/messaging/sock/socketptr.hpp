#pragma once
#ifndef _QI_SOCK_SOCKETPTR_HPP
#define _QI_SOCK_SOCKETPTR_HPP
#include <utility>
#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>
#include <qi/messaging/sock/socketwithcontext.hpp>
#include <qi/utility.hpp>

/// @file
/// Contains socket and socket pointer traits used in the sock namespace.

namespace qi { namespace sock {

  /// Default underlying socket type.
  template<typename N>
  using Socket = SocketWithContext<N>;

  /// Default underlying socket type pointer.
  template<typename N>
  using SocketPtr = boost::shared_ptr<Socket<N>>;

  /// Helper function to deduce types for `SocketPtr`.
  ///
  /// Network N
  template<typename N, typename... Args>
  SocketPtr<N> makeSocketPtr(Args&&... args)
  {
    return boost::make_shared<Socket<N>>(fwd<Args>(args)...);
  }
}} // namespace qi::sock

#endif // _QI_SOCK_SOCKETPTR_HPP
