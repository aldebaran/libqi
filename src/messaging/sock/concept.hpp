#pragma once
#ifndef _QI_SOCK_CONCEPT_HPP
#define _QI_SOCK_CONCEPT_HPP
#include <ka/concept.hpp>

/// @file
/// Contains definitions of concepts used by the code in the sock namespace.

namespace qi { namespace sock {
/// # Concept definitions
///
/// ## NetErrorCode
/// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/// concept NetErrorCode(E) =
///   With E errorCode, the following is valid:
///        if (errorCode) {
///          // an error occurred
///        }
///     && static_cast<bool>(E{}) == false
///     && Integral i = errorCode.value();
///     && std::string s = errorCode.message();
///     && E e = success<E>();
///     && E e = badAddress<E>();
///     && E e = networkUnreachable<E>();
///     && E e = ownerDead<E>();
///     && E e = operationAborted<E>();
///     && E e = hostNotFound<E>();
///     && E e = fault<E>();
///     && E e = messageSize<E>();
///     && E e = connectionRefused<E>();
///     && E e = shutdown<E>();
/// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/// Any network operation can result in an error. An error is convertible to a
/// boolean: true means "error", false means "success".
/// External functions are used to give access to common errors.
///
/// ## NetTransferHandler
/// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/// concept NetTransferHandler(H, E) =
///      NetErrorCode(E)
///   && With H handler,
///           E errorCode,
///           std::size_t bytesTransferred, the following are valid:
///       handler(errorCode, bytesTransfered)
/// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/// The handler (aka callback) called when an async read or async write completes
/// (successfully or not).
/// It gives access to an error code and the number of transfered bytes.
///
/// ## NetHandler
/// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/// concept NetHandler(H, E) =
///      NetErrorCode(E)
///   && With H handler,
///           E errorCode, the following are valid:
///       handler(errorCode)
/// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/// A handler called with a single error code. Used for example when an async
/// connect completes (successfully or not).
///
/// ## NetEndpoint
/// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/// concept NetEndpoint(P) =
///   With p of type const P, the following are valid:
///        bool b = p.address().is_v6();
///     && unsigned short port = p.port();
///     && auto x = p.protocol();
///     && std::string s = p.address().to_string();
/// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/// Allows you to get the address of the endpoint and know if the endpoint is IpV6.
///
/// ## NetResolveHandler
/// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/// concept NetResolveHandler(H, E, I) =
///      NetErrorCode(E)
///   && Iterator(I)
///   && NetEndpoint(Value<I>)
///   && With H resolveHandler,
///           E errorCode,
///           I it, the following are valid:
///        resolveHandler(errorCode, it)
/// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/// The handler called when an async resolve operation completes (successfully or
/// not). The iterator gives access to a list of endpoints. The default-constructed
/// value of the iterator represents the end.
/// As long as the iterator is alive, the underlying endpoints also are, even if
/// the originating resolver is destroyed.
///
/// ## NetHandshakeSide
/// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/// concept NetHandshakeSide(H) =
///   The following are valid:
///        H h = H::client;
///     && H h = H::server;
/// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/// Handshake is done both by the client and by the server with different roles.
/// This allows you to differentiate them.
///
/// ## NetEntry
/// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/// concept NetEntry(E) =
///   With e of type const E, the following are valid:
///        NetEndpoint endpoint = e.endpoint();
/// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/// Output of a URL resolver. Give access to an endpoint.
///
/// ## NetShutdownMode
/// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/// concept NetShutdownMode(M) =
///   The following is valid:
///     M m = M::shutdown_both;
/// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/// Shutdown mode used by the lowest layer of an SSL socket.
///
/// ## NetOption
/// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/// concept NetOption(O) =
///   With bool b, the following is valid:
///     O option{b};
/// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
///
/// ## NetLowestSocket
/// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/// concept NetLowestSocket(S, E, O) =
///      NetErrorCode(E)
///   && NetOption(O)
///   && ShutdownMode<S>: NetShutdownMode
///   && Endpoint<S>: NetEndpoint
///   && With S socket,
///           const S const_socket,
///           O noDelay,
///           Endpoint<S> endpoint,
///           NetHandler handler,
///           ShutdownMode<S> shutdownMode,
///           E errorCodeLValue, the following writings are valid:
///        socket.set_option(noDelay)
///     && socket.async_connect(endpoint, handler)
///     && socket.shutdown(shutdownMode, errorCodeLValue)
///     && socket.cancel()
///     && socket.close(errorCodeLValue)
///     && Regular handle = socket.native_handle()
///     && Endpoint<S> e = const_socket.remote_endpoint()
///     && int m = S::max_connections
/// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/// Lowest-layer of an SSL socket. It allows you to connect to an endpoint and
/// close the connection.
///
/// ## NetSslContextMethod
/// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/// concept NetSslContextMethod(M) =
///   The following is valid:
///     M m = M::tlsv12;
/// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/// The method to use for the SSL context.
///
/// ## NetSslContext
/// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/// concept NetSslContext(C, V) =
//       Regular(V)
///   && Method<C>: NetSslContextMethod
///   && Options<C>: Regular
///   && With Method<C> method,
///           Options<C> opt,
///           V verifyMode,
///           std::string fileName, the following is valid:
///        C sslContext(method);
///     && Options<C> osv2  = C::no_sslv2;
///     && Options<C> osv3  = C::no_sslv3;
///     && Options<C> otv1  = C::no_tlsv1;
///     && Options<C> otv11 = C::no_tlsv1_1;
///     && sslContext.set_options(opt);
///     && sslContext.set_verify_mode(verifyMode)
/// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/// An SSL context used to construct an SSL socket.
///
/// ## NetIoService
/// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/// concept NetIoService(I) =
///   With I io, Procedure<T (ProcArgs...)> proc the following is valid:
///     Procedure<void (ProcArgs...)> proc2 = io.wrap(proc);
///     proc2(procArgs...);
///     which means that `wrap` returns a procedure accepting the same parameters
///     as `proc` but returning `void`.
/// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/// An io service wrap a procedure (typically a network handler) to strand it.
///
/// ## NetSslSocket
/// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/// concept NetSslSocket(S, O, C) =
///      NetIoService(O)
///   && NetSslContext(C)
///   && HandshakeSide<S>: NetHandshakeSide
///   && Lowest<S>: NetLowestSocket
///   && Executor<S>: Regular
///   && With O ioServiceLValue,
///           C sslContext,
///           HandshakeSide<S> handshakeSide,
///           NetHandler handler, the following are valid:
///        S sslSocket{ioServiceLValue, sslContext};
///     && sslSocket.async_handshake(handshakeSide, handler)
///     && Lowest<S>& l = sslSocket.lowest_layer();
///     && auto& n = sslSocket.next_layer();
///     && Executor<S>& e = sslSocket.get_executor();
/// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/// A socket that can send and receive data.
/// The sending and receiving is done with external functions.
/// To cipher data with SSL, a "SSL handshake" must be initially done.
/// The socket can also send and receive data without SSL, by bypassing the SSL
/// layer and directly using the "next-layer" socket.
/// To open and close the connection, the "lowest-layer" socket must be used.
///
/// ## NetAcceptor
/// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/// concept NetAcceptor(A, P, O, S, I, E) =
///      NetEndpoint(P)
///   && NetOption(O)
///   && NetSslSocket(S)
///   && NetIoService(I)
///   && NetErrorCode(E)
///   && With A acceptor,
///           const A const_acceptor,
///           const P endpoint,
///           const bool reuse,
///           S socket,
///           const NetHandler handler,
///           E& errorCode, the following are valid:
///          acceptor.open(endpoint.protocol());
///       && bool b = const_acceptor.is_open();
///       && acceptor.set_option(O{reuse});
///       && acceptor.bind(endpoint);
///       && acceptor.listen(errorCode);
///       && acceptor.async_accept(socket, handler);
///       && acceptor.close(errorCode);
///       && P localEp = const_acceptor.local_endpoint(errorCode);
/// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/// Asynchronously accepts incomming connections on a given endpoint and provides
/// the associated socket.
///
/// ## NetQueryFlag
/// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/// concept NetQueryFlag(F) =
///   The following is valid:
///     F f = F::all_matching;
/// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/// Flags used by a resolver query.
///
/// ## NetQuery
/// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/// concept NetQuery(Q) =
///      Flags<Q>: NetQueryFlag
///   && With std::string host, port,
///           Flags<Q> flags, the following are valid:
///        Q query{host, port};
///     && Q query{host, port, flags};
/// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/// A resolver query. Flags can optionally be set.
///
/// ## NetResolver
/// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/// concept NetResolver(R, O, E, S) =
///      NetIoService(O)
///   && NetErrorCode(E)
///   && NetSslSocket(S)
///   && Query<R>: NetQuery
///   && Iterator<R>: SingularIterator on Endpoint<Lowest<S>>
///   && Entry<R>: NetEntry
///   && With O ioServiceLValue,
///           Query<R> query,
///           NetResolveHandler resolveHandler, the following are valid:
///        R resolver{ioServiceLValue};
///     && resolver.async_resolve(query, resolveHandler)
///     && resolver.cancel();
///     && If `resolver` is destroyed before `resolveHandler` has been called,
///         `resolveHandler` must eventually be called with an error equal to
///         `operationAborted<E>()`
///     && The iterator passed to `resolveHandler` must remain valid even if
///         `resolver` is destroyed
/// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/// Asynchronously gets a list of endpoints for a given URL.
///
/// ## Network
/// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/// concept Network(N) =
///      Acceptor<N>: NetAcceptor
///   && Resolver<N>: NetResolver
///   && SslContext<N>: NetSslContext
///   && SslSocket<N>: NetSslSocket
///   && SslVerifyMode<N>: Regular
///   && SocketOptionNoDelay<N>: NetOption
///   && AcceptOptionReuseAddress<N>: NetOption
///   && ErrorCode<N>: NetErrorCode
///   && IoService<N>: NetIoService
///   && ConstBuffer<N>: Regular
///   && With Lowest<SslSocket<N>> lowest,
///           decltype(s.native_handle()) handle,
///           int i,
///           bool ok,
///           void* data,
///           const void* const_data,
///           const char* const_cstr,
///           std::size_t maxSizeInBytes,
///           SslSocket<N> sslSocketLValue,
///           SslContext<N> sslContextLValue,
///           NetTransferHandler transferHandler,
///           std::string hostname,
///           Endpoint<Lowest<SslSocket<N>>> clientEndpoint,
///           const ssl::ClientConfig clientSslConfig,
///           const ssl::ServerConfig serverSslConfig, the following are valid:
///        IoService<N>& io = N::defaultIoService();
///     && IoService<N>& io = N::getIoService(sslSocketLValue)
///     && SslVerifyMode<N> vn = N::sslVerifyNone();
///     && SslVerifyMode<N> vp = N::sslVerifyPeer();
///     && SslVerifyMode<N> vf = N::sslVerifyFailIfNoPeerCert();
///     && N::setSocketNativeOptionsWindows(handle, i) if compiled on Windows
///     && N::setSocketNativeOptionsLinux(handle, i) if compiled on Linux
///     && N::setSocketNativeOptionsMacOs(handle) if compiled on MacOs
///     && MutableBufferSequence mutable_bufs = N::buffer(data, maxSizeInBytes);
///         (MutableBufferSequence only constrained by the following)
///     && ConstBufferSequence const_bufs = N::buffer(const_data, maxSizeInBytes);
///         (ConstBufferSequence only constrained by the following)
///     && N::async_read(sslSocketLValue, mutable_bufs, transferHandler)
///     && N::async_read(sslSocketLValue.next_layer(), mutable_bufs, transferHandler)
///     && N::async_write(sslSocketLValue, const_bufs, transferHandler)
///     && N::async_write(sslSocketLValue.next_layer(), const_bufs, transferHandler)
///     && const_cstr = N::clientCipherList()
///     && const_cstr = N::serverCipherList()
///     && ok = N::trySetCipherListTls12AndBelow(sslContextLValue, const_cstr)
///     && N::applyConfig(sslContextLValue, clientSslConfig, hostname)
///     && N::applyConfig(sslContextLValue, serverSslConfig, clientEndpoint)
/// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/// Gives access to all types and functions handling low-level network operations:
/// - SSL socket
/// - URL resolver
/// - buffer creation
/// - ...
///
/// Allows you to change the low-level network implementation.
/// It has been designed to closely fit Boost.Asio, so as to incur no performance
/// overhead.
/// Another model is a mock for unit tests.
namespace concept // To allow doc tools to extract this documentation.
{
}
}} // namespace qi::sock
#endif // _QI_SOCK_CONCEPT_HPP
