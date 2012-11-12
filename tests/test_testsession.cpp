/*
**
** Author(s):
**  - Pierre Roullon <proullon@aldebaran-robotics.com>
**
** Copyright (C) 2012 Aldebaran Robotics
*/

#include <iostream>
#include <gtest/gtest.h>
#include <qi/os.hpp>
#include <testsession/testsessionpair.hpp>

#include <qi/application.hpp>
#include <qimessaging/session.hpp>
#include <qitype/genericobject.hpp>
#include <qitype/genericobjectbuilder.hpp>

int incr(int val)
{
  return val + 1;
}

TEST(TestTestSession, TestTestMode)
{
  // Setenv different conf and check getTestMode answer
  qi::os::setenv(ENVIRON_VARIABLE, "direct");
  ASSERT_EQ(TestMode::Mode_Direct, TestMode::getTestMode());

  qi::os::setenv(ENVIRON_VARIABLE, "sd");
  ASSERT_EQ(TestMode::Mode_SD, TestMode::getTestMode());

  qi::os::setenv(ENVIRON_VARIABLE, "gateway");
  ASSERT_EQ(TestMode::Mode_Gateway, TestMode::getTestMode());

  qi::os::setenv(ENVIRON_VARIABLE, "remotegateway");
  ASSERT_EQ(TestMode::Mode_RemoteGateway, TestMode::getTestMode());

  qi::os::setenv(ENVIRON_VARIABLE, "reversegateway");
  ASSERT_EQ(TestMode::Mode_ReverseGateway, TestMode::getTestMode());

  qi::os::setenv(ENVIRON_VARIABLE, "nightmare");
  ASSERT_EQ(TestMode::Mode_Nightmare, TestMode::getTestMode());

  qi::os::setenv(ENVIRON_VARIABLE, "file");
  ASSERT_EQ(TestMode::Mode_NetworkMap, TestMode::getTestMode());

  // Setenv nothin
  qi::os::setenv(ENVIRON_VARIABLE, "");
  ASSERT_NE(0, TestMode::getTestMode());

  // Setenv bullshit
  qi::os::setenv(ENVIRON_VARIABLE, "plafbim");
  ASSERT_ANY_THROW(TestMode::getTestMode());
}

TEST(TestTestSession, TestForceMode)
{
  // Setenv different conf and check getTestMode answer
  TestMode::forceTestMode(TestMode::Mode_Direct);
  ASSERT_EQ(TestMode::Mode_Direct, TestMode::getTestMode());

  TestMode::forceTestMode(TestMode::Mode_SD);
  ASSERT_EQ(TestMode::Mode_SD, TestMode::getTestMode());

  TestMode::forceTestMode(TestMode::Mode_Gateway);
  ASSERT_EQ(TestMode::Mode_Gateway, TestMode::getTestMode());

  TestMode::forceTestMode(TestMode::Mode_RemoteGateway);
  ASSERT_EQ(TestMode::Mode_RemoteGateway, TestMode::getTestMode());

  TestMode::forceTestMode(TestMode::Mode_ReverseGateway);
  ASSERT_EQ(TestMode::Mode_ReverseGateway, TestMode::getTestMode());

  TestMode::forceTestMode(TestMode::Mode_Nightmare);
  ASSERT_EQ(TestMode::Mode_Nightmare, TestMode::getTestMode());

  TestMode::forceTestMode(TestMode::Mode_NetworkMap);
  ASSERT_EQ(TestMode::Mode_NetworkMap, TestMode::getTestMode());
}

TEST(TestTestSession, TestTestSessionOnly)
{
  int i = 0;
  qi::ServiceDirectory sd;
  std::stringstream    listenUrl;

  // #1 Force test mode to service directory.
  TestMode::forceTestMode(TestMode::Mode_SD);

  // #2 Initialize service directory and test sessions.
  sd.listen("tcp://0.0.0.0:0");
  TestSession          client(sd.endpoints()[0].str(), false);
  TestSession          server(sd.endpoints()[0].str(), true, TestMode::getTestMode());
  server.session()->listen("tcp://0.0.0.0:0");

  // #3 Build a dumb service
  qi::GenericObjectBuilder ob;
  ob.advertiseMethod<int (int)>("++", &incr);

  // #3.1 Get instance of object.
  qi::ObjectPtr obj(ob.object());

  // #4 Bind service on server session.
  ASSERT_TRUE(server.session()->registerService("test", obj).wait());

  // #5 Get proxy on 'test' service.
  qi::ObjectPtr proxy = client.session()->service("test");
  ASSERT_NE((void *) 0, proxy.get());

  // #6 Make a call and assert result is 1.
  i = proxy->call<int>("++", i);
  ASSERT_EQ(1, i);
}

