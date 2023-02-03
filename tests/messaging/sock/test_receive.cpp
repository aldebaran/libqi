#include <thread>
#include <algorithm>
#include <numeric>
#include <boost/shared_ptr.hpp>
#include <boost/optional.hpp>
#include <gtest/gtest.h>
#include "src/messaging/transportserver.hpp"
#include <qi/future.hpp>
#include "src/messaging/message.hpp"
#include <src/messaging/sock/networkasio.hpp>
#include <src/messaging/sock/sslcontextptr.hpp>
#include "networkasionooplock.hpp"
#include "src/messaging/tcpmessagesocket.hpp"
#include "networkcommon.hpp"
#include "networkmock.hpp"
#include <src/messaging/sock/receive.hpp>
#include <src/messaging/sock/accept.hpp>
#include <qi/testutils/testutils.hpp>
#include <ka/macro.hpp>

static const qi::MilliSeconds defaultTimeout{500};
static const std::chrono::milliseconds defaultPostPauseInMs{20};

////////////////////////////////////////////////////////////////////////////////
/// NetReceiveMessage tests:
////////////////////////////////////////////////////////////////////////////////
TEST(NetReceiveMessage, FailsOnReadNonSsl)
{
  using namespace qi;
  using namespace qi::sock;
  using N = mock::Network;
  using S = SslSocket<N>;

  std::vector<std::thread> writeThreads;
  auto _ = ka::scoped_set_and_restore(
    N::_async_read_next_layer,
    [&](S::next_layer_type&, N::_mutable_buffer_sequence, N::_anyTransferHandler h) {
      writeThreads.push_back(std::thread{[=] {
        h(ErrorCode<N>{ErrorCode<N>::unknown}, 0u);
      }});
    }
  );
  SslContext<N> context;
  auto socket = makeSslSocketPtr<N>(N::defaultIoService(), context);
  const size_t maxPayload = 10000;
  Message msg;
  Promise<ErrorCode<N>> promise;
  receiveMessage<N>(socket, &msg, SslEnabled{false}, maxPayload, [=](ErrorCode<N> e, const Message*) mutable {
    promise.setError(e.message());
    return boost::optional<Message*>{};
  });
  ASSERT_EQ("unknown", promise.future().error());
  for (auto& t: writeThreads) t.join();
  std::this_thread::sleep_for(defaultPostPauseInMs);
}

////////////////////////////////////////////////////////////////////////////////
/// NetReceiveMessageContinuous tests:
////////////////////////////////////////////////////////////////////////////////

template<typename T>
struct NetReceiveMessageContinuous : testing::Test
{
};

using sequences = testing::Types<
  // Mock
  qi::sock::ReceiveMessageContinuous<mock::N>//, qi::sock::Connected<mock::N>
  // Asio
  //, qi::sock::ReceiveMessageContinuous<qi::sock::NetworkAsio>, qi::sock::Connected<qi::sock::NetworkAsio>
>;

TYPED_TEST_SUITE(NetReceiveMessageContinuous, sequences);

