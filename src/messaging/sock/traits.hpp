#pragma once
#ifndef _QI_SOCK_TRAITS_HPP
#define _QI_SOCK_TRAITS_HPP
#include <ka/typetraits.hpp>
#include "concept.hpp"

/// @file
/// Contains traits used by the code in the sock namespace.

namespace qi { namespace sock {
  // Network-related traits
  template<typename N>
  using Acceptor = typename N::acceptor_type;

  template<typename N>
  using Resolver = typename N::resolver_type;

  template<typename N>
  using SslContext = typename N::ssl_context_type;

  template<typename N>
  using SslSocket = typename N::ssl_socket_type;

  template<typename N>
  using SslVerifyMode = typename N::ssl_verify_mode_type;

  template<typename N>
  using SocketOptionNoDelay = typename N::socket_option_no_delay_type;

  template<typename N>
  using AcceptOptionReuseAddress = typename N::accept_option_reuse_address_type;

  template<typename N>
  using ErrorCode = typename N::error_code_type;

  template<typename N>
  using IoService = typename N::io_service_type;

  // NetResolver-related traits
  template<typename R>
  using Query = typename R::query;

  template<typename R>
  using Iterator = typename R::iterator;

  template<typename R>
  using Entry = typename R::iterator::value_type;

  // NetQuery-related traits
  template<typename Q>
  using Flag = typename Q::flags;

  // NetSslContext-related traits
  template<typename C>
  using Method = typename C::method;

  template<typename C>
  using Options = typename C::options;

  // NetSslSocket-related traits
  template<typename S>
  using HandshakeSide = typename S::handshake_type;

  template<typename S>
  using Lowest = typename S::lowest_layer_type;

  template<typename S>
  using Executor = typename S::executor_type;

  // NetLowestSocket-related traits
  template<typename L>
  using ShutdownMode = typename L::shutdown_type;

  template<typename L>
  using Endpoint = typename L::endpoint_type;

  // Misc traits
  template<typename N>
  using ConstBuffer = typename N::const_buffer_type;
}} // namespace qi::sock

#endif // _QI_SOCK_TRAITS_HPP
