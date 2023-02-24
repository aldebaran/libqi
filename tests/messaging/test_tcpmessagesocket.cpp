#include <chrono>
#include <functional>
#include <numeric>
#include <random>
#include <gtest/gtest.h>
#include <gmock/gmock-matchers.h>
#include <qi/uri.hpp>
#include <qi/messaging/tcpscheme.hpp>
#include "sock/networkmock.hpp"
#include "sock/networkcommon.hpp"
#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <openssl/asn1.h>
#include <src/messaging/sock/accept.hpp>
#include "src/messaging/sock/option.hpp"
#include "src/messaging/tcpmessagesocket.hpp"
#include "src/messaging/transportserver.hpp"
#include "tests/qi/testutils/testutils.hpp"
#include <testssl/testssl.hpp>
#include "ka/scoped.hpp"

static const qi::MilliSeconds defaultTimeout{ 2000 };

////////////////////////////////////////////////////////////////////////////////
// Connected tests
////////////////////////////////////////////////////////////////////////////////

namespace mock
{
  template<typename H> // NetHandler H
  void readHeader(N::_mutable_buffer_sequence buf, H h, qi::uint32_t magic = qi::Message::Header::magicCookie, qi::uint32_t size = 10u, N::error_code_type error = {})
  {
    qi::Message::Header header;
    header.magic = magic;
    header.size = size;
    assert(std::distance(buf.begin, buf.end) >= static_cast<std::ptrdiff_t>(sizeof(header)));
    auto* p = reinterpret_cast<decltype(buf.begin)>(&header);
    std::copy(p, p + sizeof(header), buf.begin);
    h(error, sizeof(header));
  }

  /// A read handler that alternatively reads the message header, then the message
  /// data.
  struct AsyncReadNextLayerHeaderThenData
  {
    qi::uint32_t _magic = qi::Message::Header::magicCookie;
    qi::uint32_t _headerSize = 10u;
    N::error_code_type _headerError = {};
    N::error_code_type _dataError = {};
    int _callCount = 0;
    void operator()(N::ssl_socket_type::next_layer_type&, N::_mutable_buffer_sequence buf, N::_anyTransferHandler h)
    {
      ++_callCount;
      if (_callCount % 2 == 1)
      {
        readHeader(buf, h, _magic, _headerSize, _headerError);
      }
      else
      {
        // read data
        h(_dataError, 0);
      }
    }
  };
} // namespace mock

qi::Message makeMessage(const qi::MessageAddress& address)
{
  using namespace qi;
  Message msg{Message::Type_Call, address};
  Buffer bufSend;
  {
    std::vector<int> data(10000);
    std::iota(data.begin(), data.end(), 42);
    bufSend.write(&data[0], data.size() * sizeof(data[0]));
    msg.setBuffer(bufSend);
  }
  return msg;
}

template<typename N>
struct NoMoreMessage
{
  boost::optional<qi::Message*> operator()(qi::sock::ErrorCode<N> e, qi::Message*) {
    if (e) throw std::runtime_error("error sending message");
    return boost::optional<qi::Message*>{};
  }
};

// TODO: Replace this by Message == when the associated patch is available.
bool messageEqual(const qi::Message& m0, const qi::Message& m1)
{
  if (m0.address() != m1.address()) return false;
  if (m0.buffer().size() != m1.buffer().size()) return false;
  auto b0 = static_cast<const char*>(m0.buffer().data());
  return std::equal(b0, b0 + m0.buffer().size(), (char*)m1.buffer().data());
}

template<typename... Args>
struct DisconnectSignal
{
  qi::SignalLink link = qi::SignalBase::invalidSignalLink;
  qi::Signal<Args...>* signal = nullptr;

  DisconnectSignal(qi::SignalLink link, qi::Signal<Args...>* signal)
    : link{link}
    , signal{signal}
  {}

  void operator()()
  {
    if (signal && qi::isValidSignalLink(link))
      signal->disconnect(link);
  }
};

struct SignalPromises
{
  qi::Promise<void> _connectedReceived;
  qi::Promise<std::string> _disconnectedReceived;

  qi::Future<void> connectedReceived()
  {
    return _connectedReceived.future();
  }
  qi::Future<std::string> disconnectedReceived()
  {
    return _disconnectedReceived.future();
  }
};

struct SignalConnection
{
  SignalPromises promises;
  std::function<void()> disconnect = ka::constant_function();

  ~SignalConnection()
  {
    disconnect();
  }
};

using SignalConnectionPtr = std::unique_ptr<SignalConnection>;

template<typename SocketPtr>
SignalConnectionPtr connectSignals(const SocketPtr& socket)
{
  SignalConnectionPtr c { new SignalConnection };
  const auto cptr = c.get();

  const qi::SignalLink linkConnected =
      socket->connected.connect([=]() { cptr->promises._connectedReceived.setValue(0); });
  EXPECT_TRUE(qi::isValidSignalLink(linkConnected));
  DisconnectSignal<> disconnectConnected{ linkConnected, &socket->connected };

  const qi::SignalLink linkDisconnected = socket->disconnected.connect(
      [=](const std::string& msg) { cptr->promises._disconnectedReceived.setValue(msg); });
  EXPECT_TRUE(qi::isValidSignalLink(linkDisconnected));
  DisconnectSignal<std::string> disconnectDisconnected{ linkDisconnected, &socket->disconnected };

  const auto lifetimeTransfo = ka::data_bound_transfo(socket);
  c->disconnect = lifetimeTransfo([=]() mutable {
    disconnectConnected();
    disconnectDisconnected();
  });

  return c;
}

namespace {
  template<qi::TcpScheme S>
  using TcpSchemeType = std::integral_constant<qi::TcpScheme, S>;
  struct TcpSchemeRaw : TcpSchemeType<qi::TcpScheme::Raw> {};
  struct TcpSchemeTls : TcpSchemeType<qi::TcpScheme::Tls> {};
  struct TcpSchemeMutualTls : TcpSchemeType<qi::TcpScheme::MutualTls> {};

  template <qi::TcpScheme Scheme>
  qi::Future<qi::MessageSocketPtr> serverListen(qi::TransportServer& server,
                                                const std::string& host = "127.0.0.1",
                                                std::uint16_t port = 0,
                                                bool connectNewConnectionSignal = true)
  {
    using namespace qi;
    Promise<MessageSocketPtr> promiseConnectedSocket;
    if (connectNewConnectionSignal)
    {
      server.newConnection.connect([=](const std::pair<MessageSocketPtr, Url>& p) mutable
                                   { promiseConnectedSocket.setValue(p.first); });
    }
    else
    {
      promiseConnectedSocket.setValue({});
    }
    server.listen(qi::Url(host, to_string(Scheme), port));

    return promiseConnectedSocket.future();
  }