TYPED_TEST(NetReceiveMessageContinuous, FailsOnReadNonSsl)
{
  using namespace qi;
  using namespace qi::sock;
  using N = mock::Network;
  using S = SslSocket<N>;

  std::vector<std::thread> writeThreads;
  auto _ = ka::scoped_set_and_restore(
    N::_async_read_next_layer,
    [&](S::next_layer_type&, N::_mutable_buffer_sequence, N::_anyTransferHandler h) {
      writeThreads.push_back(std::thread{[=] {
        h(ErrorCode<N>{ErrorCode<N>::unknown}, 0u);
      }});
    }
  );

  SslContext<N> context;
  auto socket = makeSslSocketPtr<N>(N::defaultIoService(), context);
  const size_t maxPayload = 10000;
  Promise<ErrorCode<N>> promise;
  ReceiveMessageContinuous<N> receive;
  receive(socket, SslEnabled{false}, maxPayload, [=](ErrorCode<N> e, const Message*) mutable {
    promise.setValue(e);
    return false; // Stop receiving messages.
  });
  ASSERT_EQ(ErrorCode<N>{ErrorCode<N>::unknown}, promise.future().value());
  for (auto& t: writeThreads) t.join();
  std::this_thread::sleep_for(defaultPostPauseInMs);
}

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

  template<typename H, typename T> // NetHandler H
  std::thread callInAnotherThreadAfterFutureIsSet(H h, qi::Future<T> future)
  {
    return std::thread([=]{
      future.wait();
      h(N::error_code_type{}, 0u);
    });
  }

  void readHeaderHandlerCalledAfterParentHasBeenDestroyed(qi::sock::SslEnabled ssl)
  {
    using namespace qi;
    using namespace qi::sock;
    qi::Promise<void> nukingObjectProm;
    auto nukingObject = nukingObjectProm.future();
    std::thread t;
    // The read is going to fail. Before calling the handler, we're going to
    // wait the ReceiveMessageContinuous object is destroyed.
    if (*ssl)
    {
KA_WARNING_PUSH()
KA_WARNING_DISABLE(4068, pragmas)
KA_WARNING_DISABLE(, undefined-var-template)
      N::SocketFunctions<S>::_async_read_socket
          = [&](S&, N::_mutable_buffer_sequence, N::_anyTransferHandler h) {
        // We launch asynchronously to return immediately.
        t = callInAnotherThreadAfterFutureIsSet(h, nukingObject);
      };
KA_WARNING_POP()
    }
    else
    {
      N::_async_read_next_layer = [&](N::ssl_socket_type::next_layer_type&, N::_mutable_buffer_sequence, N::_anyTransferHandler h) {
        // We launch asynchronously to return immediately.
        t = callInAnotherThreadAfterFutureIsSet(h, nukingObject);
      };
    }

    using R = ReceiveMessageContinuousTrack<N>;
    typename std::aligned_storage<sizeof(R), alignof(R)>::type storage;
    auto& receive = reinterpret_cast<R&>(storage);
    qi::Promise<ErrorCode<N>> promiseError;
    qi::Future<ErrorCode<N>> futureError = promiseError.future();
    {
      SslContext<N> context;
      auto socket = makeSslSocketPtr<N>(N::defaultIoService(), context);
      const size_t maxPayload = 10000;
      auto _ = ka::scoped(new (&receive) R, [](R* p){ p->~R(); });
      receive(socket, ssl, maxPayload, [&](ErrorCode<N> e, const Message*) {
        promiseError.setValue(e);
        return true; // continue receiving messages
      });
    }
    // The connecting object is now destroyed and on top of that we wipe out its memory.
    overwrite(&receive);
    // Now we unblock the handler.
    nukingObjectProm.setValue(0);
    // We wait for an error to occur.
    ASSERT_TRUE(futureError.hasValue());
    ASSERT_EQ(operationAborted<ErrorCode<N>>(), futureError.value());
    t.join();
  }
} // namespace mock

TYPED_TEST(NetReceiveMessageContinuous, ReadHeaderHandlerCalledAfterParentHasBeenDestroyedNonSsl)
{
  mock::readHeaderHandlerCalledAfterParentHasBeenDestroyed(qi::sock::SslEnabled{false});
}

TYPED_TEST(NetReceiveMessageContinuous, ReadHeaderHandlerCalledAfterParentHasBeenDestroyedSsl)
{
  mock::readHeaderHandlerCalledAfterParentHasBeenDestroyed(qi::sock::SslEnabled{true});
}

namespace mock
{
  void readDataHandlerCalledAfterParentHasBeenDestroyed(qi::sock::SslEnabled ssl)
  {
    using namespace qi;
    using namespace qi::sock;
    Promise<void> nukingObjectPromise;
    auto nukingObject = nukingObjectPromise.future();
    std::thread t;
    // The read is going to fail. Before calling the handler, we're going to
    // wait the ReceiveMessageContinuous object is destroyed.
    int callCount = 0;
    auto readHeaderThenDataAfterParentHasBeenDestroyed =
      [&, nukingObject](N::_mutable_buffer_sequence buf, N::_anyTransferHandler h) {
        ++callCount;
        if (callCount == 1)
        { // read header
          const auto size = 10u;
          readHeader(buf, h, Message::Header::magicCookie, size);
        }
        else
        { // read data
          // We launch asynchronously to return immediately.
          t = callInAnotherThreadAfterFutureIsSet(h, nukingObject);
        }
      };

    if (*ssl)
    {
      N::SocketFunctions<S>::_async_read_socket
          = [=](S&, N::_mutable_buffer_sequence buf, N::_anyTransferHandler h) {
        readHeaderThenDataAfterParentHasBeenDestroyed(buf, h);
      };
    }
    else
    {
      N::_async_read_next_layer = [=](S::next_layer_type&, N::_mutable_buffer_sequence buf, N::_anyTransferHandler h) {
        readHeaderThenDataAfterParentHasBeenDestroyed(buf, h);
      };
    }

    using R = ReceiveMessageContinuousTrack<N>;
    typename std::aligned_storage<sizeof(R), alignof(R)>::type storage;
    auto& receive = reinterpret_cast<R&>(storage);
    Promise<ErrorCode<N>> promiseError;
    Future<ErrorCode<N>> futureError = promiseError.future();
    {
      SslContext<N> context;
      auto socket = makeSslSocketPtr<N>(N::defaultIoService(), context);
      const size_t maxPayload = 10000;
      auto _ = ka::scoped(new (&receive) R, [](R* p){ p->~R(); });
      receive(socket, ssl, maxPayload, [=](ErrorCode<N> e, const Message*) mutable {
        promiseError.setValue(e);
        return true;
      });
    }
    // The connecting object is now destroyed and on top of that we wipe out its memory.
    overwrite(&receive);
    // Now we unblock the handler.
    nukingObjectPromise.setValue(0);
    // We wait for an error to occur.
    ASSERT_TRUE(futureError.hasValue());
    ASSERT_EQ(operationAborted<ErrorCode<N>>(), futureError.value());
    t.join();
  }
} // namespace mock

