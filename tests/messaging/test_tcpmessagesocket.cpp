#include <chrono>
#include <functional>
#include <numeric>
#include <random>
#include <gtest/gtest.h>
#include "sock/networkmock.hpp"
#include "sock/networkcommon.hpp"
#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <src/messaging/sock/accept.hpp>
#include "src/messaging/tcpmessagesocket.hpp"
#include "src/messaging/transportserver.hpp"
#include "tests/qi/testutils/testutils.hpp"
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

  struct SchemeTcp
  {
  // Function<std::string ()>:
    std::string operator()() const { return "tcp"; }
  };

  struct SchemeTcpSSL
  {
  // Function<std::string ()>:
    std::string operator()() const { return "tcps"; }
  };


  template<class SchemeType>
  class NetMessageSocket : public ::testing::Test
  {
  public:
    static SchemeType scheme;

    static qi::Url defaultListenURL()
    {
      static const qi::Url listenUrl = scheme() + "://127.0.0.1:0";
      return listenUrl;
    }

    struct ListenResult
    {
      qi::Promise<qi::MessageSocketPtr> promiseConnectedSocket;
      qi::Url url;
    };

    static ListenResult listen(qi::TransportServer& server
      , const qi::Url& url = defaultListenURL()
      , bool connectNewConnectionSignal = true)
    {
      using namespace qi;
      const auto key = qi::path::findData("qi", "server.key");
      const auto crt = qi::path::findData("qi", "server.crt");
      server.setIdentity(key, crt);
      Promise<MessageSocketPtr> promiseConnectedSocket;
      if (connectNewConnectionSignal)
      {
        server.newConnection.connect([=](const std::pair<MessageSocketPtr, Url>& p) mutable {
          promiseConnectedSocket.setValue(p.first);
        });
      }
      else
      {
        promiseConnectedSocket.setValue({});
      }
      server.listen(url);

      return { promiseConnectedSocket, server.endpoints().front() };
    }

  };

  template<class SchemeType>
  SchemeType NetMessageSocket<SchemeType>::scheme;


  template<class SchemeType>
  class NetMessageSocketAsio : public NetMessageSocket<SchemeType> {};

  using SchemeTypes = ::testing::Types< SchemeTcp
                                        , SchemeTcpSSL
                                        >;

  TYPED_TEST_CASE(NetMessageSocket, SchemeTypes);
  TYPED_TEST_CASE(NetMessageSocketAsio, SchemeTypes);

}

TYPED_TEST(NetMessageSocket, DestroyNotConnectedAsio)
{
  using namespace qi;
  using namespace qi::sock;

  SignalConnectionPtr signalConnection;
  {
    auto clientSideSocket = makeMessageSocket(this->scheme());
    const auto _ = ka::scoped([=]{ clientSideSocket->disconnect().wait(defaultTimeout); });
    signalConnection = connectSignals(clientSideSocket);

    // Check that signal were not emitted.
    ASSERT_TRUE(signalConnection->promises.connectedReceived().isRunning());
    ASSERT_TRUE(signalConnection->promises.disconnectedReceived().isRunning());
  }

  // Socket is destroyed : disconnect must not be emitted since it was not
  // connected in the first place.
  ASSERT_TRUE(signalConnection->promises.disconnectedReceived().isRunning());
}

