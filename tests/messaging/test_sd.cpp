/*
**
** Copyright (C) 2012 Aldebaran Robotics
*/

#include <gtest/gtest.h>
#include <qi/application.hpp>
#include <qi/log.hpp>
#include <qi/session.hpp>

TEST(ServiceDirectory, DoubleListen)
{
  qi::Session sd;

  try {
    sd.listenStandalone("tcp://127.0.0.1:0");
    sd.listen("tcp://127.0.0.1:0");
  }
  catch(std::runtime_error& e)
  {
    qiLogError("test_sd") << e.what();
    ASSERT_TRUE(false);
  }
}

struct Serv
{
  int f()
  {
    return 4242;
  }
};
QI_REGISTER_OBJECT(Serv, f);

TEST(ServiceDirectory, MultiRegister)
{
  qi::Session sd1;
  qi::Session sd2;

  sd1.listenStandalone("tcp://127.0.0.1:0");
  sd2.listenStandalone("tcp://127.0.0.1:0");

  sd1.registerService("Serv", boost::make_shared<Serv>());
  sd2.registerService("Serv", boost::make_shared<Serv>());

  {
    qi::Session client;
    client.connect(sd1.url());
    ASSERT_EQ(4242, client.service("Serv").value().call<int>("f"));
  }
  {
    qi::Session client;
    client.connect(sd2.url());
    ASSERT_EQ(4242, client.service("Serv").value().call<int>("f"));
  }
}

TEST(ServiceDirectory, Republish)
{
  qi::Session sd1;
  qi::Session sd2;

  sd1.listenStandalone("tcp://127.0.0.1:0");
  sd2.listenStandalone("tcp://127.0.0.1:0");

  sd1.registerService("Serv", boost::make_shared<Serv>());

  sd2.registerService("Serv", sd1.service("Serv"));

  ASSERT_EQ(4242, sd1.service("Serv").value().call<int>("f"));
  ASSERT_EQ(4242, sd2.service("Serv").value().call<int>("f"));
  {
    qi::Session client;
    client.connect(sd1.url());
    ASSERT_EQ(4242, client.service("Serv").value().call<int>("f"));
  }
  {
    qi::Session client;
    client.connect(sd2.url());
    ASSERT_EQ(4242, client.service("Serv").value().call<int>("f"));
  }
}

int main(int argc, char **argv) {
  qi::Application app(argc, argv);
  ::testing::InitGoogleTest(&argc, argv);

  return RUN_ALL_TESTS();
}