TYPED_TEST(NetReceiveMessageContinuous, ReadDataHandlerCalledAfterParentHasBeenDestroyedNonSsl)
{
  mock::readDataHandlerCalledAfterParentHasBeenDestroyed(false);
}

TYPED_TEST(NetReceiveMessageContinuous, ReadDataHandlerCalledAfterParentHasBeenDestroyedSsl)
{
  mock::readDataHandlerCalledAfterParentHasBeenDestroyed(true);
}

TYPED_TEST(NetReceiveMessageContinuous, AsyncReadCalledUntilSomethingIsActuallyRead)
{
  using namespace qi;
  using namespace qi::sock;
  using N = mock::Network;
  using S = SslSocket<N>;

  std::atomic<int> callCount{0};
  std::mutex writeThreadsMutex;
  std::vector<std::thread> writeThreads;
  auto _ = ka::scoped_set_and_restore(
    N::_async_read_next_layer,
    [&](S::next_layer_type&, N::_mutable_buffer_sequence, N::_anyTransferHandler h) {
      std::lock_guard<std::mutex> lock{writeThreadsMutex};
      writeThreads.push_back(std::thread{[&, h] {
        ++callCount;
        // Error when called 11th times.
        h(ErrorCode<N>{callCount == 11 ? ErrorCode<N>::unknown : ErrorCode<N>::success}, 0u);
      }});
    }
  );
  Promise<ErrorCode<N>> promiseError;
  Future<ErrorCode<N>> futureError = promiseError.future();
  SslContext<N> context;
  auto socket = makeSslSocketPtr<N>(N::defaultIoService(), context);
  const size_t maxPayload = 10000;
  ReceiveMessageContinuous<N> receive;
  receive(socket, SslEnabled{false}, maxPayload,
    [&](ErrorCode<N> e, const Message*) mutable {
      if (e)
      {
        promiseError.setValue(e);
        return false;
      }
      return true;
    }
  );
  ASSERT_TRUE(futureError.hasValue());
  ASSERT_EQ(ErrorCode<N>{ErrorCode<N>::unknown}, futureError.value());
  ASSERT_EQ(11, callCount);
  std::lock_guard<std::mutex> lock{writeThreadsMutex};
  for (auto& t: writeThreads) t.join();
}

