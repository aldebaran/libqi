#include <gtest/gtest.h>
#include <qi/log.hpp>
#include <cstring>

TEST(log, logasync)
{
  qi::log::init(qi::log::info, 0, false);
  atexit(qi::log::destroy);

   for (int i = 0; i < 1000; i++)
     qiLogFatal("core.log.test1", "%d\n", i);
}
