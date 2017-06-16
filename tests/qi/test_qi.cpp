#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <qi/application.hpp>
#include <qi/log.hpp>
#include <qi/path.hpp>

std::string binDir;
std::string loopBinDir;

int main(int argc, char **argv)
{
  qi::Application app(argc, argv);
  binDir = qi::path::findBin("testlaunch");
  loopBinDir = qi::path::findBin("testlaunchloop");

  qi::log::setLogLevel(qi::LogLevel_Info);
  qi::log::addFilter("qi.*", qi::LogLevel_Debug);

  ::testing::InitGoogleMock(&argc, argv);
  int res = RUN_ALL_TESTS();
  return res;
}
