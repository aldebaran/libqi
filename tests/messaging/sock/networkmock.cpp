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
  _LowestLayer::_anyShutdowner _LowestLayer::shutdown = defaultShutdown;
  N::ssl_socket_type::_anyAsyncHandshaker N::ssl_socket_type::async_handshake = defaultAsyncHandshake;
  N::acceptor_type::_anyAsyncAccepter N::acceptor_type::async_accept = defaultAsyncAccept;
  N::_anyAsyncReaderSocket N::_async_read_socket = defaultAsyncReadSocket;
  N::_anyAsyncReaderNextLayer N::_async_read_next_layer = defaultAsyncReadNextLayer;
  N::_anyAsyncWriterSocket N::_async_write_socket = defaultAsyncWriteSocket;
  N::_anyAsyncWriterNextLayer N::_async_write_next_layer = defaultAsyncWriteNextLayer;
} // namespace mock
