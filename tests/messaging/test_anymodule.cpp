/*
** Copyright (C) 2012 Aldebaran Robotics
*/

#include <gtest/gtest.h>

#include <qi/application.hpp>

#include <qi/anyobject.hpp>
#include <qi/session.hpp>
#include <qi/anymodule.hpp>
#include <ka/scoped.hpp>

qiLogCategory("qi.test.anymodule");

class Module : public ::testing::Test
{
protected:
  void SetUp() override
  {
    session = qi::makeSession();
    session->listenStandalone("tcp://127.0.0.1:0");
  }
  void TearDown() override
  {
    try
    {
      session->close();
    }
    catch (const std::exception& ex)
    {
      qiLogError() << "Error while closing session: " << ex.what();
    }
    catch (const boost::exception& ex)
    {
      qiLogError() << "Error while closing session: " << boost::diagnostic_information(ex);
    }
    catch (...)
    {
      qiLogError() << "Unknown error while closing session";
    }
    session = nullptr;
  }

  qi::SessionPtr session;
};

TEST_F(Module, Load)
{
  session->loadService("naoqi.testanymodule.test");

  qi::AnyObject o = session->service("test").value();
  ASSERT_TRUE(o);
  int res = o.call<int>("testMethod", 12);
  ASSERT_EQ(13, res);
}

TEST_F(Module, LoadTypeErased)
{
  qi::AnyObject osession = session;
  osession.call<void>("loadServiceRename", "naoqi.testanymodule.test", "test");

  qi::AnyObject o = session->service("test").value();
  ASSERT_TRUE(o);
  int res = o.call<int>("testMethod", 12);
  ASSERT_EQ(13, res);
}

TEST_F(Module, Call)
{
  ASSERT_EQ(84, session->callModule<int>("naoqi.testanymodule.func", 42).value());
}

TEST_F(Module, CallTypeErased)
{
  qi::AnyObject osession = session;
  ASSERT_EQ(84, osession.call<int>("callModule", "naoqi.testanymodule.func", 42));
}

TEST_F(Module, LoadByHandWithSession)
{
  qi::AnyModule foomod = qi::import("naoqi.testanymodulesession");
  qi::AnyObject ao = foomod.call<qi::AnyObject>("Foo", session);
  auto scopeUnregister = ka::scoped(session->registerService("Foo", ao).value(),
                                    [&](unsigned int id) { session->unregisterService(id); });

  int res = ao.call<int>("bar");

  ASSERT_EQ(42, res);
}

TEST_F(Module, LoadWithSessionAndRename)
{
  //## register the Foo object as a service
  session->loadService("naoqi.testanymodulesession.Foo", "Bar");

  qi::AnyObject o = session->service("Bar").value();
  ASSERT_TRUE(o);
  int res = o.call<int>("bar");

  ASSERT_EQ(42, res);
}