TEST(TestTestSession, TestTestSessionPair)
{
  TestMode::forceTestMode(TestMode::Mode_SD);

  TestSessionPair pSD;
  TestSessionPair pDirect(TestMode::Mode_Direct);
  int iSD = 0, iDirect = 0;

  // #1 Assert that sessions are allocated.
  ASSERT_NE((qi::Session *) 0, pSD.client());
  ASSERT_NE((qi::Session *) 0, pSD.server());
  ASSERT_NE((qi::Session *) 0, pDirect.client());
  ASSERT_NE((qi::Session *) 0, pDirect.server());
  ASSERT_NE(pSD.client(), pSD.server());
  ASSERT_EQ(pDirect.client(), pDirect.server());

  // #1 Build a dumb service
  qi::GenericObjectBuilder ob;
  ob.advertiseMethod<int (int)>("++", &incr);

  // #1.1 Get instance of object
  qi::ObjectPtr obj(ob.object());

  // #2 Bind object on server sessions.
  pSD.server()->registerService("IncrSD", obj);
  pDirect.server()->registerService("IncrDirect", obj);

  // #3 Get proxy on services.
  qi::ObjectPtr proxyDirect = pDirect.client()->service("IncrDirect");
  qi::ObjectPtr proxySD = pSD.server()->service("IncrSD");

  ASSERT_NE((void *) 0, proxyDirect.get());
  ASSERT_NE((void *) 0, proxySD.get());

  // #4 Call ++ method from Incr service.
  iSD = proxySD->call<int>(std::string("++"), iSD);
  iDirect = proxyDirect->call<int>(std::string("++"), iDirect);

  ASSERT_EQ(1, iSD);
  ASSERT_EQ(1, iDirect);
}

TEST(TestTestSession, TestSameObject)
{
  TestSessionPair p(TestMode::Mode_Direct);
  int i = 0;

  // #1 Build a dumb service
  qi::GenericObjectBuilder ob;
  ob.advertiseMethod<int (int)>("++", &incr);

  // #1.1 Get instance of object
  qi::ObjectPtr obj(ob.object());

  // #2 Bind object on server session;
  unsigned int id = p.server()->registerService("Incr", obj);
  p.server()->waitForServiceReady("Incr");
  ASSERT_NE((unsigned int) 0, id);

  // #3 Get proxy to Incr service.
  // #3.2 Assert that client and server are same object.
  ASSERT_EQ(p.client(), p.server());

  qi::ObjectPtr proxy = p.client()->service("Incr");
  ASSERT_NE((void *) 0, proxy.get());

  // #4 Call ++ method from Incr service.
  i = proxy->call<int>(std::string("++"), i);
  ASSERT_EQ(1, i);
}

TEST(TestTestSession, TestThroughSD)
{
  TestSessionPair p;
  int i = 0;

  // #1 Build a dumb service
  qi::GenericObjectBuilder ob;
  ob.advertiseMethod<int (int)>("++", &incr);

  // #1.1 Get instance of object
  qi::ObjectPtr obj(ob.object());

  // #2.1 Get server session from TestSession.
  ASSERT_NE((qi::Session *) 0, p.server());

  // #2.2 Bind object to session.
  unsigned int id = p.server()->registerService("Incr", obj);
  p.server()->waitForServiceReady("Incr");
  ASSERT_NE((unsigned int) 0, id);

  // #3.1 Get client session from TestSession.
  ASSERT_NE((qi::Session *) 0, p.client());

  // #3.2 Assert server and client are different objects.
  ASSERT_NE(p.server(), p.client());

  // #3.3 Get proxy on Incr service
  qi::ObjectPtr proxy = p.client()->service("Incr");
  ASSERT_NE((void *) 0, proxy.get());

  // #4 Call ++ method from Incr service.
  i = proxy->call<int>(std::string("++"), i);

  // #5 No need for Cleanup !
  ASSERT_EQ(1, i);
}

#if 0

TEST(TestTestSession, TestFile)
{
  // Setenv ENVIRON_VARIABLE to file
  // Unsetenv FILEPATH
  // Setenv bullshit
  // Setenv /dev/null
  // Setenv /dev/random
  // Setenv crapy file
  // Setenv good file
  ASSERT_TRUE(false);
}

TEST(TestTestSession, TestThroughSDSpecifyAddress)
{
  // Check 2/ giving valid sd address
  ASSERT_TRUE(false);
}

TEST(TestTestSession, TestThroughSDInvalidAddress)
{
  // Check 3/ giving fake sd address
  ASSERT_TRUE(false);
}

TEST(TestTestSession, TestThroughGateways)
{
  // Check 3/
  // Check 3/ with number of gateways == 0
  // Check 3/ with number of gateways > 0
  ASSERT_TRUE(false);
}

TEST(TestTestSession, TestRemoteGateways)
{
  // Check 4/
  ASSERT_TRUE(false);
}

TEST(TestTestSession, TestCrossLanguages)
{
  // Check 5/ with C++
  // Check 5/ with Python
  // Check 5/ with C#
  // Check 5/ with C
  // (there is another test for Launcher itself, see test_testlauncher)
  ASSERT_TRUE(false);
}

TEST(TestTestSession, TestCrossLanguageThroughGateway)
{
  // Check 6/
  ASSERT_TRUE(false);
}
#endif

#if 0 // Mandatory : Test is done in Aldebaran's LAN and windows/mac vm are up.

TEST(TestTestSession, TestCrossPlatform)
{
  // Check 7/
  ASSERT_TRUE(false);
}

TEST(TestTestSession, TestCrossPlatformCrossLanguage)
{
  // Check 8/
  ASSERT_TRUE(false);
}

TEST(TestTestSession, TestSameObject)
{
  // Check 1/
  ASSERT_TRUE(false);
}

TEST(TestTestSession, TestSameObject)
{
  // Check 1/
  ASSERT_TRUE(false);
}

#endif

int main(int argc, char **argv) {
  qi::Application app(argc, argv);
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