namespace mock
{
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

TYPED_TEST(NetReceiveMessageContinuous, FailsOnReadHeaderBecauseOfBadMessageCookie)
{
  using namespace qi;
  using namespace qi::sock;
  using N = mock::Network;
  using S = SslSocket<N>;

  std::vector<std::thread> threads;
  mock::AsyncReadNextLayerHeaderThenData h;
  ++h._magic; // make it wrong
  auto _ = ka::scoped_set_and_restore(
    N::_async_read_next_layer,
    [=, &threads](S::next_layer_type& s, N::_mutable_buffer_sequence buf, N::_anyTransferHandler handler) mutable {
      threads.push_back(std::thread{[=]() mutable {h(s, buf, handler);}});
    }
  );
  SslContext<N> context;
  auto socket = makeSslSocketPtr<N>(N::defaultIoService(), context);
  Promise<ErrorCode<N>> promiseError;
  const size_t maxPayload = 10000;
  ReceiveMessageContinuous<N> receive;
  receive(socket, SslEnabled{false}, maxPayload, [=](ErrorCode<N> e, const Message*) mutable {
    promiseError.setValue(e);
    return false;
  });
  ASSERT_EQ(fault<ErrorCode<N>>(), promiseError.future().value());
  for (auto& t: threads) t.join();
}

TYPED_TEST(NetReceiveMessageContinuous, FailsOnReadHeaderBecausePayloadIsTooBig)
{
  using namespace qi;
  using namespace qi::sock;
  using N = mock::Network;

  const size_t maxPayload = 10000;
  mock::AsyncReadNextLayerHeaderThenData h;
  ++h._headerSize = maxPayload + 1; // make it wrong
  auto _ = ka::scoped_set_and_restore(N::_async_read_next_layer, h);
  SslContext<N> context;
  auto socket = makeSslSocketPtr<N>(N::defaultIoService(), context);
  Promise<ErrorCode<N>> promiseError;
  Future<ErrorCode<N>> futureError = promiseError.future();
  ReceiveMessageContinuous<N> receive;
  receive(socket, SslEnabled{false}, maxPayload, [=](ErrorCode<N> e, const Message*) mutable {
    promiseError.setValue(e);
    return false;
  });
  ASSERT_EQ(messageSize<ErrorCode<N>>(), futureError.value());
}

TYPED_TEST(NetReceiveMessageContinuous, FailsOnReadHeaderBecauseCannotAllocateBuffer)
{
  using namespace qi;
  using namespace qi::sock;
  using N = mock::Network;

  mock::AsyncReadNextLayerHeaderThenData h;

  // Make reading the header cause a "no memory" error.
  h._headerError = noMemory<ErrorCode<N>>();

  auto _ = ka::scoped_set_and_restore(N::_async_read_next_layer, h);
  SslContext<N> context;
  auto socket = makeSslSocketPtr<N>(N::defaultIoService(), context);
  Promise<ErrorCode<N>> promiseError;
  Future<ErrorCode<N>> futureError = promiseError.future();
  ReceiveMessageContinuous<N> receive;
  const size_t maxPayload = 10000;
  receive(socket, SslEnabled{false}, maxPayload, [=](ErrorCode<N> e, const Message*) mutable {
    promiseError.setValue(e);
    return false;
  });
  ASSERT_EQ(noMemory<ErrorCode<N>>(), futureError.value());
}

TYPED_TEST(NetReceiveMessageContinuous, FailsOnReadData)
{
  using namespace qi;
  using namespace qi::sock;
  using N = mock::Network;

  mock::AsyncReadNextLayerHeaderThenData h;
  h._dataError = ErrorCode<N>{ErrorCode<N>::unknown}; // read data will fail
  auto _ = ka::scoped_set_and_restore(N::_async_read_next_layer, h);
  SslContext<N> context;
  auto socket = makeSslSocketPtr<N>(N::defaultIoService(), context);
  const size_t maxPayload = 10000;
  Promise<ErrorCode<N>> promiseError;
  Future<ErrorCode<N>> futureError = promiseError.future();
  ReceiveMessageContinuous<N> receive;
  receive(socket, SslEnabled{false}, maxPayload, [=](ErrorCode<N> e, const Message*) mutable {
    promiseError.setValue(e);
    return false;
  });
  ASSERT_EQ(ErrorCode<N>{ErrorCode<N>::unknown}, futureError.value());
  ASSERT_EQ(2, N::_async_read_next_layer.target<mock::AsyncReadNextLayerHeaderThenData>()->_callCount);
}

TYPED_TEST(NetReceiveMessageContinuous, SeveralMessagesHandled)
{
  using namespace qi;
  using namespace qi::sock;
  using N = mock::Network;

  auto _ = ka::scoped_set_and_restore(N::_async_read_next_layer, mock::AsyncReadNextLayerHeaderThenData{});
  SslContext<N> context;
  auto socket = makeSslSocketPtr<N>(N::defaultIoService(), context);
  int messageHandledCount = 0;
  const int maxMessageHandledCount = 11;
  const size_t maxPayload = 10000;
  Promise<ErrorCode<N>> promiseError;
  Future<ErrorCode<N>> futureError = promiseError.future();
  ReceiveMessageContinuous<N> receive;
  receive(socket, SslEnabled{false}, maxPayload, [&](ErrorCode<N> e, const Message*) mutable {
    ++messageHandledCount;
    if (e)
    {
      promiseError.setValue(e);
      return false;
    }
    if (messageHandledCount == maxMessageHandledCount)
    {
      promiseError.setValue(success<ErrorCode<N>>());
    }
    return messageHandledCount < maxMessageHandledCount; // success until the max th message is received.
  });
  ASSERT_EQ(success<ErrorCode<N>>(), futureError.value());
  ASSERT_EQ(maxMessageHandledCount, messageHandledCount);
  ASSERT_EQ(2 * maxMessageHandledCount,
    N::_async_read_next_layer.target<mock::AsyncReadNextLayerHeaderThenData>()->_callCount);
}

TEST(NetReceiveMessage, Asio)
{
  using namespace qi;
  using namespace qi::sock;
  using N = NetworkAsio;
  using S = SslSocket<N>;
  using E = Endpoint<Lowest<S>>;

  auto& io = N::defaultIoService();
  SslContext<N> context{ Method<SslContext<N>>::tlsv12 };

  Promise<SocketPtr<S>> promiseConnect;
  Promise<E> localEndpoint;

  auto makeSocket = [&]{ return makeSslSocketPtr<N>(io, context); };
  AcceptConnectionContinuous<N, S> accept{io};
  accept(makeSocket, "tcp://127.0.0.1:0",
    IpV6Enabled{false}, ReuseAddressEnabled{true},
    [=](ErrorCode<N> erc, SocketPtr<S> socket) mutable {
      if (erc)
      {
        std::cerr << "Accept error: " << erc.message() << '\n';
        return false;
      }
      promiseConnect.setValue(socket);
      return false;
    },
    [&](ErrorCode<N> erc, boost::optional<E> ep) { // onListen
      if (erc)
      {
        localEndpoint.setError(erc.message());
        throw std::runtime_error{std::string{"Listen error: "} + erc.message()};
      }
      if (!ep)
      {
        localEndpoint.setError("Local endpoint is undefined");
        throw std::runtime_error{std::string{"Listen error: local endpoint is undefined"}};
      }
      localEndpoint.setValue(*ep);
    }
  );

  // Connect client.
  using Side = HandshakeSide<S>;
  ConnectSocketFuture<N, S> connect{io};
  connect(url(localEndpoint.future().value(), TcpScheme::Raw), makeSocket,
          IpV6Enabled{true}, Side::client);
  ASSERT_TRUE(connect.complete().hasValue()) << connect.complete().error();
  auto clientSideSocket = connect.complete().value();

  // Listen for messages on client side.
  Promise<void> promiseReceive;
  const size_t maxPayload = 10000000;
  Message msgReceived;
  receiveMessage<N>(clientSideSocket, &msgReceived, SslEnabled{false}, maxPayload,
    [=, &msgReceived](ErrorCode<N> e, boost::optional<Message*> m) mutable {
      if (e) throw std::runtime_error(e.message());
      if (!m) throw std::runtime_error("NetReceiveMessage: received an empty message");
      if (m.value() != &msgReceived) throw std::runtime_error("NetReceiveMessage: received an empty message");
      promiseReceive.setValue(0);
      return boost::optional<Message*>{}; // Fail in order to end receiving messages.
    }
  );

  // Send a message.
  const MessageAddress msgAddress{1234, 5, 9876, 107};
  Message msgSend{Message::Type_Call, msgAddress};
  std::vector<int> data(10000);
  std::iota(data.begin(), data.end(), 42);
  Buffer bufSend;
  bufSend.write(&data[0], data.size() * sizeof(data[0]));
  msgSend.setBuffer(bufSend);

  auto noMoreMessage = [=](ErrorCode<N> e, Message*) {
    if (e) throw std::runtime_error("error sending message");
    return boost::optional<Message*>{};
  };
  ASSERT_TRUE(promiseConnect.future().hasValue());
  sendMessage<N>(promiseConnect.future().value(), &msgSend, noMoreMessage, SslEnabled{false});

  // Wait the client to receive it.
  ASSERT_TRUE(test::finishesWithValue(promiseReceive.future(), test::willDoNothing(), defaultTimeout));
  ASSERT_EQ(msgAddress, msgReceived.address());
  ASSERT_EQ(bufSend.totalSize(), msgReceived.buffer().totalSize());
  ASSERT_TRUE(std::equal((char*)bufSend.data(), (char*)bufSend.data() + bufSend.size(), (char*)msgReceived.buffer().data()));

  close<N>(clientSideSocket);
}
