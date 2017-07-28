#pragma once
#ifndef _QI_SOCK_SOCKETWITHCONTEXT_HPP
#define _QI_SOCK_SOCKETWITHCONTEXT_HPP
#include <qi/messaging/sock/traits.hpp>
#include <qi/mutablestore.hpp>

namespace qi { namespace sock {

  /// Socket bound to an ssl context.
  ///
  /// The purpose is to ensure that the ssl context has the same lifetime as
  /// the socket.
  ///
  /// Warning: If the ssl context is passed by lvalue-reference, it is stored by
  ///   _address_. If the ssl context is passed by rvalue-reference, it is _moved_
  ///   inside the instance.
  ///
  /// Network N
  template<typename N>
  class SocketWithContext
  {
    using context_t = SslContext<N>;
    using socket_t = SslSocket<N>;
    using io_service_t = IoService<N>;

    // Do not change the declaration order.
    MutableStore<context_t, context_t*> context;
    socket_t socket;
  public:
  // NetSslSocket<N>:
    using handshake_type = HandshakeSide<socket_t>;
    using lowest_layer_type = Lowest<socket_t>;
    using next_layer_type = typename socket_t::next_layer_type;

    template<typename SslContext>
    SocketWithContext(io_service_t& io, SslContext&& ctx)
      : context(makeMutableStore(fwd<SslContext>(ctx)))
      , socket(io, *context)
    {
    }

    io_service_t& get_io_service()
    {
      return socket.get_io_service();
    }

    void set_verify_mode(decltype(N::sslVerifyNone()) x)
    {
      socket.set_verify_mode(x);
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
