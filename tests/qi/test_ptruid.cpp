/*
** Copyright (C) 2010, 2016 Aldebaran Robotics
*/

#include <qi/ptruid.hpp>
#include <qi/os.hpp>
#include <qi/detail/conceptpredicates.hpp>
#include <qi/application.hpp>
#include <gtest/gtest.h>
#include <unordered_map>

TEST(TestPtrUid, Regular)
{
  using namespace qi;
  const int i{0};
  PtrUid id(os::getMachineIdAsUuid(), os::getProcessUuid(), &i);
  assert(detail::isRegular(id, [](qi::PtrUid& x) {
    ++(*begin(x));
  }));
}

TEST(TestPtrUid, SameValueFromSamePointer)
{
  using namespace qi;
  const auto& muid = os::getMachineIdAsUuid();
  const auto& puid = os::getProcessUuid();
  const int i{0};
  PtrUid id0(muid, puid, &i);
  PtrUid id1(muid, puid, &i);
  EXPECT_EQ(id0, id1);
}

TEST(TestPtrUid, DifferentValuesFromDifferentPointers)
{
  using namespace qi;
  const int i{0};
  const int j{0};
  const auto& muid = os::getMachineIdAsUuid();
  const auto& puid = os::getProcessUuid();
  PtrUid id0(muid, puid, &i);
  PtrUid id1(muid, puid, &j);
  EXPECT_NE(id0, id1);
}

TEST(TestPtrUid, Sequence)
{
  using namespace qi;
  const auto& muid = os::getMachineIdAsUuid();
  const auto& puid = os::getProcessUuid();
  const int i{0};
  const qi::PtrUid id0(muid, puid, &i);
  qi::PtrUid id1(muid, puid, &i);
  static const size_t sizeOfPtrUid = 20u;
  EXPECT_EQ(sizeOfPtrUid, end(id0) - begin(id0));
  EXPECT_EQ(sizeOfPtrUid, end(id1) - begin(id1));
  EXPECT_EQ(sizeOfPtrUid, size(id0));
  EXPECT_EQ(sizeOfPtrUid, size(id1));
  EXPECT_FALSE(empty(id0));
  EXPECT_FALSE(empty(id1));
  size_t n = 0;
  auto b0 = begin(id0);
  auto b1 = begin(id1);
  for (auto byte: id0)
  {
    EXPECT_EQ(byte, id0[n]);
    EXPECT_EQ(byte, *b0);
    EXPECT_EQ(byte, *b1);
    ++n;
    ++b0;
    ++b1;
  }
}

TEST(TestPtrUid, SequenceUInt32)
{
  using namespace qi;
  const auto& muid = os::getMachineIdAsUuid();
  const auto& puid = os::getProcessUuid();
  const int i{0};
  qi::PtrUid id(muid, puid, &i);
  static const size_t nbUInt32InPtrUid = 5u;
  EXPECT_EQ(nbUInt32InPtrUid, endUInt32(id) - beginUInt32(id));
}

TEST(TestPtrUid, Extraction)
{
  using namespace qi;
  const int i{0};
  PtrUid id(os::getMachineIdAsUuid(), os::getProcessUuid(), &i);
  std::stringstream ss;
  ss << id;
  std::string str;
  for (auto byte: id)
  {
    static const char* digits = "0123456789ABCDEF";
    str += digits[(byte >> 4) & 0x0F];
    str += digits[byte & 0x0F];
  }
  EXPECT_EQ(ss.str(), str);
}

TEST(TestPtrUid, StdHash)
{
  using namespace qi;
  std::hash<PtrUid> h;
  const auto& muid = os::getMachineIdAsUuid();
  const auto& puid = os::getProcessUuid();
  const int i{0};
  qi::PtrUid id0(muid, puid, &i);
  EXPECT_EQ(h(id0), h(id0));
  const int j{0};
  qi::PtrUid id1(muid, puid, &j);
  EXPECT_EQ(h(id1), h(id1));
  EXPECT_NE(h(id0), h(id1));
}

TEST(TestPtrUid, StdUnorderedMap)
{
  using namespace qi;
  const auto& muid = os::getMachineIdAsUuid();
  const auto& puid = os::getProcessUuid();
  const int i{0};
  const int j{0};
  const int k{0};
  PtrUid idI(muid, puid, &i), idJ(muid, puid, &j), idK(muid, puid, &k);
  std::unordered_map<PtrUid, std::string> map{
    {idI, "i"},
    {idJ, "j"},
    {idK, "k"}
  };
  EXPECT_EQ("i", map[idI]);
  EXPECT_EQ("j", map[idJ]);
  EXPECT_EQ("k", map[idK]);
}

int main(int argc, char **argv)
{
  qi::Application app(argc, argv);
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
