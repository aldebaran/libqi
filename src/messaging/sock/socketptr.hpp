#pragma once
#ifndef _QI_SOCK_SOCKETPTR_HPP
#define _QI_SOCK_SOCKETPTR_HPP
#include <utility>
#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>
#include <ka/utility.hpp>
#include "socketwithcontext.hpp"

/// @file
/// Contains socket and socket pointer traits used in the sock namespace.

namespace qi { namespace sock {

  /// NetSslSocket Socket
  template<typename Socket>
  using SocketPtr = boost::shared_ptr<Socket>;

  template<typename N>
  using SslSocketPtr = SocketPtr<SslSocket<N>>;
  template<typename N>
  using SocketWithContextPtr = SocketPtr<SocketWithContext<N>>;

  /// Constructs a simple SslSocket. The caller is responsible of managing the
  /// lifetime of the SslContext.
  ///
  /// Network N
  template<typename N, typename Arg>
  SslSocketPtr<N> makeSslSocketPtr(Arg&& arg, SslContext<N>& context)
  {
    return boost::make_shared<SslSocket<N>>(std::forward<Arg>(arg), context);
  }

  /// Constructs a SocketWithContext that will ensure that the SslContext shares the
  /// lifetime of the socket.
  ///
  /// Network N
  template<typename N, typename... Args>
  SocketWithContextPtr<N> makeSocketWithContextPtr(Args&&... args)
  {
    return boost::make_shared<SocketWithContext<N>>(std::forward<Args>(args)...);
  }
}} // namespace qi::sock

#endif // _QI_SOCK_SOCKETPTR_HPP
