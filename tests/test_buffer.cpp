/*
 *  Author(s):
 *  - Pierre Roullon <proullon@aldebaran-robotics.com>
 *
 *  Copyright (c) 2012 Aldebaran Robotics. All rights reserved.
 */

#include <cstdlib>
#include <string>

#include <gtest/gtest.h>

#include <qi/qi.hpp>
#include <qi/buffer.hpp>


TEST(TestBuffer, TestReserveSpace)
{
  qi::Buffer      buffer;
  unsigned char   *image, *resultImage;
  int             fiveM = 5242880;
  void           *reservedSpace1;
  std::string     str("Oh man, this is a super config file, check it out !");

  // let's put a string in buffer
  reservedSpace1 = buffer.reserve(150);
  //Oh wait it's a config file !
  ASSERT_TRUE(buffer.reserve(1024) != NULL);

  image = new unsigned char [fiveM];
  srand(static_cast<unsigned int>(time(NULL)));
  for (int i = 0; i < fiveM; i++)
    image[i] = rand() % 256;

  buffer.write(image, fiveM);
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
