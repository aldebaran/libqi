#include <thread>
#include <future>
#include <mutex>
#include <boost/shared_ptr.hpp>
#include <boost/optional.hpp>
#include <gtest/gtest.h>
#include <qi/messaging/sock/send.hpp>
#include <qi/future.hpp>
#include <qi/scoped.hpp>
#include "src/messaging/message.hpp"
#include "src/messaging/tcpmessagesocket.hpp"
#include "networkmock.hpp"
#include "networkcommon.hpp"

static const std::chrono::milliseconds defaultPostPauseInMs{20};

////////////////////////////////////////////////////////////////////////////////
// NetSendMessage tests
////////////////////////////////////////////////////////////////////////////////

TEST(NetSendMessage, FailsOnWrite)
{
  using namespace qi;
  using namespace qi::sock;
  using namespace mock;
  auto _ = scopedSetAndRestore(
    N::_async_write_next_layer,
    [](Socket::next_layer_type&, const std::vector<N::_const_buffer_sequence>&, N::_anyTransferHandler h) {
      h(Error{Error::unknown}, 0u);
    }
  );
  IoService<N> io;
  auto socket = boost::make_shared<Socket>(io, SslContext<N>{});
  Message sentMsg;
  Promise<std::pair<Error, Message*>> promise;
  auto onComplete = [&](Error e, Message* m) {
    promise.setValue({e, m});
    return boost::optional<Message*>{};
  };
  sendMessage<N>(socket, &sentMsg, onComplete, SslEnabled{false});
  auto fut = promise.future();
  ASSERT_TRUE(fut.hasValue());
  ASSERT_EQ(fut.value().first, Error{Error::unknown});
  ASSERT_EQ(fut.value().second, &sentMsg);
}

TEST(NetSendMessage, SuccessSingleMessage)
{
  using namespace qi;
  using namespace qi::sock;
  using namespace mock;
  auto _ = scopedSetAndRestore(
    N::_async_write_next_layer,
    [](Socket::next_layer_type&, const std::vector<N::_const_buffer_sequence>&, N::_anyTransferHandler h) {
      h(Error{Error::success}, 0u);
    }
  );
  IoService<N> io;
  auto socket = boost::make_shared<Socket>(io, SslContext<N>{});
  Message sentMsg;
  Promise<std::pair<Error, Message*>> promise;
  auto onComplete = [&](Error e, Message* m) {
    promise.setValue({e, m});
    return boost::optional<Message*>{};
  };
  sendMessage<N>(socket, &sentMsg, onComplete, SslEnabled{false});
  auto fut = promise.future();
  ASSERT_TRUE(fut.hasValue());
  ASSERT_EQ(fut.value().first, Error{Error::success});
  ASSERT_EQ(fut.value().second, &sentMsg);
}

TEST(NetSendMessage, SuccessMultipleMessage)
{
  using namespace qi;
  using namespace qi::sock;
  using namespace mock;
  std::vector<std::thread> writeThreads;
  auto _ = scopedSetAndRestore(
    N::_async_write_next_layer,
    [&](Socket::next_layer_type&, const std::vector<N::_const_buffer_sequence>&, N::_anyTransferHandler h) {
      writeThreads.push_back(std::thread([&, h]{
        h(success<Error>(), 0u);
      }));
    }
  );
  IoService<N> io;
  auto socket = boost::make_shared<Socket>(io, SslContext<N>{});
  std::vector<Message> messages(100);
  auto itMsg = messages.begin();
  using I = decltype(itMsg);
  while (itMsg != messages.end())
  {
    Promise<std::pair<Error, I>> promise;
    auto onComplete = [&](Error e, I m) {
      promise.setValue({e, m});
      return boost::optional<I>{}; // no next message
    };
    sendMessage<N>(socket, itMsg, onComplete, SslEnabled{false});
    auto fut = promise.future();
    ASSERT_TRUE(fut.hasValue());
    ASSERT_EQ(fut.value().first, success<Error>());
    ASSERT_EQ(fut.value().second, itMsg);
    ++itMsg;
  }
  for (auto& t: writeThreads) t.join();
  std::this_thread::sleep_for(defaultPostPauseInMs);
}

