#include <thread>
#include <boost/algorithm/string/predicate.hpp>
#include <gtest/gtest.h>
#include <src/messaging/sock/connect.hpp>
#include <src/messaging/sock/networkasio.hpp>
#include "src/messaging/tcpmessagesocket.hpp"
#include <qi/future.hpp>
#include <ka/scoped.hpp>
#include <qi/url.hpp>
#include "networkmock.hpp"
#include "networkcommon.hpp"

////////////////////////////////////////////////////////////////////////////////
// NetConnectSocket tests:
////////////////////////////////////////////////////////////////////////////////

TEST(NetConnectSocket, ResolveCalledAfterParentHasBeenDestroyed)
{
  using namespace qi;
  using namespace qi::sock;
  using N = mock::Network;
  using S = SslSocket<N>;

  std::thread resolveThread;
  Promise<void> nukeObject;
  // The resolve is going to fail. Before calling the handler, we're going to
  // wait the ConnectSocketFuture object is destroyed.
  auto _ = ka::scoped_set_and_restore(
    Resolver<N>::async_resolve,
    [&](Query<Resolver<N>>, Resolver<N>::_anyResolveHandler h) {
      // We launch asynchronously to return immediately.
      resolveThread = std::thread{[=]{
        // Wait for the object destruction.
        nukeObject.future().wait();
        // Now call the handler.
        h(operationAborted<ErrorCode<N>>(), Resolver<N>::iterator{});
      }};
    }
  );
  using C = ConnectSocket<N, S>;
  typename std::aligned_storage<sizeof(C), alignof(C)>::type storage;
  auto& connect = reinterpret_cast<C&>(storage);
  Promise<ErrorCode<N>> promiseError;
  IoService<N>& io = N::defaultIoService();
  {
    using Side = HandshakeSide<S>;
    auto _ = ka::scoped(new (&connect) C{io}, [](C* p){ p->~C(); });
    SslContext<N> context { Method<SslContext<N>>::tlsv12 };
    connect(Url{"tcp://10.11.12.13:1234"},
      [&] { return makeSslSocketPtr<N>(io, context); },
      IpV6Enabled{false}, Side::client,
      [=](ErrorCode<N> e, SocketPtr<S>) mutable {
        promiseError.setValue(e);
      }
    );
  }
  // The connecting object is now destroyed and on top of that we wipe out its memory.
  overwrite(&connect);
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

namespace qi { namespace sock {

/// Network N,
/// With NetSslSocket S:
///   S is compatible with N
template<typename N, typename S>
struct ConnectingWrap
{
  using Handshake = HandshakeSide<S>;
  IoService<N>& _io;
  std::unique_ptr<Connecting<N, S>> _connecting;
  Promise<SocketPtr<S>> _complete;

  /// Adapt Connecting to behave as ConnectSocket.
  /// This allows to share unit tests for these two types.
  ConnectingWrap(IoService<N>& io)
    : _io(io)
  {
  }
  template<typename Proc>
  void operator()(const Url& url, Proc&& makeSocket, IpV6Enabled ipV6, Handshake side,
    const boost::optional<Seconds>& tcpPingTimeout = boost::optional<Seconds>{})
  {
    _connecting.reset(new Connecting<N, S>{_io, url, makeSocket, ipV6, side, tcpPingTimeout});
    auto complete = _complete;
    _connecting->complete().then(
      [=](Future<SyncConnectingResultPtr<N, S>> fut) mutable {
        const ConnectingResult<N, S> res = fut.value()->get(); // copy the result
        if (hasError(res))
        {
          complete.setError(res.errorMessage);
        }
        else
        {
          complete.setValue(res.socket);
        }
      }
    );
  }
  ~ConnectingWrap()
  {
  }
  Future<SocketPtr<S>> complete() const
  {
    return _complete.future();
  }
};

}} // namespace qi::sock

using sequences = testing::Types<
  // Mock
  qi::sock::ConnectSocketFuture<mock::N, qi::sock::SslSocket<mock::N>>,
  qi::sock::ConnectingWrap<mock::N, qi::sock::SslSocket<mock::N>>
  // Asio
  //, qi::sock::ConnectSocketFuture<qi::sock::NetworkAsio>, qi::sock::Connecting<qi::sock::NetworkAsio>
>;

TYPED_TEST_SUITE(NetConnectFuture, sequences);

TYPED_TEST(NetConnectFuture, FailsOnResolve)
{
  using ConnectFuture = TypeParam;
  using namespace qi;
  using namespace qi::sock;
  using N = mock::Network;
  using S = SslSocket<N>;

  std::string receivedHost, receivedPort;
  auto _ = ka::scoped_set_and_restore(
    Resolver<N>::async_resolve,
    [&](Query<Resolver<N>> q, Resolver<N>::_anyResolveHandler h) {
      receivedHost = q._host;
      receivedPort = q._port;
      h(networkUnreachable<ErrorCode<N>>(), Resolver<N>::iterator{});
    }
  );
  IoService<N>& io = N::defaultIoService();
  const Url url{"tcp://10.11.12.13:1234"};
  using Side = HandshakeSide<S>;
  ConnectFuture connect{io};
  SslContext<N> context{ Method<SslContext<N>>::tlsv12 };
  connect(url, [&] { return makeSslSocketPtr<N>(io, context); },
          IpV6Enabled{ false }, Side::client);
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
  using namespace qi::sock;
  using namespace boost::algorithm;
  using N = mock::Network;
  using S = SslSocket<N>;

  std::thread resolveThread;
  Promise<void> nukeObject;
  // The resolve is going to fail. Before calling the handler, we're going to
  // wait the ConnectFuture object is destroyed.
  auto _ = ka::scoped_set_and_restore(
    Resolver<N>::async_resolve,
    [&](Query<Resolver<N>>, Resolver<N>::_anyResolveHandler h) {
      // We launch asynchronously to return immediately.
      resolveThread = std::thread{[=]{
        // Wait for the object destruction.
        nukeObject.future().wait();
        // Now call the handler.
        h(operationAborted<ErrorCode<N>>(), Resolver<N>::iterator{});
      }};
    }
  );

  using C = ConnectFuture;
  typename std::aligned_storage<sizeof(C), alignof(C)>::type storage;
  auto& connect = reinterpret_cast<C&>(storage);
  qi::Future<SocketPtr<S>> connected;
  IoService<N>& io = N::defaultIoService();
  {
    using Side = HandshakeSide<S>;
    auto _ = ka::scoped(new (&connect) C{io}, [](C* p){ p->~C(); });
    SslContext<N> context{ Method<SslContext<N>>::tlsv12 };
    connect(Url{ "tcp://10.11.12.13:1234" },
            [&] { return makeSslSocketPtr<N>(io, context); }, IpV6Enabled{ false }, Side::client);
    connected = connect.complete();
  }
  // The connecting object is now destroyed and on top of that we wipe out its memory.
  overwrite(&connect);
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
  using namespace qi::sock;
  using namespace boost::algorithm;
  using N = mock::Network;
  using S = SslSocket<N>;

  // The resolve is going to fail. Before calling the handler, we're going to
  // wait the ConnectFuture object is destroyed.
  auto scopedResolve = ka::scoped_set_and_restore(
    Resolver<N>::async_resolve,
    [&](Query<Resolver<N>> q, Resolver<N>::_anyResolveHandler h) {
      static N::_resolver_entry entryIpV6{{{true}}}, entryIpV4{{{false, q._host}}};
      static N::_resolver_entry* a[] = {&entryIpV6, &entryIpV6, &entryIpV4, nullptr};
      h(ErrorCode<N>{}, Resolver<N>::iterator{a});
    }
  );
  std::string resolvedHost;
  auto scopedConnect = ka::scoped_set_and_restore(
    Lowest<S>::async_connect,
    [&](N::_resolver_entry e, N::_anyHandler h) {
      resolvedHost = e._e._addr._value;
      h(ErrorCode<N>{ErrorCode<N>::networkUnreachable});
    }
  );
  IoService<N>& io = N::defaultIoService();
  static const std::string host = "1.2.3.4";
  using Side = HandshakeSide<S>;
  ConnectFuture connect{io};
  SslContext<N> context{ Method<SslContext<N>>::tlsv12 };
  connect(Url{ "tcp://" + host + ":9876" },
          [&] { return makeSslSocketPtr<N>(io, context); }, IpV6Enabled{ false }, Side::client);
  ASSERT_TRUE(connect.complete().hasError());
  ASSERT_EQ(host, resolvedHost);
  ASSERT_EQ(code(connect.complete().error()), networkUnreachable<ErrorCode<N>>().value());
}

TYPED_TEST(NetConnectFuture, OnlyIpV6EndpointsResolvedButIpV6NotAllowed)
{
  using ConnectFuture = TypeParam;
  using namespace qi;
  using namespace qi::sock;
  using N = mock::Network;
  using S = SslSocket<N>;
  using Entry = N::_resolver_entry;

  // The resolve is going to fail. Before calling the handler, we're going to
  // wait the ConnectFuture object is destroyed.
  auto _ = ka::scoped_set_and_restore(
    Resolver<N>::async_resolve,
    [](Query<Resolver<N>>, Resolver<N>::_anyResolveHandler h) {
      static Entry entryIpV6{{{true}}};
      static Entry* a[] = {&entryIpV6, &entryIpV6, &entryIpV6, nullptr};
      h(success<ErrorCode<N>>(), Resolver<N>::iterator{a});
    }
  );
  IoService<N>& io = N::defaultIoService();
  using Side = HandshakeSide<S>;
  ConnectFuture connect{io};
  SslContext<N> context{ Method<SslContext<N>>::tlsv12 };
  connect(Url{ "tcp://10.11.12.13:1234" },
          [&] { return makeSslSocketPtr<N>(io, context); }, IpV6Enabled{ false }, Side::client);
  ASSERT_TRUE(connect.complete().hasError());
  ASSERT_EQ(code(connect.complete().error()), hostNotFound<ErrorCode<N>>().value());
}

// Test that when the connect callback is called and its parent has been destroyed,
// the connected future is in error.
TYPED_TEST(NetConnectFuture, ConnectCalledAfterParentHasBeenDestroyed)
{
  using ConnectFuture = TypeParam;
  using namespace qi;
  using namespace qi::sock;
  using namespace boost::algorithm;
  using N = mock::Network;
  using S = SslSocket<N>;

  std::thread resolveThread;
  qi::Promise<void> nukeObject;
  // The resolve is going to fail. Before calling the handler, we're going to
  // wait the ConnectFuture object is destroyed.
  auto scopedResolve = ka::scoped_set_and_restore(
    Resolver<N>::async_resolve,
    mock::defaultAsyncResolve
  );
  auto scopedConnect = ka::scoped_set_and_restore(
    Lowest<S>::async_connect,
    [=, &resolveThread](N::_resolver_entry, N::_anyHandler h) {
      resolveThread = std::thread{[=]{
        // Wait for the object destruction.
        nukeObject.future().wait();
        h(operationAborted<ErrorCode<N>>());
      }};
    }
  );
  Future<SocketPtr<S>> connected;
  IoService<N>& io = N::defaultIoService();
  {
    using Side = HandshakeSide<S>;
    ConnectFuture connect{io};
    SslContext<N> context{ Method<SslContext<N>>::tlsv12 };
    connect(Url{ "tcp://10.11.12.13:1234" },
            [&] { return makeSslSocketPtr<N>(io, context); }, IpV6Enabled{ false }, Side::client);
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
  using namespace qi::sock;
  using N = mock::Network;
  using S = SslSocket<N>;

  auto scopedResolve = ka::scoped_set_and_restore(
    Resolver<N>::async_resolve,
    mock::defaultAsyncResolve
  );
  std::string resolvedHost;
  auto scopedConnect = ka::scoped_set_and_restore(
    Lowest<S>::async_connect,
    [&](N::_resolver_entry e, N::_anyHandler h) {
      resolvedHost = e._e._addr._value;
      h(ErrorCode<N>{ErrorCode<N>::connectionRefused});
    }
  );
  IoService<N>& io = N::defaultIoService();
  using Side = HandshakeSide<S>;
  ConnectFuture connect{io};
  SslContext<N> context{ Method<SslContext<N>>::tlsv12 };
  connect(Url{ "tcp://10.11.12.13:1234" },
          [&] { return makeSslSocketPtr<N>(io, context); }, IpV6Enabled{ false }, Side::client);
  ASSERT_TRUE(connect.complete().hasError());
  ASSERT_EQ(code(connect.complete().error()), connectionRefused<ErrorCode<N>>().value());
  ASSERT_EQ("10.11.12.13", resolvedHost);
}

TYPED_TEST(NetConnectFuture, SucceedsNonSsl)
{
  using ConnectFuture = TypeParam;
  using namespace qi;
  using namespace qi::sock;
  using N = mock::Network;
  using S = SslSocket<N>;

  auto scopedResolve = ka::scoped_set_and_restore(
    Resolver<N>::async_resolve,
    mock::defaultAsyncResolve
  );
  auto scopedConnect = ka::scoped_set_and_restore(
    Lowest<S>::async_connect,
    mock::defaultAsyncConnect
  );
  IoService<N>& io = N::defaultIoService();
  using Side = HandshakeSide<S>;
  ConnectFuture connect{io};
  SslContext<N> context{ Method<SslContext<N>>::tlsv12 };
  connect(Url{ "tcp://10.11.12.13:1234" },
          [&] { return makeSslSocketPtr<N>(io, context); }, IpV6Enabled{ false }, Side::client);
  ASSERT_TRUE(connect.complete().hasValue());
}

TYPED_TEST(NetConnectFuture, FailsOnHandshake)
{
  using ConnectFuture = TypeParam;
  using namespace qi;
  using namespace qi::sock;
  using N = mock::Network;
  using S = SslSocket<N>;

  auto scopedResolve = ka::scoped_set_and_restore(
    Resolver<N>::async_resolve,
    mock::defaultAsyncResolve
  );
  auto scopedConnect = ka::scoped_set_and_restore(
    Lowest<S>::async_connect,
    mock::defaultAsyncConnect
  );
  auto scopedHandshake = ka::scoped_set_and_restore(
    S::async_handshake,
    [=](S::handshake_type, N::_anyHandler h) {
      h(ErrorCode<N>{ErrorCode<N>::sslErrors});
    }
  );
  IoService<N>& io = N::defaultIoService();
  using Side = HandshakeSide<S>;
  ConnectFuture connect{io};
  SslContext<N> context{ Method<SslContext<N>>::tlsv12 };
  connect(Url{ "tcps://10.11.12.13:1234" },
          [&] { return makeSslSocketPtr<N>(io, context); }, IpV6Enabled{ false }, Side::client);
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
  using namespace qi::sock;
  using N = mock::Network;
  using S = SslSocket<N>;

  Resolver<N>::async_resolve = mock::defaultAsyncResolve;
  Lowest<S>::async_connect = mock::defaultAsyncConnect;
  qi::Promise<void> nukeObject;
  std::thread t;
  // The handshake is going to fail. Before calling the handler, we're going to
  // wait the ConnectFuture object is destroyed.
  auto _ = ka::scoped_set_and_restore(
    S::async_handshake,
    [&](S::handshake_type, N::_anyHandler h) {
      // We launch asynchronously to return immediately.
      t = std::thread([=]{
        // Wait for the object destruction.
        nukeObject.future().wait();
        // Now call the handler.
        h(operationAborted<ErrorCode<N>>());
      });
    }
  );
  using C = ConnectFuture;
  typename std::aligned_storage<sizeof(C), alignof(C)>::type storage;
  auto& connect = reinterpret_cast<C&>(storage);
  qi::Future<SocketPtr<S>> connected;
  IoService<N>& io = N::defaultIoService();
  {
    using Side = HandshakeSide<S>;
    auto _ = ka::scoped(new (&connect) C{io}, [](C* p){ p->~C(); });
    SslContext<N> context{ Method<SslContext<N>>::tlsv12 };
    connect(Url{ "tcps://10.11.12.13:1234" },
            [&] { return makeSslSocketPtr<N>(io, context); }, IpV6Enabled{ false }, Side::client);
    connected = connect.complete();
  }
  // The connecting object is now destroyed and on top of that we wipe out its memory.
  overwrite(&connect);
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
  using namespace qi::sock;
  using N = mock::Network;
  using S = SslSocket<N>;

  auto scopedResolve = ka::scoped_set_and_restore(
    Resolver<N>::async_resolve,
    mock::defaultAsyncResolve
  );
  auto scopedConnect = ka::scoped_set_and_restore(
    Lowest<S>::async_connect,
    mock::defaultAsyncConnect
  );
  auto scopedHandshake = ka::scoped_set_and_restore(
    S::async_handshake,
    mock::defaultAsyncHandshake
  );
  IoService<N>& io = N::defaultIoService();
  using Side = HandshakeSide<S>;
  ConnectFuture connect{io};
  SslContext<N> context{ Method<SslContext<N>>::tlsv12 };
  connect(Url{ "tcps://10.11.12.13:1234" },
          [&] { return makeSslSocketPtr<N>(io, context); }, IpV6Enabled{ false }, Side::client);
  ASSERT_TRUE(connect.complete().hasValue());
}

template<typename N, typename S>
struct SetupStop
{
  using I = qi::sock::Iterator<qi::sock::Resolver<N>>;

  qi::Future<void> futStopResolve;
  qi::Future<void> futStopConnect;
  bool connectAlreadySetup;
  qi::Promise<std::pair<qi::sock::ErrorCode<N>, I>> promiseResolve;
  qi::Promise<qi::sock::ErrorCode<N>> promiseConnect;

  void operator()(qi::sock::Resolver<N>&)
  {
    using namespace qi::sock;
    auto promResolve = promiseResolve;
    futStopResolve.andThen([=](void*) mutable {
      promResolve.setValue({operationAborted<ErrorCode<N>>(), I{}});
    });
  }

  void operator()(const qi::sock::SocketPtr<S>&)
  {
    using namespace qi::sock;
    // Can be called in the connection step and in the handshake step.
    // The stop action being the same, we do it only once.
    if (connectAlreadySetup) return;
    auto promConnect = promiseConnect;
    futStopConnect.andThen([=](void*) mutable {
      promConnect.setValue(operationAborted<ErrorCode<N>>());
    });
    connectAlreadySetup = true;
  }
};

// Stop called while in the resolving step.
//
// The resolve step waits for a promise to be set. This promise is set when the
// stop promise is set.
TEST(NetConnectFutureStop, WhileResolving)
{
  using namespace qi;
  using namespace qi::sock;
  using N = mock::Network;
  using S = SslSocket<N>;

  Promise<std::pair<ErrorCode<N>, Iterator<Resolver<N>>>> promiseResolve;
  Promise<void> promiseStopResolve;
  Promise<void> promiseStopConnect;
  Promise<ErrorCode<N>> promiseConnect;
  std::thread threadResolve;
  auto scopedResolve = ka::scoped_set_and_restore(
    Resolver<N>::async_resolve,
    [&](Query<Resolver<N>>, Resolver<N>::_anyResolveHandler h) {
      threadResolve = std::thread{[=]() mutable {
        // Block until the resolve promise has been set.
        auto p = promiseResolve.future().value();
        h(p.first, p.second);
      }};
    }
  );
  auto scopedConnect = ka::scoped_set_and_restore(
    Lowest<S>::async_connect,
    mock::defaultAsyncConnect
  );
  auto scopedHandshake = ka::scoped_set_and_restore(
    S::async_handshake,
    mock::defaultAsyncHandshake
  );
  IoService<N>& io = N::defaultIoService();
  using Side = HandshakeSide<S>;
  ConnectSocketFuture<N, S> connect{io};
  SslContext<N> context{ Method<SslContext<N>>::tlsv12 };
  connect(Url{"tcps://10.11.12.13:1234"},
    [&] { return makeSslSocketPtr<N>(io, context); },
    IpV6Enabled{false}, Side::client, Seconds{100},
    SetupStop<N, S>{promiseStopResolve.future(), promiseStopConnect.future(), false,
      promiseResolve, promiseConnect}
  );

  // Trigger the stop.
  promiseStopResolve.setValue(nullptr);
  ASSERT_TRUE(connect.complete().hasError());
  ASSERT_EQ("5: operationAborted", connect.complete().error());
  threadResolve.join();
}

// Stop called while in the connect step.
//
// The connect step waits for a promise to be set. This promise is set when the
// stop promise is set.
TEST(NetConnectFutureStop, WhileConnecting)
{
  using namespace qi;
  using namespace qi::sock;
  using N = mock::Network;
  using S = SslSocket<N>;

  Promise<std::pair<ErrorCode<N>, Iterator<Resolver<N>>>> promiseResolve;
  Promise<void> promiseStopResolve;
  Promise<void> promiseStopConnect;
  Promise<ErrorCode<N>> promiseConnect;
  std::thread threadConnect;
  auto scopedResolve = ka::scoped_set_and_restore(
    Resolver<N>::async_resolve,
    mock::defaultAsyncResolve
  );
  auto scopedConnect = ka::scoped_set_and_restore(
    Lowest<S>::async_connect,
    [&](N::_resolver_entry, N::_anyHandler h) {
      threadConnect = std::thread{[=]() mutable {
        // Block until the resolve promise has been set.
        h(promiseConnect.future().value());
      }};
    }
  );
  auto scopedHandshake = ka::scoped_set_and_restore(
    S::async_handshake,
    mock::defaultAsyncHandshake
  );
  IoService<N>& io = N::defaultIoService();
  using Side = HandshakeSide<S>;
  ConnectSocketFuture<N, S> connect{io};
  SslContext<N> context{ Method<SslContext<N>>::tlsv12 };
  connect(Url{"tcps://10.11.12.13:1234"},
    [&] { return makeSslSocketPtr<N>(io, context); },
    IpV6Enabled{false}, Side::client, Seconds{100},
    SetupStop<N, S>{promiseStopResolve.future(), promiseStopConnect.future(), false,
      promiseResolve, promiseConnect}
  );

  // Trigger the stop.
  promiseStopConnect.setValue(nullptr);
  ASSERT_TRUE(connect.complete().hasError());
  ASSERT_EQ("5: operationAborted", connect.complete().error());
  threadConnect.join();
}

// Stop called while in the handshake step.
//
// The handshake step waits for a promise to be set. This promise is set when the
// stop promise is set.
TEST(NetConnectFutureStop, WhileHandshaking)
{
  using namespace qi;
  using namespace qi::sock;
  using N = mock::Network;
  using S = SslSocket<N>;

  Promise<std::pair<ErrorCode<N>, Iterator<Resolver<N>>>> promiseResolve;
  Promise<void> promiseStopResolve;
  Promise<void> promiseStopConnect;
  Promise<ErrorCode<N>> promiseConnect;
  std::thread threadHandshake;
  auto scopedResolve = ka::scoped_set_and_restore(
    Resolver<N>::async_resolve,
    mock::defaultAsyncResolve
  );
  auto scopedConnect = ka::scoped_set_and_restore(
    Lowest<S>::async_connect,
    mock::defaultAsyncConnect
  );
  auto scopedHandshake = ka::scoped_set_and_restore(
    S::async_handshake,
    [&](HandshakeSide<S>, N::_anyHandler h) {
      threadHandshake = std::thread{[=]() mutable {
        // Block until the resolve promise has been set.
        h(promiseConnect.future().value());
      }};
    }
  );
  IoService<N>& io = N::defaultIoService();
  using Side = HandshakeSide<S>;
  ConnectSocketFuture<N, S> connect{io};
  SslContext<N> context{ Method<SslContext<N>>::tlsv12 };
  connect(Url{"tcps://10.11.12.13:1234"},
    [&] { return makeSslSocketPtr<N>(io, context); },
    IpV6Enabled{false}, Side::client, Seconds{100},
    SetupStop<N, S>{promiseStopResolve.future(), promiseStopConnect.future(), false,
      promiseResolve, promiseConnect}
  );

  // Trigger the stop.
  promiseStopConnect.setValue(nullptr);
  ASSERT_TRUE(connect.complete().hasError());
  ASSERT_EQ("5: operationAborted", connect.complete().error());
  threadHandshake.join();
}