  template<typename N>
  qi::MessageSocketPtr makeClientSocket(
    qi::ssl::ClientConfig sslCfg = {},
    qi::sock::IoService<N>& ioService = N::defaultIoService())
  {
    using Socket = qi::TcpMessageSocket<N, qi::sock::SocketWithContext<N>>;
    auto socket = boost::make_shared<Socket>(ioService, std::move(sslCfg));
    return socket;
  }

  struct Connection
  {
    Connection(const Connection&) = delete;
    Connection& operator=(const Connection&) = delete;
    Connection(Connection&&) = default;
    Connection& operator=(Connection&&) = default;

    ~Connection()
    {
      if (socket)
        socket->disconnect().wait(defaultTimeout);
    }
    qi::MessageSocketPtr socket;
    qi::Future<void> complete;
  };

  Connection connect(qi::MessageSocketPtr socket, qi::Url url)
  {
    auto fut = socket->connect(url);
    return { socket, fut };
  }

  template<class S, class N>
  class NetMessageSocketFixture : public ::testing::Test
  {
  public:
    using NetworkType = N;

    qi::MessageSocketPtr makeClientSocket(
      qi::ssl::ClientConfig sslCfg = {},
      qi::sock::IoService<N>& ioService = N::defaultIoService())
    {
      return ::makeClientSocket<N>(std::move(sslCfg), ioService);
    }

    qi::Future<qi::MessageSocketPtr> serverListen(qi::TransportServer& server,
                                                  const std::string& host = "127.0.0.1",
                                                  std::uint16_t port = 0,
                                                  bool connectNewConnectionSignal = true)
    {
      return ::serverListen<S::value>(server, host, port, connectNewConnectionSignal);
    }

    static std::string scheme()
    {
      return to_string(S::value);
    }
  };

  template<class SchemeType>
  using NetMessageSocket = NetMessageSocketFixture<SchemeType, mock::Network>;
  template<class SchemeType>
  using NetMessageSocketAsio = NetMessageSocketFixture<SchemeType, qi::sock::NetworkAsio>;

  using SchemeTypes = ::testing::Types<TcpSchemeRaw,
                                       TcpSchemeTls,
                                       TcpSchemeMutualTls>;
  TYPED_TEST_SUITE(NetMessageSocket, SchemeTypes);
  TYPED_TEST_SUITE(NetMessageSocketAsio, SchemeTypes);

  using NetMessageSocketTcpRaw = NetMessageSocket<TcpSchemeRaw>;
  using NetMessageSocketTcpTls = NetMessageSocket<TcpSchemeTls>;
  using NetMessageSocketTcpMutualTls = NetMessageSocket<TcpSchemeMutualTls>;

  using NetMessageSocketAsioTcpRaw = NetMessageSocketAsio<TcpSchemeRaw>;
  using NetMessageSocketAsioTcpTls = NetMessageSocketAsio<TcpSchemeTls>;
  using NetMessageSocketAsioTcpMutualTls = NetMessageSocketAsio<TcpSchemeMutualTls>;
}

TYPED_TEST(NetMessageSocketAsio, DestroyNotConnected)
{
  using namespace qi;
  using namespace qi::sock;

  SignalConnectionPtr signalConnection;
  {
    auto clientSideSocket = this->makeClientSocket();
    signalConnection = connectSignals(clientSideSocket);

    // Check that signal were not emitted.
    ASSERT_TRUE(signalConnection->promises.connectedReceived().isRunning());
    ASSERT_TRUE(signalConnection->promises.disconnectedReceived().isRunning());
  }

  // Socket is destroyed : disconnect must not be emitted since it was not
  // connected in the first place.
  ASSERT_TRUE(signalConnection->promises.disconnectedReceived().isRunning());
}

TYPED_TEST(NetMessageSocketAsio, ConnectAndDisconnect)
{
  using namespace qi;
  using namespace qi::sock;

  TransportServer server(test::ssl::serverConfig(test::ssl::server(), test::ssl::rootCA()));
  this->serverListen(server);
  const auto url = toUrl(server.endpoints().front());

  // We also want to check that signals are emitted.
  auto clientSideSocket = this->makeClientSocket(test::ssl::clientConfig(test::ssl::client(), test::ssl::rootCA()));
  auto signalConnection = connectSignals(clientSideSocket);

  const auto connection = connect(clientSideSocket, url);
  ASSERT_TRUE(test::finishesWithValue(connection.complete));
  ASSERT_TRUE(test::finishesWithValue(signalConnection->promises.connectedReceived()));
  ASSERT_TRUE(signalConnection->promises.disconnectedReceived().isRunning());

  Future<void> fut1 = clientSideSocket->disconnect();
  ASSERT_TRUE(test::finishesWithValue(fut1));
  ASSERT_TRUE(test::finishesWithValue(signalConnection->promises.disconnectedReceived()));
}

TYPED_TEST(NetMessageSocketAsio, ConnectAndDestroy)
{
  using namespace qi;
  using namespace qi::sock;

  // We also want to check that signals are emitted.
  SignalConnectionPtr signalConnection;

  {
    TransportServer server(test::ssl::serverConfig(test::ssl::server(), test::ssl::rootCA()));
    this->serverListen(server);
    const auto url = toUrl(server.endpoints().front());

    auto clientSideSocket = this->makeClientSocket(test::ssl::clientConfig(test::ssl::client(), test::ssl::rootCA()));
    signalConnection = connectSignals(clientSideSocket);

    const auto connection = connect(clientSideSocket, url);
    ASSERT_TRUE(test::finishesWithValue(connection.complete));
    ASSERT_TRUE(test::finishesWithValue(signalConnection->promises.connectedReceived()));
    ASSERT_TRUE(signalConnection->promises.disconnectedReceived().isRunning());
  }

  // disconnected must be emitted on destruction.
  ASSERT_TRUE(test::finishesWithValue(signalConnection->promises.disconnectedReceived()));
}

TYPED_TEST(NetMessageSocketAsio, SendWhileNotConnected)
{
  using namespace qi;
  using namespace qi::sock;

  auto clientSideSocket = this->makeClientSocket();
  auto signalConnection = connectSignals(clientSideSocket);

  MessageAddress address{1234, 5, 9876, 107};
  ASSERT_FALSE(clientSideSocket->send(makeMessage(address)));

  // Check that signal were not emitted.
  ASSERT_TRUE(signalConnection->promises.connectedReceived().isRunning());
  ASSERT_TRUE(signalConnection->promises.disconnectedReceived().isRunning());
}

