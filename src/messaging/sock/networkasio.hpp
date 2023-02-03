#pragma once
#ifndef _QI_SOCK_NETWORKASIO_HPP
#define _QI_SOCK_NETWORKASIO_HPP
#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <openssl/ssl.h>
#include <boost/predef.h>
#include <qi/eventloop.hpp>
#include <qi/messaging/ssl/ssl.hpp>
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
    using ssl_verify_mode_type = boost::asio::ssl::verify_mode;
    static io_service_type& defaultIoService()
    {
      return *static_cast<io_service_type*>(getNetworkEventLoop()->nativeHandle());
    }
    template<typename S>
    static io_service_type& getIoService(S& socket)
    {
      auto exec = socket.get_executor();
      return static_cast<io_service_type&>(exec.context());
    }
    static ssl_verify_mode_type sslVerifyNone()
    {
      return boost::asio::ssl::verify_none;
    }
    static ssl_verify_mode_type sslVerifyPeer()
    {
      return boost::asio::ssl::verify_peer;
    }
    static ssl_verify_mode_type sslVerifyFailIfNoPeerCert()
    {
      return boost::asio::ssl::verify_fail_if_no_peer_cert;
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

// Ciphers are hardcoded for security reasons.
//
// Cipher list grammar used below:
// ```
//   cipherList         := cipher ":" cipherList | ""
//   cipher             := keyExchangeCrypt "-" symmetricCryptMode "-" hash
//   keyExchangeCrypt   := keyExchange "-" asymmetricCrypt
//   symmetricCryptMode := symmetricCrypt | symmetricCrypt "-" modeOperation
//   keyExchange        := "DHE" | "ECDHE"
//   asymmetricCrypt    := "RSA" | "ECDSA"
//   symmetricCrypt     := "AES128" | "AES256" | "CHACHA20"
//   modeOperation      := "GCM"
//   hash               := "SHA256" | "SHA384" | "POLY1305"
// ```
//
// acronym   plain text
// -------   ------------------------------------------
// DHE       Diffie-Hellman Ephemeral
// ECDHE     Elliptic Curve Diffie-Hellman Ephemeral
// RSA       Rivest-Shamir-Adleman
// DSA       Digital Signature Algorithm
// AES       Advanced Encryption Standard
// GCM       Galois/Counter Mode
// SHA       Secure Hash Algorithms
#define QI_SERVER_CIPHERLIST       \
   "ECDHE-ECDSA-AES128-GCM-SHA256" \
  ":ECDHE-RSA-AES128-GCM-SHA256"   \
  ":ECDHE-ECDSA-AES256-GCM-SHA384" \
  ":ECDHE-RSA-AES256-GCM-SHA384"   \
  ":ECDHE-ECDSA-CHACHA20-POLY1305" \
  ":ECDHE-RSA-CHACHA20-POLY1305"   \
  ":DHE-RSA-AES128-GCM-SHA256"     \
  ":DHE-RSA-AES256-GCM-SHA384"

    static constexpr char const* clientCipherList()
    {
      // These extra ciphers are for compatibility with old clients.
      //
      // TLSv1.2 cipher suite name       | OpenSSL equivalent name
      // ------------------------------- | -----------------------
      // TLS_RSA_WITH_AES_128_CBC_SHA256 | AES128-SHA256
      // TLS_RSA_WITH_AES_256_CBC_SHA256 | AES256-SHA256
      // TLS_RSA_WITH_AES_128_GCM_SHA256 | AES128-GCM-SHA256
      // TLS_RSA_WITH_AES_256_GCM_SHA384 | AES256-GCM-SHA384
      //
      return QI_SERVER_CIPHERLIST
        ":AES128-SHA256"
        ":AES256-SHA256"
        ":AES128-GCM-SHA256"
        ":AES256-GCM-SHA384";
    }

    static constexpr char const* serverCipherList()
    {
      return QI_SERVER_CIPHERLIST;
    }

#undef QI_SERVER_CIPHERLIST

    /// True iff set successfully.
    ///
    /// Precondition: `context`'s method is less than or equal to `TLS 1.2`.
    static inline bool trySetCipherListTls12AndBelow(ssl_context_type& context,
    const char* cipherList)
    {
      // Warning: `SSL_CTX_set_cipher_list` is only valid for `TLS 1.2` and below.
      //          Use `SSL_CTX_set_ciphersuites()` for `TLS 1.3`.
      return
        SSL_CTX_set_cipher_list(context.native_handle(), cipherList) == 1;
    }

    static void applyConfig(ssl_context_type& context,
                            const ssl::ServerConfig& cfg,
                            boost::asio::ip::tcp::endpoint clientEndpoint);

    static void applyConfig(ssl_context_type& context,
                            const ssl::ClientConfig& cfg,
                            std::string expectedServerName);
  };
}} // namespace qi::sock
#endif // _QI_SOCK_NETWORKASIO_HPP
