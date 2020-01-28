/*
** Copyright (C) 2010, 2016 Aldebaran Robotics
*/

#include <gtest/gtest.h>

#include <algorithm>
#include <qi/ptruid.hpp>
#include <qi/os.hpp>
#include <ka/conceptpredicate.hpp>
#include <qi/application.hpp>
#include <boost/core/ignore_unused.hpp>
#include <unordered_map>

TEST(PtrUid, Regular)
{
  using namespace qi;

  auto ptrUid = [](std::uint8_t first) {
    PtrUid p;
    auto b = begin(p);
    *b = first;
    std::fill(++b, end(p), 0);
    return p;
  };
  boost::ignore_unused(ptrUid);

  EXPECT_TRUE(ka::is_regular(ka::bounded_range(ptrUid(0), ptrUid(10), [](qi::PtrUid& x) {
    ++(*begin(x));
  })));
}

TEST(PtrUid, ZeroInitializedDefaultConstruction)
{
  using namespace qi;
  qi::PtrUid p;
  auto* b = reinterpret_cast<unsigned char*>(&p);
  std::fill(b, b + sizeof(PtrUid), 0xCA); // Fill with CA CA...
  new (&p) PtrUid(); // Construct a new PtrUid over the trashed one.
  for (auto byte: p)
  {
    EXPECT_EQ(0, byte);
  }
}

TEST(PtrUid, SameValueFromSamePointer)
{
  using namespace qi;
  const auto& muid = os::getMachineIdAsUuid();
  const auto& puid = os::getProcessUuid();
  const int i{0};
  PtrUid id0(muid, puid, &i);
  PtrUid id1(muid, puid, &i);
  EXPECT_EQ(id0, id1);
}

TEST(PtrUid, DifferentValuesFromDifferentPointers)
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

TEST(PtrUid, Sequence)
{
  using namespace qi;
  const auto& muid = os::getMachineIdAsUuid();
  const auto& puid = os::getProcessUuid();
  const int i{0};
  const qi::PtrUid id0(muid, puid, &i);
  qi::PtrUid id1(muid, puid, &i);
  static const size_t sizeOfPtrUid = 20u;
  static const std::ptrdiff_t sizeOfPtrUidDiff = 20;
  EXPECT_EQ(sizeOfPtrUidDiff, end(id0) - begin(id0));
  EXPECT_EQ(sizeOfPtrUidDiff, end(id1) - begin(id1));
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

namespace
{
  template<typename T>
  std::uintptr_t to_uintptr(T* t)
  {
    return reinterpret_cast<std::uintptr_t>(t);
  }
} // namespace

TEST(PtrUid, SequenceUInt32)
{
  using namespace qi;
  const auto& muid = os::getMachineIdAsUuid();
  const auto& puid = os::getProcessUuid();
  const int i{0};
  static const std::ptrdiff_t nbUInt32InPtrUid = 5;
  qi::PtrUid id(muid, puid, &i);
  EXPECT_EQ(nbUInt32InPtrUid, endUInt32(id) - beginUInt32(id));
  EXPECT_EQ(nbUInt32InPtrUid, endUInt32(id) - beginUInt32(id));
  {
    const auto begin8 = &*begin(id);
    const auto begin32 = &*beginUInt32(id);
    EXPECT_EQ(to_uintptr(begin8), to_uintptr(begin32));

    const auto end8 = begin8 + size(id);
    const auto end32 = begin32 + (size(id) / sizeof(std::uint32_t));
    EXPECT_EQ(to_uintptr(end8), to_uintptr(end32));

    EXPECT_EQ(end8 - begin8, end(id) - begin(id));
    EXPECT_EQ(end32 - begin32, endUInt32(id) - beginUInt32(id));
  }
}

TEST(PtrUid, Extraction)
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

TEST(PtrUid, StdHash)
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

TEST(PtrUid, StdUnorderedMap)
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