TYPED_TEST(NetMessageSocketAsio, SendAfterDisconnected)
{
  using namespace qi;
  using namespace qi::sock;

  TransportServer server(test::ssl::serverConfig(test::ssl::server(), test::ssl::rootCA()));
  this->serverListen(server);
  const auto url = toUrl(server.endpoints().front());

  auto clientSideSocket =
    this->makeClientSocket(test::ssl::clientConfig(test::ssl::client(), test::ssl::rootCA()));
  const auto connection = connect(clientSideSocket, url);
  ASSERT_TRUE(test::finishesWithValue(connection.complete));

  Future<void> fut1 = clientSideSocket->disconnect();
  ASSERT_TRUE(test::finishesWithValue(fut1));

  MessageAddress address{1234, 5, 9876, 107};
  ASSERT_FALSE(clientSideSocket->send(makeMessage(address)));
}

TYPED_TEST(NetMessageSocketAsio, DisconnectWhileNotConnected)
{
  using namespace qi;
  using namespace qi::sock;

  auto socket = this->makeClientSocket();
  const auto _ = ka::scoped([=]{ socket->disconnect().wait(defaultTimeout); });
  ASSERT_FALSE(socket->disconnect().hasError());
}

TYPED_TEST(NetMessageSocketAsio, ReceiveOneMessage)
{
  using namespace qi;
  using namespace qi::sock;

  TransportServer server(test::ssl::serverConfig(test::ssl::server(), test::ssl::rootCA()));
  auto connectedSocket = this->serverListen(server);
  const auto url = toUrl(server.endpoints().front());

  // Connect the client.
  auto clientSideSocket =
    this->makeClientSocket(test::ssl::clientConfig(test::ssl::client(), test::ssl::rootCA()));

  auto msgSent = makeMessage(MessageAddress{1234, 5, 9876, 107});
  Promise<void> promiseReceivedMessage;
  clientSideSocket->messageReady.connect([&](const Message& msgReceived) mutable {
    if (!messageEqual(msgReceived, msgSent)) throw std::runtime_error("messages are not equal.");
    promiseReceivedMessage.setValue(0);
  });
  const auto connection = connect(clientSideSocket, url);
  ASSERT_TRUE(test::finishesWithValue(connection.complete));

  // The server sends a message.
  ASSERT_TRUE(connectedSocket.hasValue());
  ASSERT_TRUE(connectedSocket.value()->ensureReading());
  ASSERT_TRUE(connectedSocket.value()->send(msgSent));

  // Wait for the client to receive it.
  ASSERT_TRUE(test::finishesWithValue(promiseReceivedMessage.future()));
}

TYPED_TEST(NetMessageSocketAsio, ReceiveManyMessages)
{
  using namespace qi;
  using namespace qi::sock;

  TransportServer server(test::ssl::serverConfig(test::ssl::server(), test::ssl::rootCA()));
  auto connectedSocket = this->serverListen(server);
  const auto url = toUrl(server.endpoints().front());

  // Connect the client.
  auto clientSideSocket =
    this->makeClientSocket(test::ssl::clientConfig(test::ssl::client(), test::ssl::rootCA()));

  Promise<void> promiseAllMessageReceived;
  const int messageCount = 100;
  MessageAddress address{1234, 5, 9876, 107};
  std::atomic<int> i { 0 };
  clientSideSocket->messageReady.connect([=, &i](const Message& msg) mutable {
    const auto oldi = i++;
    if (oldi >= messageCount) promiseAllMessageReceived.setError("Too many messages received.");
    auto addr = address;
    addr.messageId = address.messageId + oldi;
    auto msg2 = makeMessage(addr);
    if (!messageEqual(msg, msg2))
    {
      promiseAllMessageReceived.setError("message not equal.");
    }
    if (oldi + 1 == messageCount) promiseAllMessageReceived.setValue(0);
  });
  const auto connection = connect(clientSideSocket, url);
  connection.complete.wait();
  ASSERT_TRUE(test::finishesWithValue(connection.complete));

  // The server sends a message.
  ASSERT_TRUE(connectedSocket.hasValue());
  auto serverSideSocket = connectedSocket.value();
  serverSideSocket->ensureReading();
  for (int i = 0; i != messageCount; ++i)
  {
    serverSideSocket->send(makeMessage(address));
    ++address.messageId;
  }

  // Wait for the client to receive it.
  ASSERT_TRUE(test::finishesWithValue(promiseAllMessageReceived.future()));
  qiLogInfo("") << "fin\n";
}

// The test ends while a socket connection or a server accept may be pending.
// The destruction of the corresponding objects must be fine.
// This test must typically be launched a great number of times to be meaningful.
TYPED_TEST(NetMessageSocketAsio, PrematureDestroy)
{
  using namespace qi;
  using namespace qi::sock;

  TransportServer server(test::ssl::serverConfig(test::ssl::server(), test::ssl::rootCA()));
  this->serverListen(server);
  const auto url = toUrl(server.endpoints().front());

  // Connect the client.
  auto msgSent = makeMessage(MessageAddress{1234, 5, 9876, 107});

  auto clientSideSocket =
    this->makeClientSocket(test::ssl::clientConfig(test::ssl::client(), test::ssl::rootCA()));
  auto connection = connect(clientSideSocket, url);
}

TYPED_TEST(NetMessageSocket, DisconnectWhileConnecting)
{
  using namespace qi;
  using namespace qi::sock;
  using N = mock::Network;

  std::thread threadResolve;
  auto scopedResolve = ka::scoped_set_and_restore(
    Resolver<N>::async_resolve,
    [&](Resolver<N>::query q, Resolver<N>::_anyResolveHandler h) {
        threadResolve = std::thread{[=] {
          mock::defaultAsyncResolve(q, h);
        }};
    }
  );
  auto r = ka::scoped([&] {
    threadResolve.join();
  });

  Promise<void> promiseAsyncConnectStarted;
  Promise<ErrorCode<N>> promiseAsyncConnectResult;

  // The async connect waits for a promise to be set before finishing.
  std::thread threadConnect;
  auto scopedConnect = ka::scoped_set_and_restore(
    Lowest<SslSocket<N>>::async_connect,
    [&](N::_resolver_entry, N::_anyHandler h) mutable {
      threadConnect = std::thread{[=]() mutable {
        promiseAsyncConnectStarted.setValue(0);
        auto err = promiseAsyncConnectResult.future().value();
        h(err);
      }};
    }
  );
  auto c = ka::scoped([&] {
    threadConnect.join();
  });

  using LowestLayer = Lowest<SslSocket<N>>;
  auto scopedShutdown = ka::scoped_set_and_restore(
    LowestLayer::_shutdown,
    [=](LowestLayer::shutdown_type, ErrorCode<N>) mutable {
      auto err = operationAborted<ErrorCode<N>>();
      promiseAsyncConnectResult.setValue(err);
    }
  );

  auto socket = this->makeClientSocket();
  Future<void> futConnect = socket->connect(Url{this->scheme() + "://10.11.12.13:1234"});

  // We wait for the async connect to start.
  promiseAsyncConnectStarted.future().wait();
  Future<void> futDisconnect = socket->disconnect();

  // Now the async connect (therefore the connecting part) can finish.
  ASSERT_TRUE(test::finishesWithValue(futDisconnect));
  ASSERT_TRUE(test::finishesWithError(futConnect));
  ASSERT_EQ("Connect abort: disconnection requested while connecting", futConnect.error());
}

