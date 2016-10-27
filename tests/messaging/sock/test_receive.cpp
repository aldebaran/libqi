#include <thread>
#include <algorithm>
#include <numeric>
#include <boost/shared_ptr.hpp>
#include <boost/optional.hpp>
#include <gtest/gtest.h>
#include "src/messaging/transportserver.hpp"
#include <qi/future.hpp>
#include "src/messaging/message.hpp"
#include <qi/messaging/sock/networkasio.hpp>
#include "networkasionooplock.hpp"
#include "src/messaging/tcpmessagesocket.hpp"
#include "networkcommon.hpp"
#include "networkmock.hpp"
#include <qi/messaging/sock/receive.hpp>
#include <qi/messaging/sock/accept.hpp>

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
  std::vector<std::thread> writeThreads;
  auto _ = scopedSetAndRestore(
    N::_async_read_next_layer,
    [&](SslSocket<N>::next_layer_type&, N::_mutable_buffer_sequence, N::_anyTransferHandler h) {
      writeThreads.push_back(std::thread{[=] {
        h(ErrorCode<N>{ErrorCode<N>::unknown}, 0u);
      }});
    }
  );
  auto socket = makeSocketPtr<N>(N::defaultIoService(), SslContext<N>{});
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

TYPED_TEST_CASE(NetReceiveMessageContinuous, sequences);

