#include <thread>
#include <boost/algorithm/string/predicate.hpp>
#include <gtest/gtest.h>
#include <qi/messaging/net/connect.hpp>
#include <qi/messaging/net/networkasio.hpp>
#include "src/messaging/tcpmessagesocket.hpp"
#include <qi/future.hpp>
#include <qi/url.hpp>
#include "networkmock.hpp"
#include "networkcommon.hpp"

////////////////////////////////////////////////////////////////////////////////
// NetConnectSocket tests:
////////////////////////////////////////////////////////////////////////////////

TEST(NetConnectSocket, ResolveCalledAfterParentHasBeenDestroyed)
{
  using namespace qi;
  using namespace qi::net;
  using mock::N;
  using mock::Resolver;
  std::thread resolveThread;
  Promise<void> nukeObject;
  // The resolve is going to fail. Before calling the handler, we're going to
  // wait the ConnectSocketFuture object is destroyed.
  Resolver::async_resolve = [&](Resolver::query, Resolver::_anyResolveHandler h) {
    // We launch asynchronously to return immediately.
    resolveThread = std::thread{[=]{
      // Wait for the object destruction.
      nukeObject.future().wait();
      // Now call the handler.
      h(operationAborted<ErrorCode<N>>(), Resolver::iterator{});
    }};
  };
  ConnectSocket<N>* p = nullptr;
  using SocketPtr = boost::shared_ptr<SslSocket<N>>;
  Promise<ErrorCode<N>> promiseError;
  IoService<N>& io = N::defaultIoService();
  {
    using Side = HandshakeSide<SslSocket<N>>;
    ConnectSocket<N> connect{io};
    SslContext<N> context{Method<SslContext<N>>::sslv23};
    connect(Url{"tcp://10.11.12.13:1234"}, SslEnabled{false}, context,
      IpV6Enabled{false}, Side::client,
      [=](ErrorCode<N> e, SocketPtr) mutable {
        promiseError.setValue(e);
      }
    );
    p = &connect;
  }
  // The connecting object is now destroyed and on top of that we wipe out its memory.
  overwrite(p);
  // Now we unblock the resolve handler.
  nukeObject.setValue(0);
  // We wait for an error to occur.
  ASSERT_EQ(operationAborted<ErrorCode<N>>(), promiseError.future().value());
  resolveThread.join();
}

////////////////////////////////////////////////////////////////////////////////
// NetConnectSocketFuture tests:
////////////////////////////////////////////////////////////////////////////////

template<typename T>
struct NetConnectFuture : testing::Test
{
};

namespace qi { namespace net {

/// Network N
template<typename N>
struct ConnectingWrap
{
  using SocketPtr = boost::shared_ptr<SslSocket<N>>;
  using Handshake = HandshakeSide<SslSocket<N>>;
  IoService<N>& _io;
  std::unique_ptr<Connecting<N>> _connecting;
  Promise<SocketPtr> _complete;

  /// Adapt Connecting to behave as ConnectSocket.
  /// This allows to share unit tests for these two types.
  ConnectingWrap(IoService<N>& io)
    : _io(io)
  {
  }
  void operator()(const Url& url, SslEnabled ssl, SslContext<N>& context, IpV6Enabled ipV6, Handshake side,
    const boost::optional<Seconds>& tcpPingTimeout = {})
  {
    _connecting.reset(new Connecting<N>{_io, url, ssl, context, ipV6, side, tcpPingTimeout});
    auto complete = _complete;
    _connecting->complete().then(
      [=](Future<ConnectingResult<N>> fut) mutable {
        if (hasError(fut.value()))
        {
          complete.setError(fut.value().errorMessage);
        }
        else
        {
          complete.setValue(fut.value().socket);
        }
      }
    );
  }
  ~ConnectingWrap()
  {
  }
  Future<SocketPtr> complete() const
  {
    return _complete.future();
  }
};

}} // namespace qi::net

using sequences = testing::Types<
  // Mock
  qi::net::ConnectSocketFuture<mock::N>, qi::net::ConnectingWrap<mock::N>
  // Asio
  //, qi::net::ConnectSocketFuture<qi::net::NetworkAsio>, qi::net::Connecting<qi::net::NetworkAsio>
>;

TYPED_TEST_CASE(NetConnectFuture, sequences);

