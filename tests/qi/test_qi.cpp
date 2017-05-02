#include <gtest/gtest.h>
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
  qi::log::addFilter("qi.os", qi::LogLevel_Debug);
  ::testing::InitGoogleTest(&argc, argv);
  int res = RUN_ALL_TESTS();
  return res;
}