struct ThreadedWaitFutureReadWriteFunction
{
  using N = mock::Network;
  qi::Future<qi::sock::ErrorCode<N>> futureAsyncReadWrite;
  std::vector<std::thread>& threads;

  template<typename NetSslSocket, typename Buffer>
  void operator()(NetSslSocket&, Buffer&&, N::_anyTransferHandler h) const
  {
    auto future = this->futureAsyncReadWrite;
    threads.push_back(std::thread{[=] {
        h(future.value(), 0);
    }});
  }
};


TYPED_TEST(NetMessageSocket, DisconnectWhileDisconnecting)
{
  using namespace qi;
  using namespace qi::sock;
  using N = mock::Network;

  Promise<ErrorCode<N>> promiseAsyncReadWrite;
  auto futureAsyncReadWrite = promiseAsyncReadWrite.future();

  Resolver<N>::async_resolve = mock::defaultAsyncResolve;
  Lowest<SslSocket<N>>::async_connect = mock::defaultAsyncConnect;
  SslSocket<N>::async_handshake = mock::defaultAsyncHandshake;
  std::vector<std::thread> threads;

  // Make sure async read and async write block until shutdown happens.
  auto scopedRead = ka::scoped_set_and_restore(
    N::SocketFunctions<qi::sock::SocketWithContext<N>>::_async_read_socket,
    ThreadedWaitFutureReadWriteFunction{ futureAsyncReadWrite, threads }
  );
  auto scopedReadNextLayer = ka::scoped_set_and_restore(
    N::_async_read_next_layer,
    ThreadedWaitFutureReadWriteFunction{ futureAsyncReadWrite, threads }
  );
  auto scopedWrite = ka::scoped_set_and_restore(
    N::SocketFunctions<qi::sock::SocketWithContext<N>>::_async_write_socket,
    ThreadedWaitFutureReadWriteFunction{ futureAsyncReadWrite, threads }
  );
  auto scopedWriteNextLayer = ka::scoped_set_and_restore(
    N::_async_write_next_layer,
    ThreadedWaitFutureReadWriteFunction{ futureAsyncReadWrite, threads }
  );

  using LowestLayer = Lowest<SslSocket<N>>;
  auto scopedShutdown = ka::scoped_set_and_restore(
    LowestLayer::_shutdown,
    [=](LowestLayer::shutdown_type, ErrorCode<N>&) mutable {
      if (promiseAsyncReadWrite.future().isRunning())
      {
        promiseAsyncReadWrite.setValue(shutdown<ErrorCode<N>>());
      }
    }
  );

  Promise<void> promiseCloseStarted;
  Promise<void> promiseCanFinishClose;
  auto scopedClose = ka::scoped_set_and_restore(
    LowestLayer::close,
    [=](ErrorCode<N>&) mutable {
      promiseCloseStarted.setValue(0);
      promiseCanFinishClose.future().wait(); // block
    }
  );

  auto socket = this->makeClientSocket();
  socket->connect(Url{this->scheme() + "://10.11.12.13:1234"});

  auto futDisconnect0 = socket->disconnect();
  ASSERT_TRUE(test::isStillRunning(futDisconnect0,
                                   test::willDoNothing(),
                                   qi::Seconds::zero()));

  // Wait for entering close().
  ASSERT_TRUE(test::finishesWithValue(promiseCloseStarted.future()));

  // Subsequent disconnect() cannot complete because close() is blocking.
  auto futDisconnect1 = socket->disconnect();
  ASSERT_TRUE(test::isStillRunning(futDisconnect1,
                                   test::willDoNothing(),
                                   qi::Seconds::zero()));
  auto futDisconnect2 = socket->disconnect();
  ASSERT_TRUE(test::isStillRunning(futDisconnect2,
                                   test::willDoNothing(),
                                   qi::Seconds::zero()));

  // Now, unblock close() so that disconnecting state and later
  // disconnected state can complete.
  promiseCanFinishClose.setValue(0);

  // All future must complete.
  ASSERT_TRUE(test::finishesWithValue(futDisconnect0));
  ASSERT_TRUE(test::finishesWithValue(futDisconnect1));
  ASSERT_TRUE(test::finishesWithValue(futDisconnect2));
  ASSERT_FALSE(socket->isConnected());
  ASSERT_EQ(MessageSocket::Status::Disconnected, socket->status());

  for (auto& t: threads)
    if (t.joinable())
      t.join();
}

namespace {
  // Stores a value into a `Mutable` (i.e. pointer, reference, iterator, etc.).
  template<typename T>
  struct store_mutable_t {
    T t;
  // Procedure<void (Mutable U)>:
    template<typename U>
    void operator()(U&& u) const {
      using ka::src;
      src(u).store(t);
    }
  };
  KA_DERIVE_CTOR_FUNCTION_TEMPLATE(store_mutable)

} // namespace

// Failure of setting the cipher list results in a failed connection.
TYPED_TEST(NetMessageSocket, trySetCipherListTls12AndBelowFails)
{
  using namespace qi;
  using namespace qi::sock;
  using N = mock::Network;

  auto socket = this->makeClientSocket();

  // Next try to set the cipher list will fail.
  auto p = &N::resultOfTrySetCipherListTls12AndBelow;
  auto _ = ka::scoped_apply_and_retract(
    p,
    store_mutable(false),    // Store `false` to make fail.
    store_mutable(p->load()) // Restore original value on scope exit.
  );

  Future<void> futConnect = socket->connect(Url{this->scheme() + "://10.11.12.13:1234"});

  // Failed connection.
  ASSERT_TRUE(test::finishesWithError(futConnect));

  // Expected error.
  const auto errorMsg = ErrorCode<N>{ErrorCode<N>::socketCreationFailed}.message();
  ASSERT_TRUE(futConnect.error().find(errorMsg) != std::string::npos);
}

