/*
** Author(s):
**  - Herve Cuche <hcuche@aldebaran-robotics.com>
**
** Copyright (C) 2010, 2012 Aldebaran Robotics
*/

#include <vector>
#include <iostream>
#include <string>

#include <gtest/gtest.h>

#include <qimessaging/session.hpp>
#include <qimessaging/object.hpp>
#include <qimessaging/server.hpp>
#include <qimessaging/service_directory.hpp>
#include <qimessaging/gateway.hpp>
#include <qi/os.hpp>

static std::string reply(const std::string &msg)
{
  return msg;
}

static std::string connectionAddr;

class TestConnection
{
public:
  TestConnection()
    : obj(NULL)
  {
  }

  ~TestConnection()
  {
    session.disconnect();
    session.waitForDisconnected();

    delete obj;
    obj = NULL;
  }

  bool init()
  {
    session.connect(connectionAddr);
    session.waitForConnected();
    obj = session.service("serviceTest");

    if (!obj)
    {
      qiLogError("test.connection") << "can't get serviceTest" << std::endl;
      return false;
    }

    return true;
  }

public:
  qi::Object *obj;

private:
  qi::Session session;
};

TEST(QiMessagingConnexion, testSyncSendOneMessage)
{
  TestConnection tc;
  EXPECT_TRUE(tc.init());

  std::string result = tc.obj->call<std::string>("reply", "question");
  EXPECT_EQ("question", result);
}

TEST(QiMessagingConnexion, testSyncSendMessages)
{
  TestConnection tc;
  EXPECT_TRUE(tc.init());

  std::string result = tc.obj->call<std::string>("reply", "question1");
  EXPECT_EQ("question1", result);
  result = tc.obj->call<std::string>("reply", "question2");
  EXPECT_EQ("question2", result);
  result = tc.obj->call<std::string>("reply", "question3");
  EXPECT_EQ("question3", result);
}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);

  qi::ServiceDirectory sd;

  unsigned int sdPort = qi::os::findAvailablePort(5555);
  std::stringstream sdAddr;
  sdAddr << "tcp://127.0.0.1:" << sdPort;

  connectionAddr = sdAddr.str();

  sd.listen(sdAddr.str());
  std::cout << "Service Directory ready." << std::endl;

  qi::Session       session;
  qi::Object        obj;
  qi::Server        srv;
  obj.advertiseMethod("reply", &reply);

  session.connect(sdAddr.str());
  session.waitForConnected();

  std::vector<std::string> endpoints;
  unsigned int servicePort = qi::os::findAvailablePort(9559);
  std::stringstream serviceAddr;
  serviceAddr << "tcp://127.0.0.1:" << servicePort;

  endpoints.push_back(serviceAddr.str());
  srv.listen(&session, endpoints);
  srv.registerService("serviceTest", &obj);
  std::cout << "serviceTest ready." << std::endl;

  qi::os::msleep(400);

#ifdef WITH_GATEWAY_
  unsigned int gatewayPort = qi::os::findAvailablePort(12345);
  std::stringstream gatewayAddr;
  gatewayAddr << "tcp://127.0.0.1:" << gatewayPort;

  connectionAddr = gatewayAddr.str();

  qi::Gateway gate;
  gate.listen(gatewayAddr.str(), sdAddr.str());
#endif

  int res = RUN_ALL_TESTS();

  srv.stop();
  session.disconnect();
  session.waitForDisconnected();

  return res;
}
