/*
**
** Copyright (C) 2012 Aldebaran Robotics
*/

#include <gtest/gtest.h>
#include <qi/application.hpp>
#include <qi/log.hpp>
#include <qimessaging/session.hpp>

TEST(Module, pass_obj)
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

int main(int argc, char **argv) {
  qi::Application app(argc, argv);
  ::testing::InitGoogleTest(&argc, argv);

  return RUN_ALL_TESTS();
}

