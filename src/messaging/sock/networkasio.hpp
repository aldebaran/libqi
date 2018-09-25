#pragma once
#ifndef _QI_SOCK_NETWORKASIO_HPP
#define _QI_SOCK_NETWORKASIO_HPP
#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/predef.h>
#include <qi/eventloop.hpp>
#include "concept.hpp"

/// @file
/// Contains the implementation of the Network concept for boost::asio.
///
/// See traits.hpp

namespace qi { namespace sock {

  /// Model the `Network` concept for boost::asio.
  struct NetworkAsio
  {
    using acceptor_type = boost::asio::ip::tcp::acceptor;
    using resolver_type = boost::asio::ip::tcp::resolver;
    using ssl_context_type = boost::asio::ssl::context;
    using ssl_socket_type = boost::asio::ssl::stream<boost::asio::ip::tcp::socket>;
    using socket_option_no_delay_type = boost::asio::ip::tcp::no_delay;
    using accept_option_reuse_address_type = boost::asio::ip::tcp::acceptor::reuse_address;
    using error_code_type = boost::system::error_code;
    using io_service_type = boost::asio::io_service;
    using const_buffer_type = boost::asio::const_buffer;
    static io_service_type& defaultIoService()
    {
      return *static_cast<io_service_type*>(getNetworkEventLoop()->nativeHandle());
    }
    static boost::asio::ssl::verify_mode sslVerifyNone()
    {
      return boost::asio::ssl::verify_none;
    }
    template<typename T>
    static auto buffer(T* data, std::size_t maxBytes) -> decltype(boost::asio::buffer(data, maxBytes))
    {
      return boost::asio::buffer(data, maxBytes);
    }
    /// Warning: On some platform (e.g. MacOs), timeout might be ignored.
    static void setSocketNativeOptions(boost::asio::ip::tcp::socket::native_handle_type h, int timeoutInSeconds);

    /// NetSslSocket S, MutableBufferSequence B, ReadHandler H
    template<typename S, typename B, typename H>
    static void async_read(S& s, const B& b, H h)
    {
      boost::asio::async_read(s, b, h);
    }
    /// NetSslSocket S, ConstBufferSequence B, WriteHandler H
    template<typename S, typename B, typename H>
    static void async_write(S& s, const B& b, H h)
    {
      boost::asio::async_write(s, b, h);
    }
  };
}} // namespace qi::sock
#endif // _QI_SOCK_NETWORKASIO_HPP