// Failure to apply client configuration results in a failed connection.
TYPED_TEST(NetMessageSocket, ConnectFailsOnSSLApplyClientConfigFailure)
{
  using namespace qi;
  using namespace qi::sock;
  using N = mock::Network;

  auto socket = this->makeClientSocket();

  // Next apply config will fail.
  auto _ = ka::scoped_set_and_restore(
    N::_applyClientConfig,
    [](N::ssl_context_type&, const qi::ssl::ClientConfig&, const std::string&) {
      throw std::runtime_error("failed to apply client SSL configuration");
    }
  );

  auto connection = connect(socket, Url{ this->scheme() + "://10.11.12.13:1234" });

  // Failed connection.
  ASSERT_TRUE(test::finishesWithError(connection.complete));

  // Expected error.
  const auto errorMsg = ErrorCode<N>{ ErrorCode<N>::socketCreationFailed }.message();
  EXPECT_THAT(connection.complete.error(), testing::HasSubstr(errorMsg));
}

// Note: we cannot do the same test as above for server sockets because at the moment, it is the
// `TransportServer` type that configures the SSL context of sockets. Unfortunately, this type is
// not parametrizable with the `Network` type. Therefore we cannot use the mock `Network` with it.

// Failure to set the verify mode results in a failed connection.
TYPED_TEST(NetMessageSocket, ConnectFailsOnVerifyModeSetFailure)
{
  using namespace qi;
  using namespace qi::sock;
  using N = mock::Network;

  auto socket = this->makeClientSocket();

  // Next verify mode set will fail.
  auto _ = ka::scoped_set_and_restore(
    N::ssl_context_type::set_verify_mode,
    [](SslVerifyMode<N>) {
      throw std::runtime_error("failed to set the verify mode");
    }
  );

  auto connection = connect(socket, Url{ this->scheme() + "://10.11.12.13:1234" });

  // Failed connection.
  ASSERT_TRUE(test::finishesWithError(connection.complete));

  // Expected error.
  const auto errorMsg = ErrorCode<N>{ ErrorCode<N>::socketCreationFailed }.message();
  EXPECT_THAT(connection.complete.error(), testing::HasSubstr(errorMsg));
}

struct NetMessageSocketConnectUrlInvalid : testing::TestWithParam<qi::Url> {};

INSTANTIATE_TEST_SUITE_P(InvalidUrl,
                         NetMessageSocketConnectUrlInvalid,
                         testing::Values(
                           // No scheme
                           "10.11.12.13:1234",
                           // No host
                           "tcp://:1234",
                           // No port
                           "tcp://10.11.12.13"
                         ));

TEST_P(NetMessageSocketConnectUrlInvalid, InvalidNoScheme)
{
  using namespace qi;
  using namespace qi::sock;
  using N = mock::Network;

  auto socket = makeClientSocket<N>();

  auto connection = connect(socket, GetParam());

  // Failed connection.
  ASSERT_TRUE(test::finishesWithError(connection.complete));

  // Expected error.
  EXPECT_THAT(connection.complete.error(), testing::HasSubstr("invalid URL"));
}

TEST(NetMessageSocketConnectUrl, NotTcpScheme)
{
  using namespace qi;
  using namespace qi::sock;
  using N = mock::Network;

  auto socket = makeClientSocket<N>();

  auto connection = connect(socket, "qi://10.11.12.13:1234");

  // Failed connection.
  ASSERT_TRUE(test::finishesWithError(connection.complete));

  // Expected error.
  EXPECT_THAT(connection.complete.error(), testing::HasSubstr("unsupported URL TCP scheme"));
}

// Stress test for disconnections.
// A lot of threads try to disconnect the same socket at the same time.
// This test should be launched a lot of times to be meaningful.
TYPED_TEST(NetMessageSocketAsio, DisconnectBurst)
{
  using namespace qi;
  using namespace qi::sock;

  // Start a server.
  TransportServer server(test::ssl::serverConfig(test::ssl::server(), test::ssl::rootCA()));
  this->serverListen(server);
  const auto url = toUrl(server.endpoints().front());

  // Connect the client.
  auto socket =
    this->makeClientSocket(test::ssl::clientConfig(test::ssl::client(), test::ssl::rootCA()));
  auto connection = connect(socket, url);
  ASSERT_TRUE(test::finishesWithValue(connection.complete));

  // Launch threads waiting to disconnect the socket.
  Promise<void> promiseRun;
  std::array<std::thread, 100> threads;
  for (auto& t: threads)
  {
    t = std::thread{[&] {
      promiseRun.future().wait();
      socket->disconnect().wait(defaultTimeout);
    }};
  }

  // Trigger the concurrent disconnections from all threads.
  promiseRun.setValue(nullptr);
  for (auto& t: threads) t.join();
}

TYPED_TEST(NetMessageSocketAsio, SendReceiveManyMessages)
{
  using namespace qi;
  using namespace qi::sock;

  TransportServer server(test::ssl::serverConfig(test::ssl::server(), test::ssl::rootCA()));

  auto connectedSocket = this->serverListen(server);
  const auto url = toUrl(server.endpoints().front());

  // Connect the client.
  auto clientSideSocket =
    this->makeClientSocket(test::ssl::clientConfig(test::ssl::client(), test::ssl::rootCA()));

  Promise<void> promiseAllMessageReceived;
  const unsigned sendThreadCount = 100u;
  const unsigned perSendThreadMessageCount = 200u;
  const unsigned messageCount = sendThreadCount * perSendThreadMessageCount;
  MessageAddress address{1234, 5, 9876, 107};
  const auto msg0 = makeMessage(address);
  std::atomic<unsigned> i{0u};
  clientSideSocket->messageReady.connect([=, &i](const Message& msg) mutable {
    if (i > messageCount) promiseAllMessageReceived.setError("Too many messages received.");
    if (!messageEqual(msg, msg0))
    {
      promiseAllMessageReceived.setError("message not equal.");
    }
    const auto iVal = ++i;
    if (iVal == messageCount) promiseAllMessageReceived.setValue(0);
  });
  auto connection = connect(clientSideSocket, url);
  ASSERT_TRUE(test::finishesWithValue(connection.complete));

  // The server sends a message.
  ASSERT_TRUE(connectedSocket.hasValue());
  auto serverSideSocket = connectedSocket.value();
  serverSideSocket->ensureReading();

  std::mutex randMutex;
  std::random_device rd;
  std::mt19937 gen{rd()};
  std::uniform_int_distribution<> rand{1, 20};
  std::vector<std::thread> sendThreads;
  for (auto i = sendThreadCount; i != 0u; --i)
  {
    sendThreads.push_back(std::thread{
      [&] {
        for (auto j = perSendThreadMessageCount; j != 0u; --j)
        {
          serverSideSocket->send(msg0);
          std::this_thread::sleep_for(std::chrono::milliseconds{syncRand(rand, gen, randMutex)});
        }
      }
    });
  }

  // Wait for the client to receive it.
  ASSERT_TRUE(test::finishesWithValue(promiseAllMessageReceived.future(),
                                      test::willDoNothing(),
                                      MilliSeconds(FutureTimeout_Infinite)));
  for (auto& t: sendThreads) t.join();
}

