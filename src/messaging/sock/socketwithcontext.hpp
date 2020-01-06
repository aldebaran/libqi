#pragma once
#ifndef _QI_SOCK_SOCKETWITHCONTEXT_HPP
#define _QI_SOCK_SOCKETWITHCONTEXT_HPP
#include <ka/src.hpp>
#include <ka/mutablestore.hpp>
#include "traits.hpp"
#include "sslcontextptr.hpp"

namespace qi { namespace sock {

  /// Socket bound to an ssl context.
  ///
  /// The purpose is to ensure that the ssl context has the same lifetime as
  /// the socket.
  ///
  /// Network N
  template<typename N>
  class SocketWithContext
  {
    using socket_t = SslSocket<N>;
    using io_service_t = IoService<N>;

    SslContextPtr<N> context;
    socket_t socket;

  public:
    using next_layer_type = typename socket_t::next_layer_type;
  // NetSslSocket:
    using handshake_type = HandshakeSide<socket_t>;
    using lowest_layer_type = Lowest<socket_t>;
    using executor_type = typename socket_t::executor_type;

    template<typename Arg>
    SocketWithContext(Arg&& arg, SslContextPtr<N> ctx)
      : context(std::move(ctx))
      , socket(std::forward<Arg>(arg), *context)
    {
    }

    executor_type get_executor()
    {
      return socket.get_executor();
    }

    template<typename H>
    void async_handshake(handshake_type x, H h)
    {
      socket.async_handshake(x, h);
    }

    lowest_layer_type& lowest_layer()
    {
      return socket.lowest_layer();
    }

    next_layer_type& next_layer()
    {
      return socket.next_layer();
    }

  // Custom:
    template<typename T, typename U>
    void async_read_some(const T& buffers, const U& handler)
    {
      socket.async_read_some(buffers, handler);
    }

    template<typename T, typename U>
    void async_write_some(const T& buffers, const U& handler)
    {
      socket.async_write_some(buffers, handler);
    }
  };
}} // namespace qi::sock

#endif // _QI_SOCK_SOCKETWITHCONTEXT_HPP