TYPED_TEST(NetMessageSocket, ConnectAndDisconnectAsio)
{
  using namespace qi;
  using namespace qi::sock;

  // Start a server and get the server side socket.
  TransportServer server;
  const auto url = this->listen(server).url;

  // We also want to check that signals are emitted.
  auto clientSideSocket = makeMessageSocket(this->scheme());
  const auto _ = ka::scoped([=]{ clientSideSocket->disconnect().wait(defaultTimeout); });
  auto signalConnection = connectSignals(clientSideSocket);

  Future<void> fut0 = clientSideSocket->connect(url);
  ASSERT_EQ(FutureState_FinishedWithValue, fut0.wait(defaultTimeout));
  ASSERT_EQ(FutureState_FinishedWithValue,
            signalConnection->promises.connectedReceived().wait(defaultTimeout));
  ASSERT_TRUE(signalConnection->promises.disconnectedReceived().isRunning());

  Future<void> fut1 = clientSideSocket->disconnect();
  ASSERT_EQ(FutureState_FinishedWithValue, fut1.wait(defaultTimeout));
  ASSERT_EQ(FutureState_FinishedWithValue,
            signalConnection->promises.disconnectedReceived().wait(defaultTimeout));
}

TYPED_TEST(NetMessageSocket, ConnectAndDestroyAsio)
{
  using namespace qi;
  using namespace qi::sock;

  // We also want to check that signals are emitted.
  SignalConnectionPtr signalConnection;

  {
    // Start a server and get the server side socket.
    TransportServer server;
    const auto url = this->listen(server).url;
    auto clientSideSocket = makeMessageSocket(this->scheme());
    const auto _ = ka::scoped([=]{ clientSideSocket->disconnect().wait(defaultTimeout); });
    signalConnection = connectSignals(clientSideSocket);

    Future<void> fut0 = clientSideSocket->connect(url);
    ASSERT_EQ(FutureState_FinishedWithValue, fut0.wait(defaultTimeout));
    ASSERT_EQ(FutureState_FinishedWithValue,
              signalConnection->promises.connectedReceived().wait(defaultTimeout));
    ASSERT_TRUE(signalConnection->promises.disconnectedReceived().isRunning());
  }

  // disconnected must be emitted on destruction.
  ASSERT_EQ(FutureState_FinishedWithValue,
            signalConnection->promises.disconnectedReceived().wait(defaultTimeout));
}

TYPED_TEST(NetMessageSocket, SendWhileNotConnectedAsio)
{
  using namespace qi;
  using namespace qi::sock;

  auto clientSideSocket = makeMessageSocket(this->scheme());
  const auto _ = ka::scoped([=]{ clientSideSocket->disconnect().wait(defaultTimeout); });
  auto signalConnection = connectSignals(clientSideSocket);

  MessageAddress address{1234, 5, 9876, 107};
  ASSERT_FALSE(clientSideSocket->send(makeMessage(address)));

  // Check that signal were not emitted.
  ASSERT_TRUE(signalConnection->promises.connectedReceived().isRunning());
  ASSERT_TRUE(signalConnection->promises.disconnectedReceived().isRunning());
}

TYPED_TEST(NetMessageSocket, SendAfterDisconnectedAsio)
{
  using namespace qi;
  using namespace qi::sock;

  // Start a server and get the server side socket.
  TransportServer server;
  const auto url = this->listen(server).url;

  auto clientSideSocket = makeMessageSocket(this->scheme());
  const auto _ = ka::scoped([=]{ clientSideSocket->disconnect().wait(defaultTimeout); });
  Future<void> fut0 = clientSideSocket->connect(url);
  ASSERT_EQ(FutureState_FinishedWithValue, fut0.wait(defaultTimeout));

  Future<void> fut1 = clientSideSocket->disconnect();
  ASSERT_EQ(FutureState_FinishedWithValue, fut1.wait(defaultTimeout));

  MessageAddress address{1234, 5, 9876, 107};
  ASSERT_FALSE(clientSideSocket->send(makeMessage(address)));
}

TYPED_TEST(NetMessageSocket, DisconnectWhileNotConnectedAsio)
{
  using namespace qi;
  using namespace qi::sock;

  auto socket = makeMessageSocket(this->scheme());
  const auto _ = ka::scoped([=]{ socket->disconnect().wait(defaultTimeout); });
  ASSERT_FALSE(socket->disconnect().hasError());
}

