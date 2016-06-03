#include <qi/os.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <qi/application.hpp>
#include <gtest/gtest.h>

TEST(Os, getMachineIdAsUuidRegular)
{
  using namespace qi;
  Uuid u0 = os::getMachineIdAsUuid();
  for (int i = 0; i != 100; ++i) // yeah, 100.
  {
    Uuid u1 = os::getMachineIdAsUuid();
    EXPECT_EQ(u0, u1);
  }
}

TEST(Os, getMachineIdAsUuidCoherentWithGetMachineId)
{
  using namespace qi;
  std::string u0 = to_string(os::getMachineIdAsUuid());
  std::string u1 = os::getMachineId();
  EXPECT_EQ(u0, u1);
}

TEST(Os, getProcessUuidRegular)
{
  using namespace qi;
  Uuid u0 = os::getProcessUuid();
  for (int i = 0; i != 100; ++i)
  {
    Uuid u1 = os::getProcessUuid();
    EXPECT_EQ(u0, u1);
  }
}

int main(int argc, char **argv)
{
  qi::Application app(argc, argv);
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
