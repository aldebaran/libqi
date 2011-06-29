#include <gtest/gtest.h>
#include <qi/log.hpp>
#include <cstring>

TEST(log, overflow)
{
  char dulourd[8000];
  memset(dulourd, 'c', 8000);
  dulourd[8000 - 1] = 0;
  //should not fail
  qi::log::log(qi::log::verbose, dulourd, dulourd, dulourd, dulourd, 42);
}