TYPED_TEST(NetMessageSocket, ReceiveOneMessageAsio)
{
  using namespace qi;
  using namespace qi::sock;

  // Start a server and get the server side socket.
  TransportServer server;
  const auto listenRes = this->listen(server);
  auto& promiseServerSideSocket = listenRes.promiseConnectedSocket;
  const auto url = listenRes.url;

  // Connect the client.
  auto msgSent = makeMessage(MessageAddress{1234, 5, 9876, 107});

  Promise<void> promiseReceivedMessage;
  auto clientSideSocket = makeMessageSocket(this->scheme());
  const auto _ = ka::scoped([=]{ clientSideSocket->disconnect().wait(defaultTimeout); });
  clientSideSocket->messageReady.connect([&](const Message& msgReceived) mutable {
    if (!messageEqual(msgReceived, msgSent)) throw std::runtime_error("messages are not equal.");
    promiseReceivedMessage.setValue(0);
  });
  Future<void> fut = clientSideSocket->connect(url);
  ASSERT_EQ(FutureState_FinishedWithValue, fut.wait(defaultTimeout));

  // The server sends a message.
  ASSERT_TRUE(promiseServerSideSocket.future().hasValue());
  ASSERT_TRUE(promiseServerSideSocket.future().value()->ensureReading());
  ASSERT_TRUE(promiseServerSideSocket.future().value()->send(msgSent));

  // Wait for the client to receive it.
  ASSERT_EQ(FutureState_FinishedWithValue, promiseReceivedMessage.future().wait(defaultTimeout));
}

TYPED_TEST(NetMessageSocketAsio, ReceiveManyMessages)
{
  using namespace qi;
  using namespace qi::sock;

  // Start a server and get the server side socket.
  TransportServer server;
  const auto listenRes = this->listen(server);
  auto& promiseServerSideSocket = listenRes.promiseConnectedSocket;
  const auto url = listenRes.url;

  // Connect the client.
  Promise<void> promiseAllMessageReceived;
  auto clientSideSocket = makeMessageSocket(this->scheme());
  const auto _ = ka::scoped([=]{ clientSideSocket->disconnect().wait(defaultTimeout); });

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
  Future<void> fut = clientSideSocket->connect(url);
  fut.wait();
  ASSERT_EQ(FutureState_FinishedWithValue, fut.wait(defaultTimeout));

  // The server sends a message.
  ASSERT_TRUE(promiseServerSideSocket.future().hasValue());
  auto serverSideSocket = promiseServerSideSocket.future().value();
  serverSideSocket->ensureReading();
  for (int i = 0; i != messageCount; ++i)
  {
    serverSideSocket->send(makeMessage(address));
    ++address.messageId;
  }

  // Wait for the client to receive it.
  ASSERT_EQ(FutureState_FinishedWithValue, promiseAllMessageReceived.future().wait());
  qiLogInfo("") << "fin\n";
}

// The test ends while a socket connection or a server accept may be pending.
// The destruction of the corresponding objects must be fine.
// This test must typically be launched a great number of times to be meaningful.
TYPED_TEST(NetMessageSocket, PrematureDestroy)
{
  using namespace qi;
  using namespace qi::sock;

  // Start a server and get the server side socket.
  TransportServer server;
  const auto url = this->listen(server).url;

  // Connect the client.
  auto msgSent = makeMessage(MessageAddress{1234, 5, 9876, 107});

  auto clientSideSocket = makeMessageSocket(this->scheme());
  const auto _ = ka::scoped([=]{ clientSideSocket->disconnect().wait(defaultTimeout); });
  clientSideSocket->connect(url);
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

  auto socket = boost::make_shared<TcpMessageSocket<N>>();
  Future<void> futConnect = socket->connect(Url{"tcp://10.11.12.13:1234"});

  // We wait for the async connect to start.
  promiseAsyncConnectStarted.future().wait();
  Future<void> futDisconnect = socket->disconnect();

  // Now the async connect (therefore the connecting part) can finish.
  ASSERT_EQ(FutureState_FinishedWithValue, futDisconnect.wait(defaultTimeout));
  ASSERT_EQ(FutureState_FinishedWithError, futConnect.wait(defaultTimeout));
  ASSERT_EQ("Connect abort: disconnection requested while connecting", futConnect.error());
}

