#include <chrono>
#include <numeric>
#include <random>
#include <gtest/gtest.h>
#include "sock/networkmock.hpp"
#include "sock/networkcommon.hpp"
#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <qi/messaging/sock/accept.hpp>
#include "src/messaging/tcpmessagesocket.hpp"
#include "src/messaging/transportserver.hpp"
#include "tests/qi/testutils/testutils.hpp"
#include "qi/scoped.hpp"

static const qi::MilliSeconds defaultTimeout{ 2000 };

////////////////////////////////////////////////////////////////////////////////
// Connected tests
////////////////////////////////////////////////////////////////////////////////

namespace mock
{
  template<typename H> // NetHandler H
  void readHeader(N::_mutable_buffer_sequence buf, H h, qi::uint32_t magic = qi::MessagePrivate::magic, qi::uint32_t size = 10u, Error error = {})
  {
    qi::MessagePrivate::MessageHeader header;
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
    qi::uint32_t _magic = qi::MessagePrivate::magic;
    qi::uint32_t _headerSize = 10u;
    Error _headerError = {};
    Error _dataError = {};
    int _callCount = 0;
    void operator()(Socket::next_layer_type&, N::_mutable_buffer_sequence buf, N::_anyTransferHandler h)
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

template<typename Socket>
SignalPromises connectSignals(Socket& socket)
{
  SignalPromises p;
  socket.connected.connect([=]() mutable {
    p._connectedReceived.setValue(0);
  });
  socket.disconnected.connect([=](const std::string& msg) mutable {
    p._disconnectedReceived.setValue(msg);
  });
  return p;
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
      static const qi::Url listenUrl = scheme() + "://127.0.0.1:34988";
      return listenUrl;
    }

    static qi::Promise<qi::MessageSocketPtr> listen(qi::TransportServer& server
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

      return promiseConnectedSocket;
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

  SignalPromises signalPromises;
  {
    auto clientSideSocket = makeMessageSocket(this->scheme());
    signalPromises = connectSignals(*clientSideSocket);

    // Check that signal were not emitted.
    ASSERT_TRUE(signalPromises.connectedReceived().isRunning());
    ASSERT_TRUE(signalPromises.disconnectedReceived().isRunning());
  }

  // Socket is destroyed : disconnect must not be emitted since it was not
  // connected in the first place.
  ASSERT_TRUE(signalPromises.disconnectedReceived().isRunning());
}

TYPED_TEST(NetMessageSocket, ConnectAndDisconnectAsio)
{
  using namespace qi;
  using namespace qi::sock;

  // Start a server and get the server side socket.
  TransportServer server;
  const auto url = this->defaultListenURL();
  this->listen(server, url);

  // We also want to check that signals are emitted.
  auto clientSideSocket = makeMessageSocket(this->scheme());
  auto signalPromises = connectSignals(*clientSideSocket);

  Future<void> fut0 = clientSideSocket->connect(url);
  ASSERT_EQ(FutureState_FinishedWithValue, fut0.wait(defaultTimeout));
  ASSERT_EQ(FutureState_FinishedWithValue, signalPromises.connectedReceived().wait(defaultTimeout));
  ASSERT_TRUE(signalPromises.disconnectedReceived().isRunning());

  Future<void> fut1 = clientSideSocket->disconnect();
  ASSERT_EQ(FutureState_FinishedWithValue, fut1.wait(defaultTimeout));
  ASSERT_EQ(FutureState_FinishedWithValue, signalPromises.disconnectedReceived().wait(defaultTimeout));
}

TYPED_TEST(NetMessageSocket, ConnectAndDestroyAsio)
{
  using namespace qi;
  using namespace qi::sock;

  // We also want to check that signals are emitted.
  SignalPromises signalPromises;

  {
    // Start a server and get the server side socket.
    TransportServer server;
    const auto url = this->defaultListenURL();
    this->listen(server, url);
    auto clientSideSocket = makeMessageSocket(this->scheme());
    signalPromises = connectSignals(*clientSideSocket);

    Future<void> fut0 = clientSideSocket->connect(url);
    ASSERT_EQ(FutureState_FinishedWithValue, fut0.wait(defaultTimeout));
    ASSERT_EQ(FutureState_FinishedWithValue, signalPromises.connectedReceived().wait(defaultTimeout));
    ASSERT_TRUE(signalPromises.disconnectedReceived().isRunning());
  }

  // disconnected must be emitted on destruction.
  ASSERT_EQ(FutureState_FinishedWithValue, signalPromises.disconnectedReceived().wait(defaultTimeout));
}

TYPED_TEST(NetMessageSocket, SendWhileNotConnectedAsio)
{
  using namespace qi;
  using namespace qi::sock;

  auto clientSideSocket = makeMessageSocket(this->scheme());
  auto signalPromises = connectSignals(*clientSideSocket);

  MessageAddress address{1234, 5, 9876, 107};
  ASSERT_FALSE(clientSideSocket->send(makeMessage(address)));

  // Check that signal were not emitted.
  ASSERT_TRUE(signalPromises.connectedReceived().isRunning());
  ASSERT_TRUE(signalPromises.disconnectedReceived().isRunning());
}

TYPED_TEST(NetMessageSocket, SendAfterDisconnectedAsio)
{
  using namespace qi;
  using namespace qi::sock;

  // Start a server and get the server side socket.
  TransportServer server;
  const auto url = this->defaultListenURL();
  this->listen(server, url);

  auto clientSideSocket = makeMessageSocket(this->scheme());
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
  ASSERT_FALSE(socket->disconnect().hasError());
}

TYPED_TEST(NetMessageSocket, ReceiveOneMessageAsio)
{
  using namespace qi;
  using namespace qi::sock;

  // Start a server and get the server side socket.
  TransportServer server;
  const auto url = this->defaultListenURL();
  auto promiseServerSideSocket = this->listen(server, url);

  // Connect the client.
  auto msgSent = makeMessage(MessageAddress{1234, 5, 9876, 107});

  Promise<void> promiseReceivedMessage;
  auto clientSideSocket = makeMessageSocket(this->scheme());//N::defaultIoService(), SslEnabled{true});
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
  const auto url = this->defaultListenURL();
  auto promiseServerSideSocket = this->listen(server, url);

  // Connect the client.
  Promise<void> promiseAllMessageReceived;
  auto clientSideSocket = makeMessageSocket(this->scheme());
  const int messageCount = 100;
  MessageAddress address{1234, 5, 9876, 107};
  int i = 0;
  clientSideSocket->messageReady.connect([=, &i](const Message& msg) mutable {
    if (i > messageCount) promiseAllMessageReceived.setError("Too many messages received.");
    auto addr = address;
    addr.messageId = address.messageId + i;
    auto msg2 = makeMessage(addr);
    if (!messageEqual(msg, msg2))
    {
      promiseAllMessageReceived.setError("message not equal.");
    }
    ++i;
    if (i == messageCount) promiseAllMessageReceived.setValue(0);
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
  const auto url = this->defaultListenURL();
  this->listen(server, url);

  // Connect the client.
  auto msgSent = makeMessage(MessageAddress{1234, 5, 9876, 107});

  auto clientSideSocket = makeMessageSocket(this->scheme());
  clientSideSocket->connect(url);
}

TYPED_TEST(NetMessageSocket, DisconnectWhileConnecting)
{
  using namespace qi;
  using namespace qi::sock;
  using mock::Resolver;
  using namespace mock;

  std::thread threadResolve;
  auto scopedResolve = scopedSetAndRestore(
    Resolver::async_resolve,
    [&](Resolver::query q, Resolver::_anyResolveHandler h) {
        threadResolve = std::thread{[=] {
          defaultAsyncResolve(q, h);
        }};
    }
  );
  auto r = scoped([&] {
    threadResolve.join();
  });

  Promise<void> promiseAsyncConnectStarted;
  Promise<Error> promiseAsyncConnectResult;

  // The async connect waits for a promise to be set before finishing.
  std::thread threadConnect;
  auto scopedConnect = scopedSetAndRestore(
    LowestLayer::async_connect,
    [&](N::_resolver_entry, N::_anyHandler h) mutable {
      threadConnect = std::thread{[=]() mutable {
        promiseAsyncConnectStarted.setValue(0);
        auto err = promiseAsyncConnectResult.future().value();
        h(err);
      }};
    }
  );
  auto c = scoped([&] {
    threadConnect.join();
  });

  using LowestLayer = N::ssl_socket_type::lowest_layer_type;
  auto scopedShutdown = scopedSetAndRestore(
    LowestLayer::shutdown,
    [=](LowestLayer::shutdown_type, Error) mutable {
      auto err = operationAborted<Error>();
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
  using mock::Resolver;
  using namespace mock;

  Promise<Error> promiseAsyncReadWrite;

  Resolver::async_resolve = defaultAsyncResolve;
  LowestLayer::async_connect = defaultAsyncConnect;
  std::vector<std::thread> readThreads, writeThreads;

  // Make sure async read and async write block until cancel happens.
  auto scopedRead = scopedSetAndRestore(
    N::_async_read_next_layer,
    [=, &readThreads](Socket::next_layer_type&, N::_mutable_buffer_sequence, N::_anyTransferHandler h) {
      readThreads.push_back(std::thread{[=] {
        h(promiseAsyncReadWrite.future().value(), 0);
      }});
    }
  );
  auto scopedWrite = scopedSetAndRestore(
    N::_async_write_next_layer,
    [=, &writeThreads](Socket::next_layer_type&, const std::vector<N::_const_buffer_sequence>&, N::_anyTransferHandler h) {
      writeThreads.push_back(std::thread{[=] {
        h(promiseAsyncReadWrite.future().value(), 0);
      }});
    }
  );
  auto scopedCancel = scopedSetAndRestore(
    N::ssl_socket_type::lowest_layer_type::cancel,
    [=]() mutable {
      promiseAsyncReadWrite.setValue(operationAborted<Error>());
    }
  );

  Promise<void> promiseShutdownStarted;
  Promise<void> promiseCanShutdown;
  auto scopedShutdown = scopedSetAndRestore(
    LowestLayer::shutdown,
    [=](LowestLayer::shutdown_type, Error) mutable {
      promiseShutdownStarted.setValue(0);
      promiseCanShutdown.future().wait();
    }
  );

  auto socket = boost::make_shared<TcpMessageSocket<N>>();
  socket->connect(Url{"tcp://10.11.12.13:1234"});

  Future<void> futDisconnect0 = socket->disconnect();
  ASSERT_TRUE(futDisconnect0.isRunning());

  // Wait for entering shutdown().
  ASSERT_EQ(FutureState_FinishedWithValue, promiseShutdownStarted.future().wait(defaultTimeout));

  // Subsequent disconnect() cannot complete because shutdown() is blocking.
  Future<void> futDisconnect1 = socket->disconnect();
  ASSERT_TRUE(futDisconnect1.isRunning()) << "finished: " << futDisconnect1.isFinished();
  Future<void> futDisconnect2 = socket->disconnect();
  ASSERT_TRUE(futDisconnect2.isRunning());

  // Now, unblock shutdown() so that disconnecting state and later
  // disconnected state can complete.
  promiseCanShutdown.setValue(0);

  // All future must complete.
  ASSERT_EQ(FutureState_FinishedWithValue, futDisconnect0.wait(defaultTimeout));
  ASSERT_EQ(FutureState_FinishedWithValue, futDisconnect1.wait(defaultTimeout));
  ASSERT_EQ(FutureState_FinishedWithValue, futDisconnect2.wait(defaultTimeout));
  ASSERT_FALSE(socket->isConnected());
  ASSERT_EQ(MessageSocket::Status::Disconnected, socket->status());

  for (auto& t: readThreads) t.join();
  for (auto& t: writeThreads) t.join();
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
  const auto url = this->defaultListenURL();
  this->listen(server, url);

  // Connect the client.
  auto socket = makeMessageSocket(this->scheme());
  Future<void> fut = socket->connect(url);
  ASSERT_EQ(FutureState_FinishedWithValue, fut.wait(defaultTimeout));

  // Launch threads waiting to disconnect the socket.
  Promise<void> promiseRun;
  std::array<std::thread, 100> threads;
  for (auto& t: threads)
  {
    t = std::thread{[&] {
      promiseRun.future().wait();
      socket->disconnect();
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
  const auto url = this->defaultListenURL();
  auto promiseServerSideSocket = this->listen(server, url);

  // Connect the client.
  Promise<void> promiseAllMessageReceived;
  auto clientSideSocket = makeMessageSocket(this->scheme());
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
    ++i;
    if (i == messageCount) promiseAllMessageReceived.setValue(0);
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
  using std::chrono::milliseconds;

  const std::string remoteServiceOwnerPath = path::findBin("remoteserviceowner");
  const std::string scheme{"tcp"};
  const Url url{scheme + "://127.0.0.1:54321"};
  MessageSocketPtr socket;
  ScopedProcess _{
    remoteServiceOwnerPath, {"--qi-standalone", "--qi-listen-url=" + url.str()}
  };
  std::this_thread::sleep_for(milliseconds{100});
  socket = makeMessageSocket(scheme, getEventLoop());
  Future<void> futCo = socket->connect(url);
  ASSERT_EQ(FutureState_FinishedWithValue, futCo.wait(defaultTimeout));
  Future<void> futDisco = socket->disconnect();
  ASSERT_EQ(FutureState_FinishedWithValue, futDisco.wait(defaultTimeout));
}

// Connect to another process and make it brutally crash to check that the
// disconnection is quick and does not last until a system timeout expires.
TEST(NetMessageSocketAsio, DistantCrashWhileConnected)
{
  using namespace qi;
  using namespace qi::sock;
  using std::chrono::milliseconds;

  const std::string remoteServiceOwnerPath = path::findBin("remoteserviceowner");
  const std::string protocol{"tcp"};
  const Url url{protocol + "://127.0.0.1:54321"};
  MessageSocketPtr socket;
  {
    ScopedProcess _{
      remoteServiceOwnerPath, {"--qi-standalone", "--qi-listen-url=" + url.str()}
    };
    std::this_thread::sleep_for(milliseconds{100});
    socket = makeMessageSocket(protocol, getEventLoop());
    Future<void> fut = socket->connect(url);
    ASSERT_EQ(FutureState_FinishedWithValue, fut.wait(defaultTimeout));
  }
  std::this_thread::sleep_for(milliseconds{500});
  ASSERT_EQ(MessageSocket::Status::Disconnected, socket->status());
  Future<void> fut = socket->disconnect();
  ASSERT_EQ(FutureState_FinishedWithValue, fut.wait(defaultTimeout));
}