TYPED_TEST(NetConnectFuture, FailsOnResolve)
{
  using ConnectFuture = TypeParam;
  using namespace qi;
  using namespace qi::net;
  using mock::N;
  using mock::Resolver;
  std::string receivedHost, receivedPort;
  Resolver::async_resolve = [&](Resolver::query q, Resolver::_anyResolveHandler h) {
    receivedHost = q._host;
    receivedPort = q._port;
    h(networkUnreachable<ErrorCode<N>>(), Resolver::iterator{});
  };
  IoService<N>& io = N::defaultIoService();
  const Url url{"tcp://10.11.12.13:1234"};
  using Side = HandshakeSide<SslSocket<N>>;
  ConnectFuture connect{io};
  SslContext<N> context{Method<SslContext<N>>::sslv23};
  connect(url, SslEnabled{false}, context, IpV6Enabled{false}, Side::client);
  ASSERT_TRUE(connect.complete().hasError());
  ASSERT_EQ(code(connect.complete().error()), networkUnreachable<ErrorCode<N>>().value());
  ASSERT_EQ("10.11.12.13", receivedHost);
  ASSERT_EQ("1234", receivedPort);
}

// Test that when the resolve callback is called and its parent has been destroyed,
// the connected future is in error.
TYPED_TEST(NetConnectFuture, ResolveCalledAfterParentHasBeenDestroyed)
{
  using ConnectFuture = TypeParam;
  using namespace qi;
  using namespace qi::net;
  using namespace boost::algorithm;
  using mock::N;
  using mock::Resolver;
  std::thread resolveThread;
  Promise<void> nukeObject;
  // The resolve is going to fail. Before calling the handler, we're going to
  // wait the ConnectFuture object is destroyed.
  Resolver::async_resolve = [&](Resolver::query, Resolver::_anyResolveHandler h) {
    // We launch asynchronously to return immediately.
    resolveThread = std::thread{[=]{
      // Wait for the object destruction.
      nukeObject.future().wait();
      // Now call the handler.
      h(operationAborted<ErrorCode<N>>(), Resolver::iterator{});
    }};
  };
  ConnectFuture* p = nullptr;
  using SocketPtr = boost::shared_ptr<SslSocket<N>>;
  qi::Future<SocketPtr> connected;
  IoService<N>& io = N::defaultIoService();
  {
    using Side = HandshakeSide<SslSocket<N>>;
    ConnectFuture connect{io};
    SslContext<N> context{Method<SslContext<N>>::sslv23};
    connect(Url{"tcp://10.11.12.13:1234"}, SslEnabled{false}, context,
      IpV6Enabled{false}, Side::client);
    connected = connect.complete();
    p = &connect;
  }
  // The connecting object is now destroyed and on top of that we wipe out its memory.
  // cppcheck-suppress deadpointer
  overwrite(p);
  // Now we unblock the resolve handler.
  nukeObject.setValue(0);
  // We wait for an error to occur.
  ASSERT_TRUE(connected.hasError());
  ASSERT_EQ(code(connected.error()), operationAborted<ErrorCode<N>>().value());
  resolveThread.join();
}

TYPED_TEST(NetConnectFuture, ResolvedBySkippingIpV6)
{
  using ConnectFuture = TypeParam;
  using namespace qi;
  using namespace qi::net;
  using namespace boost::algorithm;
  using namespace mock;
  using mock::Resolver;
  // The resolve is going to fail. Before calling the handler, we're going to
  // wait the ConnectFuture object is destroyed.
  Resolver::async_resolve = [&](Resolver::query q, Resolver::_anyResolveHandler h) {
    static N::_resolver_entry entryIpV6{{{true}}}, entryIpV4{{{false, q._host}}};
    static N::_resolver_entry* a[] = {&entryIpV6, &entryIpV6, &entryIpV4, nullptr};
    h(Error{}, Resolver::iterator{a});
  };
  std::string resolvedHost;
  LowestLayer::async_connect = [&](N::_resolver_entry e, N::_anyHandler h) {
    resolvedHost = e._e._addr._value;
    h(Error{Error::networkUnreachable});
  };
  IoService<N>& io = N::defaultIoService();
  static const std::string host = "1.2.3.4";
  using Side = HandshakeSide<SslSocket<N>>;
  ConnectFuture connect{io};
  SslContext<N> context{Method<SslContext<N>>::sslv23};
  connect(Url{"tcp://" + host + ":9876"}, SslEnabled{false}, context,
    IpV6Enabled{false}, Side::client);
  ASSERT_TRUE(connect.complete().hasError());
  ASSERT_EQ(host, resolvedHost);
  ASSERT_EQ(code(connect.complete().error()), networkUnreachable<ErrorCode<N>>().value());
}

