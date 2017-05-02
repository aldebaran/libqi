#include <gtest/gtest.h>
#include <qi/application.hpp>
#include <qi/log.hpp>

int main(int argc, char **argv)
{
  qi::Application app(argc, argv);
  ::testing::InitGoogleTest(&argc, argv);
  int res = RUN_ALL_TESTS();
  qi::log::addFilter("qimessaging.*", qi::LogLevel_Debug);
  return res;
}