TEST(NetSendMessage, SuccessMultipleMessageRafaleMode)
{
  using namespace qi;
  using namespace qi::sock;
  using namespace mock;
  boost::synchronized_value<std::vector<std::thread>> writeThreads;
  std::atomic_bool finished {false};
  auto _ = scopedSetAndRestore(
    N::_async_write_next_layer,
    [&](Socket::next_layer_type&, const std::vector<N::_const_buffer_sequence>&, N::_anyTransferHandler h) {
      if (!finished)
      {
        auto syncThreads = writeThreads.synchronize();
        if (!finished)
        {
          syncThreads->push_back(std::thread([&, h]{
            h(success<Error>(), 0u);
          }));
        }
      }
    }
  );
  IoService<N> io;
  auto socket = boost::make_shared<Socket>(io, SslContext<N>{});
  const int messageCount = 100;
  std::vector<Message> messages(messageCount);
  auto itMsg = messages.begin();
  int sendCount = 0;
  Promise<void> promise;
  using I = decltype(itMsg);
  auto onComplete = [&](Error e, I m) -> boost::optional<I> {
    if (m != itMsg)
    {
      promise.setError("Wrong message.");
      return {};
    }
    if (e != success<Error>())
    {
      promise.setError("Wrong error value.");
      return {};
    }
    ++itMsg;
    ++sendCount;
    boost::optional<I> next;
    if (itMsg == messages.end())
    {
      promise.setValue(0);
    }
    else
    {
      next = itMsg;
    }
    return next;
  };
  sendMessage<N>(socket, itMsg, onComplete, SslEnabled{false});
  promise.future().wait();
  ASSERT_EQ(sendCount, messageCount);
  {
    finished = true;
    auto syncThreads = writeThreads.synchronize();
    for (auto& t : *syncThreads)
    {
      if (t.joinable())
        t.join();
    }
  }
  std::this_thread::sleep_for(defaultPostPauseInMs);
}

////////////////////////////////////////////////////////////////////////////////
// NetSendMessageEnqueue tests
////////////////////////////////////////////////////////////////////////////////

template<typename T>
struct NetSendMessageEnqueue : testing::Test
{
};

using sequences = testing::Types<
  // Mock
  qi::sock::SendMessageEnqueue<mock::N> //, ConnectedSender<mock::N>
  // Asio
  //, qi::sock::MessageReceiver<qi::sock::NetworkAsio>, qi::sock::Connected<qi::sock::NetworkAsio>
>;

TYPED_TEST_CASE(NetSendMessageEnqueue, sequences);

