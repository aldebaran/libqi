#include <gtest/gtest.h>
#include <qi/application.hpp>
#include <qi/log.hpp>
#include <testsession/testsessionpair.hpp>

int main(int argc, char **argv)
{
#if defined(__APPLE__) || defined(__linux__)
  setsid();
#endif
  qi::os::setenv("QI_IGNORE_STRUCT_NAME", "1");
  qi::Application app(argc, argv);
  TestMode::initTestMode(argc, argv);
  TestMode::getTestMode();
  ::testing::InitGoogleTest(&argc, argv);
  int res = RUN_ALL_TESTS();
  qi::log::addFilter("qimessaging.*", qi::LogLevel_Debug);
  return res;
}
