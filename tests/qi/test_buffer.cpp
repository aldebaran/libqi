/*
 *  Author(s):
 *  - Pierre Roullon <proullon@aldebaran-robotics.com>
 *
 *  Copyright (c) 2012 Aldebaran Robotics. All rights reserved.
 */

#include <cstdlib>
#include <string>
#include <vector>
#include <algorithm>
#include <random>
#include <numeric> // std::iota

#include <gtest/gtest.h>

#include <qi/buffer.hpp>
#include <qi/numeric.hpp>

#include <ka/range.hpp>
#include <ka/relationpredicate.hpp>

TEST(TestBuffer, IsEqualityComparable)
{
  const char data[] = "I like cookies.";
  const char data2[] = "Donuts are good too.";
  qi::Buffer buffers[4];
  auto& a = buffers[0];
  auto& b = buffers[1];
  auto& c = buffers[2];
  auto& d = buffers[3];

  a.write(data, sizeof(data));
  b.write(data2, sizeof(data2));
  c.addSubBuffer(b);
  d.addSubBuffer(a);
  d.addSubBuffer(c);

  auto range = ka::bounded_range(buffers);
  EXPECT_TRUE(ka::is_equivalence(std::equal_to<qi::Buffer>{}, range));
  EXPECT_TRUE(ka::are_complement(std::equal_to<qi::Buffer>{}, std::not_equal_to<qi::Buffer>{}, range));
}


TEST(TestBuffer, TestReserveSpace)
{
  qi::Buffer      buffer;
  unsigned char   *resultImage;
  int             fiveM = 5242880;
  void           *reservedSpace1;
  std::string     str("Oh man, this is a super config file, check it out !");

  auto randEngine = [] {
    std::random_device rd;
    std::seed_seq seq{ rd(), rd(), rd(), rd(), rd(), rd(), rd(), rd() };
    return std::default_random_engine{ seq };
  }();
  std::uniform_int_distribution<int> ucDistrib{
    std::numeric_limits<unsigned char>::min(),
    std::numeric_limits<unsigned char>::max()
  };

  // let's put a string in buffer
  reservedSpace1 = buffer.reserve(150);
  ASSERT_NE(nullptr, reservedSpace1);
  //Oh wait it's a config file !
  ASSERT_NE(nullptr, buffer.reserve(1024));

  std::vector<unsigned char> image(fiveM);
  for (int i = 0; i < fiveM; i++)
    image[i] = qi::numericConvert<unsigned char>(ucDistrib(randEngine));

  buffer.write(image.data(), fiveM);
  resultImage = (unsigned char*)buffer.data() + 150 + 1024;

  for (int i = 0; i < fiveM; i++)
    EXPECT_EQ(image[i], resultImage[i]) << "Original image and actual differ at index " << i;

  ::memset(reservedSpace1, 0, 150);
  ::memcpy(reservedSpace1, str.c_str(), str.size());
}

TEST(TestBuffer, TestSubBuffer)
{
  qi::Buffer buffer, subBuffer1, subBuffer2;
  std::string str("A dummy string");
  size_t cstrSize = str.size()+1;

  const char *data;

  buffer.write(str.c_str(),     cstrSize);
  subBuffer1.write(str.c_str(), cstrSize);
  subBuffer2.write(str.c_str(), cstrSize);

  ASSERT_EQ(buffer.size(),      cstrSize);
  ASSERT_EQ(buffer.totalSize(), cstrSize);

  buffer.addSubBuffer(subBuffer1);

  ASSERT_EQ(buffer.size(),      cstrSize+sizeof(qi::uint32_t));
  ASSERT_EQ(buffer.totalSize(), 2*cstrSize+sizeof(qi::uint32_t));

  ASSERT_FALSE(buffer.hasSubBuffer(0));
  ASSERT_TRUE(buffer.hasSubBuffer(cstrSize));

  const qi::Buffer& subBuf1 = buffer.subBuffer(cstrSize);
  ASSERT_EQ(subBuf1.size(), subBuffer1.size());
  ASSERT_STREQ(str.c_str(), (const char*)subBuf1.data());

  data = (const char*)subBuffer2.read((size_t)0, cstrSize);
  ASSERT_STREQ(str.c_str(), data);

  subBuffer2.addSubBuffer(buffer);
  ASSERT_EQ(subBuffer2.size(),      cstrSize+sizeof(qi::uint32_t));
  ASSERT_EQ(subBuffer2.totalSize(), 3*cstrSize+2*sizeof(qi::uint32_t));

  buffer.clear();
  ASSERT_EQ(buffer.size(), 0u);
  ASSERT_EQ(buffer.totalSize(), 0u);
}

TEST(TestBuffer, TestCopiesAreDistinct)
{
  using namespace qi;
  // Fill the first buffer.
  std::vector<int> v(100);
  std::iota(begin(v), end(v), 993);
  Buffer b0;
  const auto len = v.size() * sizeof(v[0]);
  b0.write(&v[0], len);
  ASSERT_EQ(len, b0.size());
  // Copy it.
  Buffer b1(b0);
  ASSERT_EQ(len, b1.size());
  // Assert that the two buffers have the same content.
  {
    auto f0 = static_cast<unsigned char*>(b0.data());
    auto f1 = static_cast<unsigned char*>(b1.data());
    ASSERT_TRUE(std::equal(f0, f0 + b0.size(), f1));
  }
  // Assert that modifying one copy doesn't affect the other one.
  *static_cast<int*>(b0.data()) = 1234;
  ASSERT_EQ(993, *static_cast<int*>(b1.data()));
}

TEST(TestBuffer, TestAssignmentsAreDistinct)
{
  using namespace qi;
  auto asIntPtr = [](void* x) {
    return static_cast<int*>(x);
  };
  auto fill = [](qi::Buffer& b, int size, int start) {
    std::vector<int> v(size);
    std::iota(begin(v), end(v), start);
    const auto len = v.size() * sizeof(v[0]);
    b.write(&v[0], len);
  };

  // Fill the first buffer.
  Buffer b0;
  fill(b0, 100, 993);

  // Fill the second one with different values.
  Buffer b1;
  fill(b1, 100, 7263);

  // Assign.
  b1 = b0;

  // Assert that the two buffers have the same content.
  {
    ASSERT_EQ(b0.size() , b1.size());
    auto f0 = static_cast<unsigned char*>(b0.data());
    auto f1 = static_cast<unsigned char*>(b1.data());
    ASSERT_TRUE(std::equal(f0, f0 + b0.size(), f1));
  }
  // Assert that modifying one copy doesn't affect the other one.
  *asIntPtr(b0.data()) = 1234;
  ASSERT_EQ(993, *asIntPtr(b1.data()));
}
