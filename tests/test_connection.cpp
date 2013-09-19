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
#include <qitype/anyobject.hpp>
#include <qitype/dynamicobjectbuilder.hpp>
#include <qimessaging/servicedirectory.hpp>
#include <qimessaging/gateway.hpp>
#include <qi/application.hpp>
#include <qi/os.hpp>
#include <qitype/binarycodec.hpp>

qiLogCategory("test");

static std::string reply(const std::string &msg)
{
  return msg;
}

static std::string connectionAddr;

class TestConnection
{
public:
  TestConnection()
    : obj()
  {
  }

  ~TestConnection()
  {
    session.close();
  }

  bool init()
  {
    session.connect(connectionAddr);
    obj = session.service("serviceTest");

    if (!obj)
    {
      qiLogError() << "can't get serviceTest" << std::endl;
      return false;
    }

    return true;
  }

public:
  qi::AnyObject obj;

private:
  qi::Session session;
};

TEST(QiMessagingConnexion, testSyncSendOneMessage)
{
  TestConnection tc;
  ASSERT_TRUE(tc.init());

  std::string result = tc.obj.call<std::string>("reply", "question");
  EXPECT_EQ("question", result);
}

TEST(QiMessagingConnexion, testSyncSendMessages)
{
  TestConnection tc;
  ASSERT_TRUE(tc.init());

  std::string result = tc.obj.call<std::string>("reply", "question1");
  EXPECT_EQ("question1", result);
  result = tc.obj.call<std::string>("reply", "question2");
  EXPECT_EQ("question2", result);
  result = tc.obj.call<std::string>("reply", "question3");
  EXPECT_EQ("question3", result);
}

static qi::Buffer replyBufBA(const unsigned int&, const qi::Buffer& arg, const int&)
{
  return arg;
}
static qi::Buffer replyBufB(const int&, const qi::Buffer& arg)
{
  std::cerr <<"B " << arg.size() << std::endl;
  return arg;
}
static qi::Buffer replyBufA(const qi::Buffer& arg, const int&)
{
  std::cerr <<"A " << arg.size() << std::endl;
  return arg;
}
static qi::Buffer replyBuf(const qi::Buffer& arg)
{
  return arg;
}

TEST(QiMessagingConnexion, testBuffer)
{
  TestConnection tc;
  ASSERT_TRUE(tc.init());
  qi::Buffer buf;
  std::string challenge = "foo*******************************";
  qi::encodeBinary(&buf, challenge);
  qiLogDebug() << "call BA";
  qi::Buffer result = tc.obj.call<qi::Buffer>("replyBufBA", (unsigned int)1, buf, 2);
  std::string reply;
  qi::BufferReader br(result);
  qi::decodeBinary(&br, &reply);
  ASSERT_EQ(challenge, reply);
  qiLogDebug() << "call BA";
  result = tc.obj.call<qi::Buffer>("replyBufBA", (unsigned int)2, buf, 1);
  {
    std::string reply;
    qi::BufferReader br(result);
    qi::decodeBinary(&br, &reply);
    ASSERT_EQ(challenge, reply);
  }
  qiLogDebug() << "call A";
  result = tc.obj.call<qi::Buffer>("replyBufA", buf, 1);
  {
    std::string reply;
    qi::BufferReader br(result);
    qi::decodeBinary(&br, &reply);
    ASSERT_EQ(challenge, reply);
  }
  result = tc.obj.call<qi::Buffer>("replyBuf", buf);
   {
    std::string reply;
    qi::BufferReader br(result);
    qi::decodeBinary(&br, &reply);
    ASSERT_EQ(challenge, reply);
  }
  result = tc.obj.call<qi::Buffer>("replyBufB", 1, buf);
  {
    std::string reply;
    qi::BufferReader br(result);
    qi::decodeBinary(&br, &reply);
    ASSERT_EQ(challenge, reply);
  }
}

int main(int argc, char **argv) {
  qi::Application app(argc, argv);
  ::testing::InitGoogleTest(&argc, argv);
  qi::ServiceDirectory sd;

  sd.listen("tcp://127.0.0.1:0");
  connectionAddr = sd.endpoints()[0].str();

  std::cout << "Service Directory ready." << std::endl;
  qi::Session       session;
  qi::DynamicObjectBuilder ob;
  ob.advertiseMethod("reply", &reply);
  ob.advertiseMethod("replyBuf", &replyBuf);
  ob.advertiseMethod("replyBufA", &replyBufA);
  ob.advertiseMethod("replyBufB", &replyBufB);
  ob.advertiseMethod("replyBufBA", &replyBufBA);
  qi::AnyObject        obj(ob.object());

  session.connect(connectionAddr);

  session.listen("tcp://127.0.0.1:0");
  unsigned int id = session.registerService("serviceTest", obj);
  std::cout << "serviceTest ready:" << id << std::endl;

#ifdef WITH_GATEWAY_

  qi::Gateway gate;
  gate.attachToServiceDirectory(sd.endpoints()[0]);
  gate.listen(gatewayAddr.str());
  connectionAddr = gate.endpoints()[0];
#endif

  int res = RUN_ALL_TESTS();
  sd.close();
  session.close();


  return res;
}
