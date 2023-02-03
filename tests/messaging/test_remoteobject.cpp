#include <chrono>
#include <future>
#include <mutex>
#include <gtest/gtest.h>
#include <qi/jsoncodec.hpp>
#include <qi/log.hpp>
#include "../../src/messaging/remoteobject_p.hpp"
#include "../../src/messaging/server.hpp"

qiLogCategory("Test.RemoteObject");

const int usualTimeoutMs = 100;
const std::chrono::milliseconds usualTimeout{usualTimeoutMs};

const qi::Signature returnSignature{"_"};
const std::string name{"plop"};
const qi::Signature parametersSignature{"()"};
const std::string description{"let us plop!"};
const qi::MetaMethodParameterVector parameters;
const std::string returnDescription;

qi::MetaMethodBuilder makeMetaMethodBuilder()
{
  qi::MetaMethodBuilder mmb;
  mmb.setName(name);
  mmb.setDescription(description);
  mmb.setParametersSignature(parametersSignature);
  mmb.setReturnSignature(returnSignature);
  mmb.setReturnDescription(returnDescription);
  return mmb;
}

class RemoteObject: public testing::Test
{
public:
  void SetUp() override
  {
    using namespace qi;
    // A server to receive socket connections
    server.listen("tcp://127.0.0.1:12121");
    server.newConnection.connect([this](const std::pair<MessageSocketPtr, Url>& socketUrl) mutable
    {
      qiLogInfo() << "A client connects, let's track its messages";
      assert(!serverSocket);
      serverSocket = socketUrl.first;
      serverSocket->messageReady.connect([this](const Message& message) mutable
      {
        qiLogInfo() << "Message received: " << message;
        std::lock_guard<std::mutex> lock{_mutex};
        std::promise<Message> promise;
        std::swap(promise, _nextClientToServerMessagePromise);
        promise.set_value(message);
      });

      // At this level, we have to do it manually!
      serverSocket->ensureReading();

      // We are ready to track messages!
      _messagesTrackedPromise.set_value();
    });

    // Prepare a client socket. The test fails whenever it is disconnected.
    clientSocket = makeMessageSocket();
    clientSocket->connect(toUrl(server.endpoints()[0]));
    ASSERT_TRUE(clientSocket->isConnected());
    _clientDisconnectionLink =
        clientSocket->disconnected.connect([](const std::string& what)
    {
      qiLogInfo() << "Client has disconnected unexpectedly: " << what;
      FAIL();
    }).setCallType(MetaCallType_Direct);
    ASSERT_TRUE(isValidSignalLink(_clientDisconnectionLink));

    // Wait for messages to be tracked before executing tests
    auto status = _messagesTrackedPromise.get_future().wait_for(usualTimeout);
    ASSERT_EQ(std::future_status::ready, status);
  }

  void TearDown() override
  {
    clientSocket->disconnected.disconnect(_clientDisconnectionLink);
    clientSocket->disconnect();
    server.close();
  }

  /// Track the next message received
  std::future<qi::Message> nextClientToServerMessage()
  {
    std::lock_guard<std::mutex> lock{_mutex};
    return _nextClientToServerMessagePromise.get_future();
  }

  qi::TransportServer server;
  qi::MessageSocketPtr serverSocket;
  qi::MessageSocketPtr clientSocket;

private:
  std::promise<void> _messagesTrackedPromise;
  std::promise<qi::Message> _nextClientToServerMessagePromise;
  std::mutex _mutex;
  qi::SignalLink _clientDisconnectionLink = qi::SignalBase::invalidSignalLink;
};

TEST_F(RemoteObject, CallingAVoidMethodSendsACallMessage)
{
  const unsigned int serviceId = 24u;
  const unsigned int methodId = 42u;

  qi::MetaObjectBuilder mob;
  auto mmb = makeMetaMethodBuilder();
  mob.addMethod(mmb, static_cast<int>(methodId)); // KLUDGE: meta object have int for method indexes!

  auto remoteObject = qi::RemoteObject::makePtr(serviceId);
  remoteObject->setMetaObject(mob.metaObject());
  remoteObject->setTransportSocket(clientSocket);

  auto futureMessage = nextClientToServerMessage(); // get ready to receive messages

  // Send a call message from the client to the server
  remoteObject->metaCall(qi::AnyObject{}, methodId, qi::GenericFunctionParameters{});

  auto status = futureMessage.wait_for(usualTimeout);
  ASSERT_EQ(std::future_status::ready, status);
  auto message = futureMessage.get();

  EXPECT_NE(0u, message.id());
  EXPECT_EQ(0u, message.buffer().size());
  EXPECT_EQ(qi::Message::Type_Call, message.type());
  EXPECT_EQ(serviceId, message.address().serviceId);
  EXPECT_EQ(qi::Message::GenericObject_Main, message.address().objectId);
  EXPECT_EQ(methodId, message.address().functionId);
}