TYPED_TEST(NetConnectFuture, OnlyIpV6EndpointsResolvedButIpV6NotAllowed)
{
  using ConnectFuture = TypeParam;
  using namespace qi;
  using namespace qi::net;
  using mock::Resolver;
  using namespace mock;
  using Entry = N::_resolver_entry;
  // The resolve is going to fail. Before calling the handler, we're going to
  // wait the ConnectFuture object is destroyed.
  Resolver::async_resolve = [](Resolver::query, Resolver::_anyResolveHandler h) {
    static Entry entryIpV6{{{true}}};
    static Entry* a[] = {&entryIpV6, &entryIpV6, &entryIpV6, nullptr};
    h(success<Error>(), Resolver::iterator{a});
  };
  IoService<N>& io = N::defaultIoService();
  using Side = HandshakeSide<SslSocket<N>>;
  ConnectFuture connect{io};
  SslContext<N> context{Method<SslContext<N>>::sslv23};
  connect(Url{"tcp://10.11.12.13:1234"}, SslEnabled{false}, context,
    IpV6Enabled{false}, Side::client);
  ASSERT_TRUE(connect.complete().hasError());
  ASSERT_EQ(code(connect.complete().error()), hostNotFound<ErrorCode<N>>().value());
}

// Test that when the connect callback is called and its parent has been destroyed,
// the connected future is in error.
TYPED_TEST(NetConnectFuture, ConnectCalledAfterParentHasBeenDestroyed)
{
  using ConnectFuture = TypeParam;
  using namespace qi;
  using namespace qi::net;
  using namespace boost::algorithm;
  using mock::Resolver;
  using namespace mock;
  std::thread resolveThread;
  qi::Promise<void> nukeObject;
  // The resolve is going to fail. Before calling the handler, we're going to
  // wait the ConnectFuture object is destroyed.
  Resolver::async_resolve = defaultAsyncResolve;
  LowestLayer::async_connect = [=, &resolveThread](N::_resolver_entry, N::_anyHandler h) {
    resolveThread = std::thread{[=]{
      // Wait for the object destruction.
      nukeObject.future().wait();
      h(operationAborted<ErrorCode<N>>());
    }};
  };
  using SocketPtr = boost::shared_ptr<SslSocket<N>>;
  Future<SocketPtr> connected;
  IoService<N>& io = N::defaultIoService();
  {
    using Side = HandshakeSide<SslSocket<N>>;
    ConnectFuture connect{io};
    SslContext<N> context{Method<SslContext<N>>::sslv23};
    connect(Url{"tcp://10.11.12.13:1234"}, SslEnabled{false}, context,
      IpV6Enabled{false}, Side::client);
    connected = connect.complete();
  }
  // Now we unblock the resolve handler.
  nukeObject.setValue(0);
  // We wait for an error to occur.
  ASSERT_TRUE(connected.hasError());
  ASSERT_EQ(code(connected.error()), operationAborted<ErrorCode<N>>().value());
  resolveThread.join();
}

TYPED_TEST(NetConnectFuture, FailsOnConnect)
{
  using ConnectFuture = TypeParam;
  using namespace qi;
  using namespace qi::net;
  using namespace boost::algorithm;
  using mock::Resolver;
  using namespace mock;
  Resolver::async_resolve = defaultAsyncResolve;
  std::string resolvedHost;
  LowestLayer::async_connect = [&](N::_resolver_entry e, N::_anyHandler h) {
    resolvedHost = e._e._addr._value;
    h(Error{Error::connectionRefused});
  };
  IoService<N>& io = N::defaultIoService();
  using Side = HandshakeSide<SslSocket<N>>;
  ConnectFuture connect{io};
  SslContext<N> context{Method<SslContext<N>>::sslv23};
  connect(Url{"tcp://10.11.12.13:1234"}, SslEnabled{false}, context,
    IpV6Enabled{false}, Side::client);
  ASSERT_TRUE(connect.complete().hasError());
  ASSERT_EQ(code(connect.complete().error()), connectionRefused<ErrorCode<N>>().value());
  ASSERT_EQ("10.11.12.13", resolvedHost);
}