TYPED_TEST(NetMessageSocket, DisconnectWhileDisconnecting)
{
  using namespace qi;
  using namespace qi::sock;
  using N = mock::Network;

  Promise<ErrorCode<N>> promiseAsyncReadWrite;

  Resolver<N>::async_resolve = mock::defaultAsyncResolve;
  Lowest<SslSocket<N>>::async_connect = mock::defaultAsyncConnect;
  std::vector<std::thread> readThreads, writeThreads;

  // Make sure async read and async write block until shutdown happens.
  auto scopedRead = ka::scoped_set_and_restore(
    N::_async_read_next_layer,
    [=, &readThreads](SslSocket<N>::next_layer_type&, N::_mutable_buffer_sequence, N::_anyTransferHandler h) {
      readThreads.push_back(std::thread{[=] {
        h(promiseAsyncReadWrite.future().value(), 0);
      }});
    }
  );
  auto scopedWrite = ka::scoped_set_and_restore(
    N::_async_write_next_layer,
    [=, &writeThreads](SslSocket<N>::next_layer_type&, const std::vector<N::_const_buffer_sequence>&, N::_anyTransferHandler h) {
      writeThreads.push_back(std::thread{[=] {
        h(promiseAsyncReadWrite.future().value(), 0);
      }});
    }
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

  auto socket = boost::make_shared<TcpMessageSocket<N>>();
  socket->connect(Url{"tcp://10.11.12.13:1234"});

  Future<void> futDisconnect0 = socket->disconnect();
  ASSERT_TRUE(futDisconnect0.isRunning());

  // Wait for entering close().
  ASSERT_EQ(FutureState_FinishedWithValue, promiseCloseStarted.future().wait(defaultTimeout));

  // Subsequent disconnect() cannot complete because close() is blocking.
  Future<void> futDisconnect1 = socket->disconnect();
  ASSERT_TRUE(futDisconnect1.isRunning()) << "finished: " << futDisconnect1.isFinished();
  Future<void> futDisconnect2 = socket->disconnect();
  ASSERT_TRUE(futDisconnect2.isRunning());

  // Now, unblock close() so that disconnecting state and later
  // disconnected state can complete.
  promiseCanFinishClose.setValue(0);

  // All future must complete.
  ASSERT_EQ(FutureState_FinishedWithValue, futDisconnect0.wait(defaultTimeout));
  ASSERT_EQ(FutureState_FinishedWithValue, futDisconnect1.wait(defaultTimeout));
  ASSERT_EQ(FutureState_FinishedWithValue, futDisconnect2.wait(defaultTimeout));
  ASSERT_FALSE(socket->isConnected());
  ASSERT_EQ(MessageSocket::Status::Disconnected, socket->status());

  for (auto& t: readThreads) t.join();
  for (auto& t: writeThreads) t.join();
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

  auto socket = boost::make_shared<TcpMessageSocket<N>>();

  // Next try to set the cipher list will fail.
  auto p = &N::resultOfTrySetCipherListTls12AndBelow;
  auto _ = ka::scoped_apply_and_retract(
    p,
    store_mutable(false),    // Store `false` to make fail.
    store_mutable(p->load()) // Restore original value on scope exit.
  );

  Future<void> futConnect = socket->connect(Url{"tcp://10.11.12.13:1234"});

  // Failed connection.
  ASSERT_EQ(FutureState_FinishedWithError, futConnect.wait(defaultTimeout));

  // Expected error.
  const auto errorMsg = ErrorCode<N>{ErrorCode<N>::socketCreationFailed}.message();
  ASSERT_TRUE(futConnect.error().find(errorMsg) != std::string::npos);
}

// Stress test for disconnections.
// A lot of threads try to disconnect the same socket at the same time.
// This test should be launched a lot of times to be meaningful.
TYPED_TEST(NetMessageSocketAsio, DisconnectBurst)
{
  using namespace qi;
  using namespace qi::sock;

  // Start a server.
  TransportServer server;
  const auto url = this->listen(server).url;

  // Connect the client.
  auto socket = makeMessageSocket(this->scheme());
  const auto _ = ka::scoped([=]{ socket->disconnect().wait(defaultTimeout); });
  Future<void> fut = socket->connect(url);
  ASSERT_EQ(FutureState_FinishedWithValue, fut.wait(defaultTimeout));

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

  // Start a server and get the server side socket.
  TransportServer server;
  const auto listenRes = this->listen(server);
  auto& promiseServerSideSocket = listenRes.promiseConnectedSocket;
  const auto url = listenRes.url;

  // Connect the client.
  Promise<void> promiseAllMessageReceived;
  auto clientSideSocket = makeMessageSocket(this->scheme());
  const auto _ = ka::scoped([=]{ clientSideSocket->disconnect().wait(defaultTimeout); });
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
  Future<void> fut = clientSideSocket->connect(url);
  ASSERT_EQ(FutureState_FinishedWithValue, fut.wait(defaultTimeout));

  // The server sends a message.
  ASSERT_TRUE(promiseServerSideSocket.future().hasValue());
  auto serverSideSocket = promiseServerSideSocket.future().value();
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
  ASSERT_EQ(FutureState_FinishedWithValue, promiseAllMessageReceived.future().wait());
  for (auto& t: sendThreads) t.join();
}

TEST(NetMessageSocketAsio, DisconnectToDistantWhileConnected)
{
  using namespace qi;
  using namespace qi::sock;

  const std::string remoteServiceOwnerPath = path::findBin("remoteserviceowner");
  const std::string scheme{"tcp"};
  const Url url{scheme + "://127.0.0.1:54321"};
  test::ScopedProcess _{
    remoteServiceOwnerPath, {"--qi-standalone", "--qi-listen-url=" + url.str()}
  };
  auto socket = makeMessageSocket(scheme);
  const auto _2 = ka::scoped([=]{ socket->disconnect().wait(defaultTimeout); });
  Future<void> futCo = test::attemptConnect(*socket, url);
  ASSERT_EQ(FutureState_FinishedWithValue, futCo.wait(defaultTimeout));
  Future<void> futDisco = socket->disconnect().async();
  ASSERT_EQ(FutureState_FinishedWithValue, futDisco.wait(defaultTimeout));
}

// Connect to another process and make it brutally crash to check that the
// disconnection is quick and does not last until a system timeout expires.
TEST(NetMessageSocketAsio, DistantCrashWhileConnected)
{
  using namespace qi;
  using namespace qi::sock;

  const std::string remoteServiceOwnerPath = path::findBin("remoteserviceowner");
  const std::string protocol{"tcp"};
  const Url url{protocol + "://127.0.0.1:54321"};
  MessageSocketPtr socket;
  const auto _ = ka::scoped([=]{ if (socket) socket->disconnect().wait(defaultTimeout); });
  {
    test::ScopedProcess _{
      remoteServiceOwnerPath, {"--qi-standalone", "--qi-listen-url=" + url.str()}
    };
    socket = makeMessageSocket(protocol);
    Future<void> fut = test::attemptConnect(*socket, url);
    ASSERT_EQ(FutureState_FinishedWithValue, fut.wait(defaultTimeout));
  }
  std::this_thread::sleep_for(std::chrono::milliseconds{500});
  ASSERT_EQ(MessageSocket::Status::Disconnected, socket->status());
  Future<void> fut = socket->disconnect();
  ASSERT_EQ(FutureState_FinishedWithValue, fut.wait(defaultTimeout));
}
