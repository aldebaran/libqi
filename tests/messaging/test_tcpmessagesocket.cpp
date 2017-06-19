#include <numeric>
#include <random>
#include <gtest/gtest.h>
#include "net/networkmock.hpp"
#include "net/networkcommon.hpp"
#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <qi/messaging/net/accept.hpp>
#include "src/messaging/tcpmessagesocket.hpp"
#include "src/messaging/transportserver.hpp"

static const int defaultTimeoutInMs = 500;
static const std::chrono::milliseconds defaultPostPauseInMs{20};

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
  boost::optional<qi::Message*> operator()(qi::net::ErrorCode<N> e, qi::Message*) {
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

TEST(NetMessageSocket, DestroyNotConnectedAsio)
{
  using namespace qi;
  using namespace qi::net;
  using N = NetworkAsio;

  SignalPromises signalPromises;
  {
    auto clientSideSocket = boost::make_shared<TcpMessageSocket<N>>();
    signalPromises = connectSignals(*clientSideSocket);

    // Check that signal were not emitted.
    ASSERT_TRUE(signalPromises.connectedReceived().isRunning());
    ASSERT_TRUE(signalPromises.disconnectedReceived().isRunning());
  }

  // Socket is destroyed : disconnect must not be emitted since it was not
  // connected in the first place.
  ASSERT_TRUE(signalPromises.disconnectedReceived().isRunning());
}

qi::Promise<qi::MessageSocketPtr> listen(
  qi::TransportServer& server,
  const qi::Url& url = {"tcp://127.0.0.1:34987"},
  bool connectNewConnectionSignal = true)
{
  using namespace qi;
  Promise<MessageSocketPtr> promiseConnectedSocket;
  if (connectNewConnectionSignal)
  {
    server.newConnection.connect([=](const std::pair<MessageSocketPtr, Url>& p) mutable {
      promiseConnectedSocket.setValue(p.first);
    });
  }
  server.listen(url);
  return promiseConnectedSocket;
}

TEST(NetMessageSocket, ConnectAndDisconnectAsio)
{
  using namespace qi;
  using namespace qi::net;
  using N = NetworkAsio;

  // Start a server and get the server side socket.
  TransportServer server;
  Url url{"tcp://127.0.0.1:34987"};
  listen(server, url);

  // We also want to check that signals are emitted.
  auto clientSideSocket = boost::make_shared<TcpMessageSocket<N>>();
  auto signalPromises = connectSignals(*clientSideSocket);

  Future<void> fut0 = clientSideSocket->connect(url);
  ASSERT_EQ(FutureState_FinishedWithValue, fut0.wait(defaultTimeoutInMs));
  ASSERT_EQ(FutureState_FinishedWithValue, signalPromises.connectedReceived().wait(defaultTimeoutInMs));
  ASSERT_TRUE(signalPromises.disconnectedReceived().isRunning());

  Future<void> fut1 = clientSideSocket->disconnect();
  ASSERT_EQ(FutureState_FinishedWithValue, fut1.wait(defaultTimeoutInMs));
  ASSERT_EQ(FutureState_FinishedWithValue, signalPromises.disconnectedReceived().wait(defaultTimeoutInMs));
}

TEST(NetMessageSocket, ConnectAndDestroyAsio)
{
  using namespace qi;
  using namespace qi::net;
  using N = NetworkAsio;

  // We also want to check that signals are emitted.
  SignalPromises signalPromises;

  {
    // Start a server and get the server side socket.
    TransportServer server;
    Url url{"tcp://127.0.0.1:34987"};
    listen(server, url);
    auto clientSideSocket = boost::make_shared<TcpMessageSocket<N>>();
    signalPromises = connectSignals(*clientSideSocket);

    Future<void> fut0 = clientSideSocket->connect(url);
    ASSERT_EQ(FutureState_FinishedWithValue, fut0.wait(defaultTimeoutInMs));
    ASSERT_EQ(FutureState_FinishedWithValue, signalPromises.connectedReceived().wait(defaultTimeoutInMs));
    ASSERT_TRUE(signalPromises.disconnectedReceived().isRunning());
  }

  // disconnected must be emitted on destruction.
  ASSERT_EQ(FutureState_FinishedWithValue, signalPromises.disconnectedReceived().wait(defaultTimeoutInMs));
}

TEST(NetMessageSocket, SendWhileNotConnectedAsio)
{
  using namespace qi;
  using namespace qi::net;
  using N = NetworkAsio;

  auto clientSideSocket = boost::make_shared<TcpMessageSocket<N>>();
  auto signalPromises = connectSignals(*clientSideSocket);

  MessageAddress address{1234, 5, 9876, 107};
  ASSERT_FALSE(clientSideSocket->send(makeMessage(address)));

  // Check that signal were not emitted.
  ASSERT_TRUE(signalPromises.connectedReceived().isRunning());
  ASSERT_TRUE(signalPromises.disconnectedReceived().isRunning());
}

TEST(NetMessageSocket, SendAfterDisconnectedAsio)
{
  using namespace qi;
  using namespace qi::net;
  using N = NetworkAsio;

  // Start a server and get the server side socket.
  TransportServer server;
  Url url{"tcp://127.0.0.1:34987"};
  listen(server, url);

  auto clientSideSocket = boost::make_shared<TcpMessageSocket<N>>();
  Future<void> fut0 = clientSideSocket->connect(url);
  ASSERT_EQ(FutureState_FinishedWithValue, fut0.wait(defaultTimeoutInMs));

  Future<void> fut1 = clientSideSocket->disconnect();
  ASSERT_EQ(FutureState_FinishedWithValue, fut1.wait(defaultTimeoutInMs));

  MessageAddress address{1234, 5, 9876, 107};
  ASSERT_FALSE(clientSideSocket->send(makeMessage(address)));
}

TEST(NetMessageSocket, DisconnectWhileNotConnectedAsio)
{
  using namespace qi;
  using namespace qi::net;
  using N = NetworkAsio;

  auto socket = boost::make_shared<TcpMessageSocket<N>>();
  ASSERT_FALSE(socket->disconnect().hasError());
}

TEST(NetMessageSocket, ReceiveOneMessageAsio)
{
  using namespace qi;
  using namespace qi::net;
  using N = NetworkAsio;

  // Start a server and get the server side socket.
  TransportServer server;
  Url url{"tcp://127.0.0.1:34987"};
  auto promiseServerSideSocket = listen(server, url);

  // Connect the client.
  auto msgSent = makeMessage(MessageAddress{1234, 5, 9876, 107});

  Promise<void> promiseReceivedMessage;
  auto clientSideSocket = boost::make_shared<TcpMessageSocket<N>>();//N::defaultIoService(), SslEnabled{true});
  clientSideSocket->messageReady.connect([&](const Message& msgReceived) mutable {
    if (!messageEqual(msgReceived, msgSent)) throw std::runtime_error("messages are not equal.");
    promiseReceivedMessage.setValue(0);
  });
  Future<void> fut = clientSideSocket->connect(url);
  ASSERT_EQ(FutureState_FinishedWithValue, fut.wait(defaultTimeoutInMs));

  // The server sends a message.
  ASSERT_TRUE(promiseServerSideSocket.future().hasValue());
  ASSERT_TRUE(promiseServerSideSocket.future().value()->ensureReading());
  ASSERT_TRUE(promiseServerSideSocket.future().value()->send(msgSent));

  // Wait for the client to receive it.
  ASSERT_EQ(FutureState_FinishedWithValue, promiseReceivedMessage.future().wait(defaultTimeoutInMs));
}

TEST(NetMessageSocketAsio, ReceiveManyMessages)
{
  using namespace qi;
  using namespace qi::net;
  using N = NetworkAsio;

  // Start a server and get the server side socket.
  TransportServer server;
  Url url{"tcp://127.0.0.1:34987"};
  auto promiseServerSideSocket = listen(server, url);

  // Connect the client.
  Promise<void> promiseAllMessageReceived;
  auto clientSideSocket = boost::make_shared<TcpMessageSocket<N>>();
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
  ASSERT_EQ(FutureState_FinishedWithValue, fut.wait(defaultTimeoutInMs));

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

TEST(NetMessageSocket, DisconnectWhileConnecting)
{
  using namespace qi;
  using namespace qi::net;
  using mock::Resolver;
  using namespace mock;
  std::thread resolveThread;

  Resolver::async_resolve = [&](Resolver::query q, Resolver::_anyResolveHandler h) {
    resolveThread = std::thread{[=] {
      defaultAsyncResolve(q, h);
    }};
  };

  Promise<void> promiseAsyncConnectStarted;
  Promise<void> promiseAsyncConnectCanFinish;

  // The async connect waits for a promise to be set before finishing.
  LowestLayer::async_connect = [&](N::_resolver_entry e, N::_anyHandler h) {
    promiseAsyncConnectStarted.setValue(0);
    promiseAsyncConnectCanFinish.future().wait();
    h(success<Error>());
  };

  auto socket = boost::make_shared<TcpMessageSocket<N>>();
  Future<void> futConnect = socket->connect(Url{"tcp://10.11.12.13:1234"});

  // We wait for the async connect to start.
  promiseAsyncConnectStarted.future().wait();
  Future<void> futDisconnect = socket->disconnect();

  // Now the async connect (therefore the connecting part) can finish.
  promiseAsyncConnectCanFinish.setValue(0);
  ASSERT_EQ(FutureState_FinishedWithValue, futDisconnect.wait(defaultTimeoutInMs)) << futDisconnect.error();
  ASSERT_EQ(FutureState_FinishedWithError, futConnect.wait(defaultTimeoutInMs));
  ASSERT_EQ("Abort: disconnection requested while connecting", futConnect.error());
  resolveThread.join();
}

TEST(NetMessageSocket, DisconnectWhileDisconnecting)
{
  using namespace qi;
  using namespace qi::net;
  using mock::Resolver;
  using namespace mock;

  Promise<void> promiseCanFinishAsyncReadWrite;

  Resolver::async_resolve = defaultAsyncResolve;
  LowestLayer::async_connect = defaultAsyncConnect;
  std::vector<std::thread> readThreads, writeThreads;

  // Make sure async read and async write block until the end of this test.
  N::_async_read_next_layer =
    [=, &readThreads](Socket::next_layer_type&, N::_mutable_buffer_sequence, N::_anyTransferHandler h) {
      readThreads.push_back(std::thread{[=] {
        promiseCanFinishAsyncReadWrite.future().wait();
      }});
    };
  N::_async_write_next_layer =
    [=, &writeThreads](Socket::next_layer_type&, const std::vector<N::_const_buffer_sequence>&, N::_anyTransferHandler) {
      writeThreads.push_back(std::thread{[=] {
        promiseCanFinishAsyncReadWrite.future().wait();
      }});
    };
  Promise<void> promiseShutdownStarted;
  Promise<void> promiseCanShutdown;
  LowestLayer::shutdown = [=](LowestLayer::shutdown_type, Error) mutable {
    promiseShutdownStarted.setValue(0);
    promiseCanShutdown.future().wait();
  };

  auto socket = boost::make_shared<TcpMessageSocket<N>>();
  socket->connect(Url{"tcp://10.11.12.13:1234"});

  Future<void> futDisconnect0 = socket->disconnect();
  ASSERT_TRUE(futDisconnect0.isRunning());

  // Wait for entering shutdown().
  ASSERT_EQ(FutureState_FinishedWithValue, promiseShutdownStarted.future().wait(defaultTimeoutInMs));

  // Subsequent disconnect() cannot complete because shutdown() is blocking.
  Future<void> futDisconnect1 = socket->disconnect();
  ASSERT_TRUE(futDisconnect1.isRunning()) << "finished: " << futDisconnect1.isFinished();
  Future<void> futDisconnect2 = socket->disconnect();
  ASSERT_TRUE(futDisconnect2.isRunning());

  // Now, unblock shutdown() so that disconnecting state and later
  // disconnected state can complete.
  promiseCanShutdown.setValue(0);

  // All future must complete.
  ASSERT_EQ(FutureState_FinishedWithValue, futDisconnect0.wait(defaultTimeoutInMs));
  ASSERT_EQ(FutureState_FinishedWithValue, futDisconnect1.wait(defaultTimeoutInMs));
  ASSERT_EQ(FutureState_FinishedWithValue, futDisconnect2.wait(defaultTimeoutInMs));
  ASSERT_FALSE(socket->isConnected());
  ASSERT_EQ(MessageSocket::Status::Disconnected, socket->status());

  promiseCanFinishAsyncReadWrite.setValue(0);
  for (auto& t: readThreads) t.join();
  for (auto& t: writeThreads) t.join();
}

// Stress test for disconnections.
// A lot of threads try to disconnect the same socket at the same time.
// This test should be launched a lot of times to be meaningful.
TEST(NetMessageSocketAsio, DisconnectBurst)
{
  using namespace qi;
  using namespace qi::net;
  using N = NetworkAsio;

  // Start a server.
  TransportServer server;
  Url url{"tcp://127.0.0.1:34987"};
  listen(server, url);

  // Connect the client.
  auto socket = boost::make_shared<TcpMessageSocket<N>>();
  Future<void> fut = socket->connect(url);
  ASSERT_EQ(FutureState_FinishedWithValue, fut.wait(defaultTimeoutInMs));

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

TEST(NetMessageSocketAsio, SendReceiveManyMessages)
{
  using namespace qi;
  using namespace qi::net;
  using N = NetworkAsio;

  // Start a server and get the server side socket.
  TransportServer server;
  Url url{"tcp://127.0.0.1:34987"};
  auto promiseServerSideSocket = listen(server, url);

  // Connect the client.
  Promise<void> promiseAllMessageReceived;
  auto clientSideSocket = boost::make_shared<TcpMessageSocket<N>>();
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
  ASSERT_EQ(FutureState_FinishedWithValue, fut.wait(defaultTimeoutInMs));

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
