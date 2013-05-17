/*
**
** Copyright (C) 2012 Aldebaran Robotics
*/

#include <gtest/gtest.h>
#include <qi/application.hpp>
#include <qimessaging/servicedirectory.hpp>

TEST(Module, pass_obj)
{
  qi::ServiceDirectory sd;

  try
    {
    sd.listen("tcp://127.0.0.1:9559");
    sd.listen("tcp://127.0.0.1:0");
    }
    catch(std::runtime_error&)
    {
      ASSERT_TRUE(false);
    }
}

int main(int argc, char **argv) {
  qi::Application app(argc, argv);
  ::testing::InitGoogleTest(&argc, argv);

  return RUN_ALL_TESTS();
}