TEST_F(NetMessageSocketAsioTcpRaw, DisconnectToDistantWhileConnected)
{
  using namespace qi;
  using namespace qi::sock;

  const std::string remoteServiceOwnerPath = path::findBin("remoteserviceowner");
  const Url url{this->scheme() + "://127.0.0.1:54321"};
  test::ScopedProcess _{
    remoteServiceOwnerPath, {"--qi-standalone", "--qi-listen-url=" + url.str()}
  };
  auto socket = this->makeClientSocket();
  const auto _2 = ka::scoped([=]{ socket->disconnect().wait(defaultTimeout); });
  Future<void> futCo = test::attemptConnect(*socket, url);
  ASSERT_TRUE(test::finishesWithValue(futCo));
  Future<void> futDisco = socket->disconnect().async();
  ASSERT_TRUE(test::finishesWithValue(futDisco));
}

// Connect to another process and make it brutally crash to check that the
// disconnection is quick and does not last until a system timeout expires.
TEST_F(NetMessageSocketAsioTcpRaw, DistantCrashWhileConnected)
{
  using namespace qi;
  using namespace qi::sock;

  const std::string remoteServiceOwnerPath = path::findBin("remoteserviceowner");
  const Url url{"tcp://127.0.0.1:54321"};
  MessageSocketPtr socket;
  const auto _ = ka::scoped([=]{ if (socket) socket->disconnect().wait(defaultTimeout); });
  {
    test::ScopedProcess _{
      remoteServiceOwnerPath, {"--qi-standalone", "--qi-listen-url=" + url.str()}
    };
    socket = this->makeClientSocket();
    Future<void> fut = test::attemptConnect(*socket, url);
    ASSERT_TRUE(test::finishesWithValue(fut));
  }
  std::this_thread::sleep_for(std::chrono::milliseconds{500});
  ASSERT_EQ(MessageSocket::Status::Disconnected, socket->status());
  Future<void> fut = socket->disconnect();
  ASSERT_TRUE(test::finishesWithValue(fut));
}

namespace
{
  struct ConnectClientToServer
  {
    std::unique_ptr<qi::TransportServer> transportServer;
    qi::MessageSocketPtr clientSocket;
    Connection connection;
  };

  bool uriIsLoopback(const qi::Uri& uri)
  {
    auto auth = uri.authority();
    if (auth.empty())
      return false;
    return qi::isLoopbackAddress((*auth).host());
  }

  template<typename N = qi::sock::NetworkAsio>
  ConnectClientToServer connectClientToServer(qi::ssl::ServerConfig serverCfg, qi::ssl::ClientConfig clientCfg,
                                              const std::string& host = "127.0.0.1", std::uint16_t port = 0,
                                              // default predicate looks for the first endpoint that is a loopback.
                                              std::function<bool(const qi::Uri&)> endpointPredicate = uriIsLoopback)
  {
    // Start a server and get the server side socket.
    std::unique_ptr<qi::TransportServer> server(new qi::TransportServer(serverCfg));
    auto connectedSocket = serverListen<qi::TcpScheme::MutualTls>(*server, host, port);

    // Connect the client.
    auto clientSocket = makeClientSocket<N>(clientCfg);

    const auto endpoints = server->endpoints();
    const auto epEnd = endpoints.end();
    auto epIt = std::find_if(endpoints.begin(), epEnd, std::move(endpointPredicate));
    if (epIt == epEnd)
      throw std::runtime_error("could not find an acceptable server endpoint");

    auto connection = connect(clientSocket, qi::toUrl(*epIt));

    EXPECT_TRUE(test::finishesWithValue(connectedSocket));
    auto serverSideSocket = connectedSocket.value();

    // No connection error will be propagated unless we call `ensureReading`.
    serverSideSocket->ensureReading();

    return { std::move(server), clientSocket, std::move(connection) };
  }

#define ASSERT_HAS_SSL_ERROR(fut, lib, func, reason) \
  ASSERT_TRUE(test::finishesWithError(fut)); \
  const auto err = ERR_PACK(lib, func, reason); \
  const auto errorMsg = qi::ssl::errorCategory().message(err); \
  EXPECT_THAT(fut.error(), testing::HasSubstr(errorMsg)); \

} // anonymous namespace

TEST_F(NetMessageSocketAsioTcpMutualTls,
       CertificateVerificationClientMissingCertFailure)
{
  const auto connect = connectClientToServer(
      test::ssl::serverConfig(test::ssl::server(), test::ssl::rootCA()),
      test::ssl::clientConfig(test::ssl::client()));
  ASSERT_HAS_SSL_ERROR(connect.connection.complete,
                       ERR_LIB_SSL,
                       SSL_F_TLS_PROCESS_SERVER_CERTIFICATE,
                       SSL_R_CERTIFICATE_VERIFY_FAILED);
}

TEST_F(NetMessageSocketAsioTcpMutualTls,
       CertificateVerificationServerMissingCertFailure)
{
  const auto connect = connectClientToServer(
      test::ssl::serverConfig(test::ssl::server()),
      test::ssl::clientConfig(test::ssl::client(), test::ssl::rootCA()));
  // Expected: the server returns an "unknown CA" error.
  ASSERT_HAS_SSL_ERROR(connect.connection.complete,
                       ERR_LIB_SSL,
                       SSL_F_SSL3_READ_BYTES,
                       SSL_R_TLSV1_ALERT_UNKNOWN_CA);
}

TEST_F(NetMessageSocketAsioTcpMutualTls,
       CertificateVerificationClientWrongCertFailure)
{
  const auto connect = connectClientToServer(
      test::ssl::serverConfig(test::ssl::a::server(), test::ssl::a::rootCA()),
      test::ssl::clientConfig(test::ssl::a::client(), test::ssl::b::rootCA()));
  ASSERT_HAS_SSL_ERROR(connect.connection.complete,
                       ERR_LIB_SSL,
                       SSL_F_TLS_PROCESS_SERVER_CERTIFICATE,
                       SSL_R_CERTIFICATE_VERIFY_FAILED);
}

TEST_F(NetMessageSocketAsioTcpMutualTls,
       CertificateVerificationServerWrongCertFailure)
{
  const auto connect = connectClientToServer(
      test::ssl::serverConfig(test::ssl::a::server(), test::ssl::b::rootCA()),
      test::ssl::clientConfig(test::ssl::a::client(), test::ssl::a::rootCA()));
  // Expected: the server returns an "unknown CA" error.
  ASSERT_HAS_SSL_ERROR(connect.connection.complete,
                       ERR_LIB_SSL,
                       SSL_F_SSL3_READ_BYTES,
                       SSL_R_TLSV1_ALERT_UNKNOWN_CA);
}