TYPED_TEST(NetSendMessageEnqueue, MultipleSendsFromMultipleThreads)
{
  using SendMessageEnqueue = TypeParam;
  using namespace qi;
  using namespace qi::sock;
  using namespace mock;
  auto scopedRead = scopedSetAndRestore(
    N::_async_read_next_layer,
    defaultAsyncReadNextLayer
  );
  const unsigned sendThreadCount = 10u;
  const unsigned perSendThreadMessageCount = 100u;
  const unsigned maxSentCount = sendThreadCount * perSendThreadMessageCount;
  std::mutex writeThreadMutex;
  std::vector<std::thread> writeThreads;
  writeThreads.reserve(maxSentCount);
  auto scopedWrite = scopedSetAndRestore(
    N::_async_write_next_layer,
    [&](Socket::next_layer_type&, const std::vector<N::_const_buffer_sequence>&,
          N::_anyTransferHandler writeCont) mutable {
      // We launch the continuation in another thread to avoid recursive calls.
      std::lock_guard<std::mutex> lock(writeThreadMutex);
      writeThreads.push_back(std::thread([=]() mutable {
        writeCont(success<Error>(), 0u);
      }));
    }
  );
  auto socket = boost::make_shared<Socket>(N::defaultIoService(), SslContext<N>{});
  using I = std::list<Message>::const_iterator;
  std::atomic<unsigned> sentCount{0u};
  Promise<void> promiseEnoughSent;
  SendMessageEnqueue send{socket};
  auto onSent = [&](ErrorCode<N>, I) {
    // We wait for all messages to be enqueued before starting processing them.
    ++sentCount;
    if (sentCount > maxSentCount)
    {
      promiseEnoughSent.setError("Too many messages sent.");
      return false;
    }
    if (sentCount < maxSentCount) return true; // `true` means "continue processing the message queue".
    promiseEnoughSent.setValue(0);
    return false;
  };
  std::mutex randMutex;
  std::random_device rd;
  std::mt19937 gen{rd()};
  std::uniform_int_distribution<> rand{1, 20};
  std::vector<std::thread> sendThreads;
  for (auto i = sendThreadCount; i != 0; --i)
  {
    sendThreads.push_back(std::thread{
      [&] {
        for (auto j = perSendThreadMessageCount; j != 0; --j)
        {
          send(Message{}, SslEnabled{false}, onSent);
          std::this_thread::sleep_for(std::chrono::milliseconds{syncRand(rand, gen, randMutex)});
        }
      }
    });
  }
  // Wait a bit for the first send to complete.
  ASSERT_EQ(FutureState_FinishedWithValue, promiseEnoughSent.future().wait());
  // As we requested to stop processing the message queue, only one message
  // should have been sent.
  ASSERT_EQ(maxSentCount, sentCount);
  // Allow detached thread to finish.
  {
    std::lock_guard<std::mutex> lock(writeThreadMutex);
    for (auto& t: writeThreads) t.join();
  }
  for (auto& t: sendThreads) t.join();
  std::this_thread::sleep_for(defaultPostPauseInMs);
}

// Multiple threads send messages with the same send object.
// The socket is not connected so the send fails but it's not important here.
// See test_tcpmessagesocket for a similar test on the real socket.
TEST(NetSendMessageEnqueue, MultipleSendsFromMultipleThreadsAsio)
{
  using namespace qi;
  using namespace qi::sock;
  using N = NetworkAsio;
  SslContext<N> context{Method<SslContext<N>>::sslv23};

  auto socket = boost::make_shared<SslSocket<N>>(N::defaultIoService(), context);
  using I = std::list<Message>::const_iterator;
  const unsigned sendThreadCount = 100u;
  const unsigned perSendThreadMessageCount = 100u;
  const unsigned maxSentCount = sendThreadCount * perSendThreadMessageCount;
  std::atomic<unsigned> sentCount{0u};
  Promise<void> promiseEnoughSent;
  SendMessageEnqueue<N> send{socket};
  auto onSent = [&](ErrorCode<N>, I) {
    // We wait for all messages to be enqueued before starting processing them.
    ++sentCount;
    if (sentCount > maxSentCount)
    {
      throw std::runtime_error("Too many messages sent.");
    }
    if (sentCount < maxSentCount) return true; // `true` means "continue processing the message queue".
    promiseEnoughSent.setValue(0);
    return false;
  };
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
          send(Message{}, SslEnabled{false}, onSent);
          std::this_thread::sleep_for(std::chrono::milliseconds{syncRand(rand, gen, randMutex)});
        }
      }
    });
  }
  // Wait a bit for the first send to complete.
  ASSERT_EQ(FutureState_FinishedWithValue, promiseEnoughSent.future().wait());
  // As we requested to stop processing the message queue, only one message
  // should have been sent.
  ASSERT_EQ(maxSentCount, sentCount);
  // Allow detached thread to finish.
  for (auto& t: sendThreads) t.join();
}
