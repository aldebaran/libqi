/*
 ** Author(s):
 **  - Thomas Fontenay <tfontenay@aldebaran.com>
 **
 ** Copyright (C) 2013 Aldebaran Robotics
 */

#include <iostream>
#include <array>

#include <gtest/gtest.h>

#include <qi/applicationsession.hpp>

static std::string priority;
static char* gav = nullptr;

static void testLowPrio()
{
  const qi::Url lowPrioListen("tcp://127.0.0.1:9558");
  const qi::Url lowPrioUrl("tcp://127.0.0.1:9559");
  const qi::Url middlePrioListen("tcp://127.0.0.1:12344");
  const qi::Url middlePrioUrl("tcp://127.0.0.1:12345");
  const qi::Url highPrioListen("tcp://127.0.0.1:5994");
  const qi::Url highPrioUrl("tcp://127.0.0.1:5995");

  qi::ApplicationSession::Config config;
  config.setListenUrls({ lowPrioListen });
  config.setConnectUrl(lowPrioUrl);

  int ac = 1;
  char** av = &gav;
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
  config.setListenUrls({ lowPrioListen });
  config.setConnectUrl(lowPrioUrl);

  qi::os::setenv("QI_LISTEN_URL", middlePrioListen.str().c_str());
  qi::os::setenv("QI_URL", middlePrioUrl.str().c_str());

  int ac = 1;
  char** av = &gav;
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
  config.setListenUrls({ lowPrioListen });
  config.setConnectUrl(lowPrioUrl);

  std::array<char*, 5> av {
    gav,
    const_cast<char*>("--qi-listen-url"),
    const_cast<char*>(highPrioListen.str().c_str()),
    const_cast<char*>("--qi-url"),
    const_cast<char*>(highPrioUrl.str().c_str()),
  };

  auto ac = static_cast<int>(av.size());
  auto refav = av.data();

  qi::ApplicationSession appsession(ac, refav, config);

  ASSERT_EQ(highPrioUrl.str(), appsession.url().str());
  ASSERT_EQ(highPrioListen.str(), appsession.listenUrl().str());
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
    FAIL() << priority;
}

int main(int ac, char **av)
{
  auto scopedGav = ka::scoped_set_and_restore(gav, av[0]);
  auto scopedPriority = ka::scoped_set_and_restore(priority, av[1]);
  ::testing::InitGoogleTest(&ac, av);

  return RUN_ALL_TESTS();
}