TEST_F(NetMessageSocketAsioTcpMutualTls,
       CertificateVerificationFromRootCASuccess)
{
  const auto connect = connectClientToServer(
      test::ssl::serverConfig(test::ssl::a::one::server(), test::ssl::a::rootCA()),
      test::ssl::clientConfig(test::ssl::a::two::client(), test::ssl::a::rootCA()));
  EXPECT_TRUE(test::finishesWithValue(connect.connection.complete));
}

TEST_F(NetMessageSocketAsioTcpMutualTls,
       CertificateVerificationPartialChainFromRootCASuccess)
{
  const auto connect = connectClientToServer(
      []{
        auto cfg = test::ssl::serverConfig(test::ssl::server(), test::ssl::rootCA());
        cfg.verifyPartialChain = true;
        return cfg;
      }(),
      []{
        auto cfg = test::ssl::clientConfig(test::ssl::client(), test::ssl::rootCA());
        cfg.verifyPartialChain = true;
        return cfg;
      }());
  EXPECT_TRUE(test::finishesWithValue(connect.connection.complete));
}

TEST_F(NetMessageSocketAsioTcpMutualTls,
       CertificateVerificationServerPartialChainFromIntermediateCAFailure)
{
  const auto connect = connectClientToServer(
      []{
        auto cfg = test::ssl::serverConfig(test::ssl::server(), test::ssl::ca());
        cfg.verifyPartialChain = true;
        return cfg;
      }(),
      test::ssl::clientConfig(test::ssl::client(), test::ssl::ca()));
  ASSERT_HAS_SSL_ERROR(connect.connection.complete,
                       ERR_LIB_SSL,
                       SSL_F_TLS_PROCESS_SERVER_CERTIFICATE,
                       SSL_R_CERTIFICATE_VERIFY_FAILED);
}

TEST_F(NetMessageSocketAsioTcpMutualTls,
       CertificateVerificationClientPartialChainFromIntermediateCAFailure)
{
  const auto connect = connectClientToServer(
      test::ssl::serverConfig(test::ssl::server(), test::ssl::ca()),
      []{
        auto cfg = test::ssl::clientConfig(test::ssl::client(), test::ssl::ca());
        cfg.verifyPartialChain = true;
        return cfg;
      }());
  ASSERT_HAS_SSL_ERROR(connect.connection.complete,
                       ERR_LIB_SSL,
                       SSL_F_SSL3_READ_BYTES,
                       SSL_R_TLSV1_ALERT_UNKNOWN_CA);
}

TEST_F(NetMessageSocketAsioTcpMutualTls,
       CertificateVerificationFromIntermediateCASuccess)
{
  const auto connect = connectClientToServer(
      []{
        auto cfg = test::ssl::serverConfig(test::ssl::server(), test::ssl::ca());
        cfg.verifyPartialChain = true;
        return cfg;
      }(),
      []{
        auto cfg = test::ssl::clientConfig(test::ssl::client(), test::ssl::ca());
        cfg.verifyPartialChain = true;
        return cfg;
      }());
  EXPECT_TRUE(test::finishesWithValue(connect.connection.complete));
}


TEST_F(NetMessageSocketAsioTcpMutualTls,
       CertificateVerificationServerPartialChainFromEndEntityFailure)
{
  const auto connect = connectClientToServer(
      []{
        auto cfg = test::ssl::serverConfig(test::ssl::server(), test::ssl::client());
        cfg.verifyPartialChain = true;
        return cfg;
      }(),
      test::ssl::clientConfig(test::ssl::client(), test::ssl::server()));
  ASSERT_HAS_SSL_ERROR(connect.connection.complete,
                       ERR_LIB_SSL,
                       SSL_F_TLS_PROCESS_SERVER_CERTIFICATE,
                       SSL_R_CERTIFICATE_VERIFY_FAILED);
}

TEST_F(NetMessageSocketAsioTcpMutualTls,
       CertificateVerificationClientPartialChainFromEndEntityFailure)
{
  const auto connect = connectClientToServer(
      test::ssl::serverConfig(test::ssl::server(), test::ssl::client()),
      []{
        auto cfg = test::ssl::clientConfig(test::ssl::client(), test::ssl::server());
        cfg.verifyPartialChain = true;
        return cfg;
      }());
  ASSERT_HAS_SSL_ERROR(connect.connection.complete,
                       ERR_LIB_SSL,
                       SSL_F_SSL3_READ_BYTES,
                       SSL_R_TLSV1_ALERT_UNKNOWN_CA);
}

TEST_F(NetMessageSocketAsioTcpMutualTls,
       CertificateVerificationPartialChainFromEndEntitySuccess)
{
  const auto connect = connectClientToServer(
      []{
        auto cfg = test::ssl::serverConfig(test::ssl::server(), test::ssl::client());
        cfg.verifyPartialChain = true;
        return cfg;
      }(),
      []{
        auto cfg = test::ssl::clientConfig(test::ssl::client(), test::ssl::server());
        cfg.verifyPartialChain = true;
        return cfg;
      }());
  EXPECT_TRUE(test::finishesWithValue(connect.connection.complete));
}

// Checks that the certificate contains only loopback DNS/IP names in its 'Subject Alternative Names
// (SAN)' extension, and at least one of them.
static void assertAllAltNamesLocal(const qi::ssl::Certificate& cert)
{
  qiLogCategory("test.tcpmessagesocket.assertAllAltNamesLocal");
  const auto altNames = static_cast<GENERAL_NAMES*>(
    ::X509_get_ext_d2i(cert.get(), NID_subject_alt_name, nullptr, nullptr));
  ASSERT_TRUE(altNames);
  const auto altNamesCount = sk_GENERAL_NAME_num(altNames);

  bool hasAtLeastIpOrDns = false;
  for (int i = 0; i < altNamesCount; ++i)
  {
    const auto altName = sk_GENERAL_NAME_value(altNames, i);
    if (altName->type == GEN_DNS)
    {
      hasAtLeastIpOrDns = true;
      auto rawName = ka::scoped([&altName]{
          unsigned char* buf = nullptr;
          ASN1_STRING_to_UTF8(&buf, altName->d.dNSName);
          return buf;
        }(),
        [](unsigned char* buf) {
          OPENSSL_free(buf);
        }
      );
      std::string name = reinterpret_cast<const char*>(rawName.value);
      ASSERT_TRUE(qi::isLoopbackAddress(name)) << name;
    }
    else if (altName->type == GEN_IPADD)
    {
      hasAtLeastIpOrDns = true;
      const auto ipAddr = altName->d.iPAddress;
      // IPv4
      if (ipAddr->length == 4)
      {
        boost::asio::ip::address_v4::bytes_type bytes;
        std::copy_n(ipAddr->data, ipAddr->length, bytes.begin());
        const auto addr = boost::asio::ip::make_address_v4(bytes);
        ASSERT_TRUE(addr.is_loopback()) << addr;
      }
      // IPv6
      else if (ipAddr->length == 16)
      {
        boost::asio::ip::address_v6::bytes_type bytes;
        std::copy_n(ipAddr->data, ipAddr->length, bytes.begin());
        const auto addr = boost::asio::ip::make_address_v6(bytes);
        ASSERT_TRUE(addr.is_loopback()) << addr;
      }
    }
  }
  ASSERT_TRUE(hasAtLeastIpOrDns);
}

