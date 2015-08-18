/*
 ** Author(s):
 **  - Thomas Fontenay <tfontenay@aldebaran.com>
 **
 ** Copyright (C) 2013 Aldebaran Robotics
 */

#include <iostream>

#include <gtest/gtest.h>

#include <qi/applicationsession.hpp>

static std::string priority;
static char* gav[] = {NULL};

static void testLowPrio()
{
  const qi::Url lowPrioListen("tcp://127.0.0.1:9558");
  const qi::Url lowPrioUrl("tcp://127.0.0.1:9559");
  const qi::Url middlePrioListen("tcp://127.0.0.1:12344");
  const qi::Url middlePrioUrl("tcp://127.0.0.1:12345");
  const qi::Url highPrioListen("tcp://127.0.0.1:5994");
  const qi::Url highPrioUrl("tcp://127.0.0.1:5995");
  qi::ApplicationSession::Config config;
  config.setDefaultListenUrl(lowPrioListen);
  config.setDefaultUrl(lowPrioUrl);
  int ac = 1;
  char** av = &gav[0];
  qi::ApplicationSession appsession(ac, av, config);


  ASSERT_EQ(lowPrioListen.str(), appsession.listenUrl().str());
  ASSERT_EQ(lowPrioUrl.str(), appsession.url().str());
}

static void testMidPrio()
{
  const qi::Url lowPrioListen("tcp://127.0.0.1:9558");
  const qi::Url lowPrioUrl("tcp://127.0.0.1:9559");
  const qi::Url middlePrioListen("tcp://127.0.0.1:12344");
  const qi::Url middlePrioUrl("tcp://127.0.0.1:12345");
  const qi::Url highPrioListen("tcp://127.0.0.1:5994");
  const qi::Url highPrioUrl("tcp://127.0.0.1:5995");
  qi::ApplicationSession::Config config;
  config.setDefaultListenUrl(lowPrioListen);
  config.setDefaultUrl(lowPrioUrl);
  int ac = 1;
  char** av = &gav[0];
  qi::os::setenv("QI_LISTEN_URL", middlePrioListen.str().c_str());
  qi::os::setenv("QI_URL", middlePrioUrl.str().c_str());
  qi::ApplicationSession appsession(ac, av, config);

  ASSERT_EQ(middlePrioListen.str(), appsession.listenUrl().str());
  ASSERT_EQ(middlePrioUrl.str(), appsession.url().str());
}

static void testHighPrio()
{
  const qi::Url lowPrioListen("tcp://127.0.0.1:9558");
  const qi::Url lowPrioUrl("tcp://127.0.0.1:9559");
  const qi::Url middlePrioListen("tcp://127.0.0.1:12344");
  const qi::Url middlePrioUrl("tcp://127.0.0.1:12345");
  const qi::Url highPrioListen("tcp://127.0.0.1:5994");
  const qi::Url highPrioUrl("tcp://127.0.0.1:5995");
  qi::ApplicationSession::Config config;
  config.setDefaultListenUrl(lowPrioListen);
  config.setDefaultUrl(lowPrioUrl);
  int ac = 5;
  char** av = new char*[5];
  av[0] = gav[0];
  av[1] = qi::os::strdup("--qi-listen-url");
  av[2] = qi::os::strdup(highPrioListen.str().c_str());
  av[3] = qi::os::strdup("--qi-url");
  av[4] = qi::os::strdup(highPrioUrl.str().c_str());

  qi::ApplicationSession appsession(ac, av, config);

  ASSERT_EQ(highPrioUrl.str(), appsession.url().str());
  ASSERT_EQ(highPrioListen.str(), appsession.listenUrl().str());

  delete[] av;
}

TEST(QiApplicationSessionOptions, testOptionPriority)
{

  if (priority == "low")
    testLowPrio();
  else if (priority == "medium")
    testMidPrio();
  else if (priority == "high")
    testHighPrio();
  else
    ASSERT_TRUE(false) << priority;
}

int main(int ac, char **av)
{
  gav[0] = av[0];
  priority = av[1];
  ::testing::InitGoogleTest(&ac, av);

  std::cout << priority << std::endl;

  return RUN_ALL_TESTS();
}
