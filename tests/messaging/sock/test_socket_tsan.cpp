#include <thread>
#include <algorithm>
#include <numeric>
#include <boost/shared_ptr.hpp>
#include <boost/optional.hpp>
#include <gtest/gtest.h>
#include "src/messaging/transportserver.hpp"
#include <qi/future.hpp>
#include "src/messaging/message.hpp"
#include <qi/messaging/net/networkasio.hpp>
#include "networkasionooplock.hpp"
#include "src/messaging/tcpmessagesocket.hpp"
#include "networkcommon.hpp"
#include "networkmock.hpp"
#include <qi/messaging/net/receive.hpp>
#include <qi/messaging/net/accept.hpp>

static const qi::MilliSeconds defaultTimeout{500};
static const std::chrono::milliseconds defaultPostPauseInMs{20};

TEST(NetReceiveMessage, Asio)
{
  using namespace qi;
  using namespace qi::net;
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
