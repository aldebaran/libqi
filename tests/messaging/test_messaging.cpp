#include <gtest/gtest.h>
#include <qi/application.hpp>
#include <qi/log.hpp>
#include <qi/path.hpp>

std::string simpleSdPath;
std::string mirrorSdPath;

int main(int argc, char **argv)
{
#if defined(__APPLE__) || defined(__linux__)
  setsid();
#endif
  qi::Application app(argc, argv);
  ::testing::InitGoogleTest(&argc, argv);
  qi::log::addFilter("qimessaging.*", qi::LogLevel_Debug);
  simpleSdPath = qi::path::findBin("simplesd");
  mirrorSdPath = qi::path::findBin("mirrorsd");
  int res = RUN_ALL_TESTS();
  return res;
}
