#include <gtest/gtest.h>
#include <qi/application.hpp>

int main(int argc, char **argv)
{
  qi::Application app(argc, argv);
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
