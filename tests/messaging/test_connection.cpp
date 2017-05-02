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

#include <qi/session.hpp>
#include <qi/anyobject.hpp>
#include <qi/type/dynamicobjectbuilder.hpp>
#include <qi/messaging/gateway.hpp>
#include <qi/application.hpp>
#include <qi/os.hpp>
#include <qi/binarycodec.hpp>

qiLogCategory("test");

static std::string reply(const std::string &msg)
{
  return msg;
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

class Connection: public ::testing::Test
{
protected:
  void SetUp() override
  {
    session.listenStandalone("tcp://127.0.0.1:0");
    connectionAddr = session.endpoints()[0].str();

    std::cout << "Service Directory ready." << std::endl;
    qi::DynamicObjectBuilder ob;
    ob.advertiseMethod("reply", &reply);
    ob.advertiseMethod("replyBuf", &replyBuf);
    ob.advertiseMethod("replyBufA", &replyBufA);
    ob.advertiseMethod("replyBufB", &replyBufB);
    ob.advertiseMethod("replyBufBA", &replyBufBA);

    obj = ob.object();

    unsigned int id = session.registerService("serviceTest", obj);
    std::cout << "serviceTest ready:" << id << std::endl;

  #ifdef WITH_GATEWAY_
    gate.attachToServiceDirectory(sd.endpoints()[0]);
    gate.listen(gatewayAddr.str());
    connectionAddr = gate.endpoints()[0];
  #endif
  }

  void TearDown() override
  {
    session.close();
  }

private:
  qi::Session session;
  qi::AnyObject obj;
#ifdef WITH_GATEWAY_
  static qi::Gateway gate;
#endif
};

#ifdef WITH_GATEWAY_
qi::Gateway Connection::gate = qi::Gateway{};
#endif

TEST_F(Connection, testSyncSendOneMessage)
{
  TestConnection tc;
  ASSERT_TRUE(tc.init());

  std::string result = tc.obj.call<std::string>("reply", "question");
  EXPECT_EQ("question", result);
}

TEST_F(Connection, testSyncSendMessages)
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

TEST_F(Connection, testBuffer)
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
