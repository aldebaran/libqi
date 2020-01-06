#include "networkmock.hpp"

/// @file
/// Contains definitions for the default network mock operations :
/// socket connect, socket read, etc.

namespace mock
{
  N::_resolver_entry* N::resolver_type::iterator::_sentinel = nullptr;
  N::resolver_type::_anyAsyncResolver N::resolver_type::async_resolve = defaultAsyncResolve;
  _LowestLayer::_anyAsyncConnecter _LowestLayer::async_connect = defaultAsyncConnect;
  _LowestLayer::_anyCanceler _LowestLayer::cancel = defaultCancel;
  _LowestLayer::_anyShutdowner _LowestLayer::_shutdown = defaultShutdown;
  _LowestLayer::_anyCloser _LowestLayer::close = defaultClose;
  N::ssl_context_type::_anyVerifyModeSetter N::ssl_context_type::set_verify_mode
    = defaultVerifyModeSetter;
  N::ssl_socket_type::_anyAsyncHandshaker N::ssl_socket_type::async_handshake = defaultAsyncHandshake;
  N::acceptor_type::_anyAsyncAccepter N::acceptor_type::async_accept = defaultAsyncAccept;
  std::atomic_bool N::resultOfTrySetCipherListTls12AndBelow(true);

  template <>
  N::_anyAsyncReaderSocket<N::ssl_socket_type>
      N::SocketFunctions<N::ssl_socket_type>::_async_read_socket =
          defaultAsyncReadSocket<N::ssl_socket_type>;
  template <>
  N::_anyAsyncReaderSocket<qi::sock::SocketWithContext<N>>
      N::SocketFunctions<qi::sock::SocketWithContext<N>>::_async_read_socket =
          defaultAsyncReadSocket<qi::sock::SocketWithContext<N>>;
  N::_anyAsyncReaderNextLayer N::_async_read_next_layer = defaultAsyncReadNextLayer;

  template <>
  N::_anyAsyncWriterSocket<N::ssl_socket_type>
      N::SocketFunctions<N::ssl_socket_type>::_async_write_socket =
          defaultAsyncWriteSocket<N::ssl_socket_type>;
  template <>
  N::_anyAsyncWriterSocket<qi::sock::SocketWithContext<N>>
      N::SocketFunctions<qi::sock::SocketWithContext<N>>::_async_write_socket =
          defaultAsyncWriteSocket<qi::sock::SocketWithContext<N>>;
  N::_anyAsyncWriterNextLayer N::_async_write_next_layer = defaultAsyncWriteNextLayer;

  N::_anyApplyClientConfig N::_applyClientConfig = defaultApplyClientConfig;
} // namespace mock