TYPED_TEST(NetReceiveMessageContinuous, FailsOnReadNonSsl)
{
  using namespace qi;
  using namespace qi::sock;
  using N = mock::Network;
  std::vector<std::thread> writeThreads;
  auto _ = scopedSetAndRestore(
    N::_async_read_next_layer,
    [&](SslSocket<N>::next_layer_type&, N::_mutable_buffer_sequence, N::_anyTransferHandler h) {
      writeThreads.push_back(std::thread{[=] {
        h(ErrorCode<N>{ErrorCode<N>::unknown}, 0u);
      }});
    }
  );
  auto socket = makeSocketPtr<N>(N::defaultIoService(), SslContext<N>{});
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
  std::thread callInAnotherThreadAfterFutureIsSet(H h, qi::Promise<T> promise)
  {
    return std::thread([=]{
      promise.future().wait();
      h(N::error_code_type{}, 0u);
    });
  }

  void readHeaderHandlerCalledAfterParentHasBeenDestroyed(qi::sock::SslEnabled ssl)
  {
    using namespace qi;
    using namespace qi::sock;
    qi::Promise<void> nukingObject;
    std::thread t;
    // The read is going to fail. Before calling the handler, we're going to
    // wait the ReceiveMessageContinuous object is destroyed.
    if (*ssl)
    {
      N::_async_read_socket = [&](Socket<N>&, N::_mutable_buffer_sequence, N::_anyTransferHandler h) {
        // We launch asynchronously to return immediately.
        t = callInAnotherThreadAfterFutureIsSet(h, nukingObject);
      };
    }
    else
    {
      N::_async_read_next_layer = [&](N::ssl_socket_type::next_layer_type&, N::_mutable_buffer_sequence, N::_anyTransferHandler h) {
        // We launch asynchronously to return immediately.
        t = callInAnotherThreadAfterFutureIsSet(h, nukingObject);
      };
    }
    ReceiveMessageContinuousTrack<N>* p = nullptr;
    qi::Promise<ErrorCode<N>> promiseError;
    qi::Future<ErrorCode<N>> futureError = promiseError.future();
    {
      auto socket = boost::make_shared<Socket<N>>(N::defaultIoService(), SslContext<N>{});
      const size_t maxPayload = 10000;
      ReceiveMessageContinuousTrack<N> receive;
      receive(socket, ssl, maxPayload, [&](ErrorCode<N> e, const Message*) {
        promiseError.setValue(e);
        return true; // continue receiving messages
      });
      p = &receive;
    }
    // The connecting object is now destroyed and on top of that we wipe out its memory.
    // cppcheck-suppress deadpointer
    overwrite(p);
    // Now we unblock the handler.
    nukingObject.setValue(0);
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
    Promise<void> nukingObject;
    std::thread t;
    // The read is going to fail. Before calling the handler, we're going to
    // wait the ReceiveMessageContinuous object is destroyed.
    int callCount = 0;
    auto readHeaderThenDataAfterParentHasBeenDestroyed =
      [&](N::_mutable_buffer_sequence buf, N::_anyTransferHandler h) {
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
      N::_async_read_socket = [&](Socket<N>&, N::_mutable_buffer_sequence buf, N::_anyTransferHandler h) {
        readHeaderThenDataAfterParentHasBeenDestroyed(buf, h);
      };
    }
    else
    {
      N::_async_read_next_layer = [&](Socket<N>::next_layer_type&, N::_mutable_buffer_sequence buf, N::_anyTransferHandler h) {
        readHeaderThenDataAfterParentHasBeenDestroyed(buf, h);
      };
    }
    ReceiveMessageContinuousTrack<N>* p = nullptr;
    Promise<ErrorCode<N>> promiseError;
    Future<ErrorCode<N>> futureError = promiseError.future();
    {
      auto socket = boost::make_shared<Socket<N>>(N::defaultIoService(), SslContext<N>{});
      const size_t maxPayload = 10000;
      ReceiveMessageContinuousTrack<N> receive;
      receive(socket, ssl, maxPayload, [=](ErrorCode<N> e, const Message*) mutable {
        promiseError.setValue(e);
        return true;
      });
      p = &receive;
    }
    // The connecting object is now destroyed and on top of that we wipe out its memory.
    // cppcheck-suppress deadpointer
    overwrite(p);
    // Now we unblock the handler.
    nukingObject.setValue(0);
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
  std::atomic<int> callCount{0};
  std::mutex writeThreadsMutex;
  std::vector<std::thread> writeThreads;
  auto _ = scopedSetAndRestore(
    N::_async_read_next_layer,
    [&](SslSocket<N>::next_layer_type&, N::_mutable_buffer_sequence, N::_anyTransferHandler h) {
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
  auto socket = boost::make_shared<Socket<N>>(N::defaultIoService(), SslContext<N>{});
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
  std::vector<std::thread> threads;
  mock::AsyncReadNextLayerHeaderThenData h;
  ++h._magic; // make it wrong
  auto _ = scopedSetAndRestore(
    N::_async_read_next_layer,
    [=, &threads](SslSocket<N>::next_layer_type& s, N::_mutable_buffer_sequence buf, N::_anyTransferHandler handler) mutable {
      threads.push_back(std::thread{[=]() mutable {h(s, buf, handler);}});
    }
  );
  auto socket = makeSocketPtr<N>(N::defaultIoService(), SslContext<N>{});
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
  auto _ = scopedSetAndRestore(N::_async_read_next_layer, h);
  auto socket = makeSocketPtr<N>(N::defaultIoService(), SslContext<N>{});
  Promise<ErrorCode<N>> promiseError;
  Future<ErrorCode<N>> futureError = promiseError.future();
  ReceiveMessageContinuous<N> receive;
  receive(socket, SslEnabled{false}, maxPayload, [=](ErrorCode<N> e, const Message*) mutable {
    promiseError.setValue(e);
    return false;
  });
  ASSERT_EQ(messageSize<ErrorCode<N>>(), futureError.value());
}

TYPED_TEST(NetReceiveMessageContinuous, FailsOnReadData)
{
  using namespace qi;
  using namespace qi::sock;
  using N = mock::Network;
  mock::AsyncReadNextLayerHeaderThenData h;
  h._dataError = ErrorCode<N>{ErrorCode<N>::unknown}; // read data will fail
  auto _ = scopedSetAndRestore(N::_async_read_next_layer, h);
  auto socket = makeSocketPtr<N>(N::defaultIoService(), SslContext<N>{});
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
  auto _ = scopedSetAndRestore(N::_async_read_next_layer, mock::AsyncReadNextLayerHeaderThenData{});
  auto socket = makeSocketPtr<N>(N::defaultIoService(), SslContext<N>{});
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

  auto& io = N::defaultIoService();
  SslContext<N> context{Method<SslContext<N>>::sslv23};
  const Url url{"tcp://127.0.0.1:34987"};

  Promise<SocketPtr<N>> promiseConnect;

  AcceptConnectionContinuous<N> accept{io};
  accept(context, url,
    IpV6Enabled{false}, ReuseAddressEnabled{true},
    [=](ErrorCode<N> erc, SocketPtr<N> socket) mutable {
      if (erc)
      {
        std::cerr << "Accept error: " << erc.message() << '\n';
        return false;
      }
      promiseConnect.setValue(socket);
      return false;
    }
  );

  // Connect client.
  using Side = HandshakeSide<SslSocket<N>>;
  ConnectSocketFuture<N> connect{io};
  connect(url, SslEnabled{false}, context, IpV6Enabled{true},
    Side::client);
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
  ASSERT_TRUE(promiseReceive.future().waitFor(defaultTimeout)); // milliseconds.
  ASSERT_EQ(msgAddress, msgReceived.address());
  ASSERT_EQ(bufSend.totalSize(), msgReceived.buffer().totalSize());
  ASSERT_TRUE(std::equal((char*)bufSend.data(), (char*)bufSend.data() + bufSend.size(), (char*)msgReceived.buffer().data()));

  close<N>(clientSideSocket);
}