TEST_F(NetMessageSocketAsioTcpMutualTls,
       CertificateVerificationBadServerHostnameFailure)
{
  auto server = test::ssl::server();
  assertAllAltNamesLocal(test::ssl::endEntity(server));

  const auto connect = connectClientToServer(
      test::ssl::serverConfig(test::ssl::server(), test::ssl::rootCA()),
      test::ssl::clientConfig(test::ssl::client(), test::ssl::rootCA()),
      // Listen to all interfaces, any port.
      "0.0.0.0", 0,
      // Predicate to force using an endpoint that is not a loopback address.
      [](const qi::Uri& uri) {
        const auto auth = uri.authority();
        if (auth.empty())
          return false;
        return !qi::isLoopbackAddress((*auth).host());
      });
  ASSERT_HAS_SSL_ERROR(connect.connection.complete,
                       ERR_LIB_SSL,
                       SSL_F_TLS_PROCESS_SERVER_CERTIFICATE,
                       SSL_R_CERTIFICATE_VERIFY_FAILED);
}

namespace test
{
namespace
{
  struct ForcedSuccessVerifyCallback
  {
    bool firstCall = true;
    template<typename... Args>
    bool operator()(bool ok, Args&&...)
    {
      if (firstCall)
      {
        EXPECT_FALSE(ok);
        firstCall = false;
      }
      return true;
    }
  };
} // anonymous namespace
} // namespace test

TEST_F(NetMessageSocketAsioTcpMutualTls,
       CertificateVerificationClientVerifyCallbackForcedFailure)
{
  const auto connect = connectClientToServer(
      test::ssl::serverConfig(test::ssl::server(), test::ssl::rootCA()),
      []{
        auto cfg = test::ssl::clientConfig(test::ssl::client(), test::ssl::rootCA());
        cfg.verifyCallback = [](bool ok, ::X509_STORE_CTX&, const std::string&) {
          EXPECT_TRUE(ok);
          return false;
        };
        return cfg;
      }());
  ASSERT_HAS_SSL_ERROR(connect.connection.complete,
                       ERR_LIB_SSL,
                       SSL_F_TLS_PROCESS_SERVER_CERTIFICATE,
                       SSL_R_CERTIFICATE_VERIFY_FAILED);
}

TEST_F(NetMessageSocketAsioTcpMutualTls,
       CertificateVerificationClientVerifyCallbackForcedSuccess)
{
  const auto connect = connectClientToServer(
      test::ssl::serverConfig(test::ssl::a::server(), test::ssl::rootCA()),
      []{
        auto cfg = test::ssl::clientConfig(test::ssl::client(), test::ssl::b::rootCA());
        cfg.verifyCallback = test::ForcedSuccessVerifyCallback{};
        return cfg;
      }());
  EXPECT_TRUE(test::finishesWithValue(connect.connection.complete));
}

TEST_F(NetMessageSocketAsioTcpMutualTls,
       CertificateVerificationServerVerifyCallbackForcedFailure)
{
  const auto connect = connectClientToServer(
      []{
        auto cfg = test::ssl::serverConfig(test::ssl::server(), test::ssl::rootCA());
        cfg.verifyCallback = [](bool ok, ::X509_STORE_CTX& ctx, boost::asio::ip::tcp::endpoint) {
          ::X509_STORE_CTX_set_error(&ctx, X509_V_ERR_APPLICATION_VERIFICATION);
          EXPECT_TRUE(ok);
          return false;
        };
        return cfg;
      }(),
      test::ssl::clientConfig(test::ssl::client(), test::ssl::rootCA()));
  ASSERT_HAS_SSL_ERROR(connect.connection.complete,
                       ERR_LIB_SSL,
                       SSL_F_SSL3_READ_BYTES,
                       SSL_R_SSLV3_ALERT_HANDSHAKE_FAILURE);
}

TEST_F(NetMessageSocketAsioTcpMutualTls,
       CertificateVerificationServerVerifyCallbackForcedSuccess)
{
  const auto connect = connectClientToServer(
      []{
        auto cfg = test::ssl::serverConfig(test::ssl::server(), test::ssl::b::rootCA());
        cfg.verifyCallback = test::ForcedSuccessVerifyCallback{};
        return cfg;
      }(),
        test::ssl::clientConfig(test::ssl::client(), test::ssl::rootCA()));
  EXPECT_TRUE(test::finishesWithValue(connect.connection.complete));
}

TEST_F(NetMessageSocketAsioTcpMutualTls,
       CertificateVerificationClientVerifyCallbackThrowFailure)
{
  const auto connect = connectClientToServer(
      test::ssl::serverConfig(test::ssl::server(), test::ssl::rootCA()),
      []{
        auto cfg = test::ssl::clientConfig(test::ssl::client(), test::ssl::rootCA());
        cfg.verifyCallback = [](bool /*ok*/, ::X509_STORE_CTX&, const std::string&) -> bool {
          throw 42;
        };
        return cfg;
      }());
  ASSERT_HAS_SSL_ERROR(connect.connection.complete,
                       ERR_LIB_SSL,
                       SSL_F_TLS_PROCESS_SERVER_CERTIFICATE,
                       SSL_R_CERTIFICATE_VERIFY_FAILED);
}

TEST_F(NetMessageSocketAsioTcpMutualTls,
       CertificateVerificationServerVerifyCallbackThrowFailure)
{
  const auto connect = connectClientToServer(
      []{
        auto cfg = test::ssl::serverConfig(test::ssl::server(), test::ssl::rootCA());
        cfg.verifyCallback = [](bool /*ok*/, ::X509_STORE_CTX&, boost::asio::ip::tcp::endpoint) -> bool {
          throw 42;
        };
        return cfg;
      }(),
      test::ssl::clientConfig(test::ssl::client(), test::ssl::rootCA()));
  ASSERT_HAS_SSL_ERROR(connect.connection.complete,
                       ERR_LIB_SSL,
                       SSL_F_SSL3_READ_BYTES,
                       SSL_R_TLSV1_ALERT_INTERNAL_ERROR);
}