TYPED_TEST(NetConnectFuture, SucceedsNonSsl)
{
  using ConnectFuture = TypeParam;
  using namespace qi;
  using namespace qi::net;
  using mock::Resolver;
  using namespace mock;
  Resolver::async_resolve = defaultAsyncResolve;
  LowestLayer::async_connect = defaultAsyncConnect;
  IoService<N>& io = N::defaultIoService();
  using Side = HandshakeSide<SslSocket<N>>;
  ConnectFuture connect{io};
  SslContext<N> context{Method<SslContext<N>>::sslv23};
  connect(Url{"tcp://10.11.12.13:1234"}, SslEnabled{false}, context,
    IpV6Enabled{false}, Side::client);
  ASSERT_TRUE(connect.complete().hasValue());
}

TYPED_TEST(NetConnectFuture, FailsOnHandshake)
{
  using ConnectFuture = TypeParam;
  using namespace qi;
  using namespace qi::net;
  using namespace boost::algorithm;
  using mock::Resolver;
  using namespace mock;
  Resolver::async_resolve = defaultAsyncResolve;
  LowestLayer::async_connect = defaultAsyncConnect;
  Socket::async_handshake = [=](Socket::handshake_type, N::_anyHandler h) {
    h(Error{Error::sslErrors});
  };
  IoService<N>& io = N::defaultIoService();
  using Side = HandshakeSide<SslSocket<N>>;
  ConnectFuture connect{io};
  SslContext<N> context{Method<SslContext<N>>::sslv23};
  connect(Url{"tcp://10.11.12.13:1234"}, SslEnabled{true}, context,
    IpV6Enabled{false}, Side::client);
  ASSERT_TRUE(connect.complete().hasError());
  const auto s = connect.complete().error();
  ASSERT_EQ(code(s), ErrorCode<N>::sslErrors);
}

// Test that when the handshake callback is called and its parent has been destroyed,
// the connected future is in error.
TYPED_TEST(NetConnectFuture, HandshakeHandlerCalledAfterParentHasBeenDestroyed)
{
  using ConnectFuture = TypeParam;
  using namespace qi;
  using namespace qi::net;
  using namespace boost::algorithm;
  using mock::Resolver;
  using namespace mock;
  Resolver::async_resolve = defaultAsyncResolve;
  LowestLayer::async_connect = defaultAsyncConnect;
  qi::Promise<void> nukeObject;
  std::thread t;
  // The handshake is going to fail. Before calling the handler, we're going to
  // wait the ConnectFuture object is destroyed.
  Socket::async_handshake = [&](Socket::handshake_type, N::_anyHandler h) {
    // We launch asynchronously to return immediately.
    t = std::move(std::thread([=]{
      // Wait for the object destruction.
      nukeObject.future().wait();
      // Now call the handler.
      h(operationAborted<ErrorCode<N>>());
    }));
  };
  ConnectFuture* p = nullptr;
  qi::Future<boost::shared_ptr<SslSocket<N>>> connected;
  IoService<N>& io = N::defaultIoService();
  {
    using Side = HandshakeSide<SslSocket<N>>;
    ConnectFuture connect{io};
    SslContext<N> context{Method<SslContext<N>>::sslv23};
    connect(Url{"tcp://10.11.12.13:1234"}, SslEnabled{true}, context,
      IpV6Enabled{false}, Side::client);
    connected = connect.complete();
    p = &connect;
  }
  // The connecting object is now destroyed and on top of that we wipe out its memory.
  // cppcheck-suppress deadpointer
  overwrite(p);
  // Now we unblock the handler.
  nukeObject.setValue(0);
  // We wait for an error to occur.
  ASSERT_TRUE(connected.hasError());
  ASSERT_EQ(code(connected.error()), operationAborted<ErrorCode<N>>().value());
  t.join();
}

TYPED_TEST(NetConnectFuture, SucceedsSsl)
{
  using ConnectFuture = TypeParam;
  using namespace qi;
  using namespace qi::net;
  using namespace mock;
  using mock::Resolver;
  Resolver::async_resolve = defaultAsyncResolve;
  LowestLayer::async_connect = defaultAsyncConnect;
  Socket::async_handshake = defaultAsyncHandshake;
  IoService<N>& io = N::defaultIoService();
  using Side = HandshakeSide<SslSocket<N>>;
  ConnectFuture connect{io};
  SslContext<N> context{Method<SslContext<N>>::sslv23};
  connect(Url{"tcp://10.11.12.13:1234"}, SslEnabled{true}, context,
    IpV6Enabled{false}, Side::client);
  ASSERT_TRUE(connect.complete().hasValue());
}
