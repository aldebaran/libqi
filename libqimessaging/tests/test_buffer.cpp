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
#include <qimessaging/datastream.hpp>
#include <qimessaging/buffer.hpp>

void    fillString(std::string &str, int size)
{
  str = "";
  for (int i = 0; i < size; i++)
    str.append("a");
}

TEST(TestNormalStackBuffer, TestBuffer)
{
  qi::Buffer      buffer;
  qi::ODataStream  d(buffer);
  std::string     data, resultString;

  fillString(data, 150);

  d << data;
  ASSERT_EQ(150 + sizeof(int), buffer.size());
  qi::IDataStream dout(buffer);
  dout >> resultString;
  ASSERT_STREQ(data.c_str(), resultString.c_str());
}

TEST(TestFirstAllocation, TestBuffer)
{
  qi::Buffer      buffer;
  qi::ODataStream  d(buffer);
  std::string     data, resultString;

  fillString(data, 252);

  d << data;
  ASSERT_EQ(252 + sizeof(int), buffer.size());
  d << data;
  ASSERT_EQ(504 + 2 * sizeof(int), buffer.size());

  data = "!";
  d << data;
  ASSERT_EQ(505 + 3 * sizeof(int), buffer.size());

  qi::IDataStream dout(buffer);
  fillString(data, 252);
  dout >> resultString;
  ASSERT_STREQ(data.c_str(), resultString.c_str());

  dout >> resultString;
  ASSERT_STREQ(data.c_str(), resultString.c_str());

  dout >> resultString;
  ASSERT_STREQ("!", resultString.c_str());
}

TEST(TestSecondAllocation, TestBuffer)
{
  qi::Buffer      buffer;
  qi::ODataStream  d(buffer);
  std::string     data, resultString;

  fillString(data, 525);

  d << data;
  ASSERT_EQ(525 + sizeof(int), buffer.size());

  fillString(data, 4097);
  d << data;
  ASSERT_EQ(4622 + 2 * sizeof(int), buffer.size());

  qi::IDataStream dout(buffer);
  fillString(data, 525);
  dout >> resultString;
  ASSERT_STREQ(data.c_str(), resultString.c_str());

  fillString(data, 4097);
  dout >> resultString;
  ASSERT_STREQ(data.c_str(), resultString.c_str());
}

TEST(TestReserveSpace, TestBuffer)
{
  qi::Buffer      buffer;
  unsigned char   *image, *resultImage;
  int             fiveM = 5242880;
  void           *reservedSpace1;
  std::string     str("Oh man, this is a super config file, check it out !");

  // let's put a string in buffer
  reservedSpace1 = buffer.reserve(150);
  //Oh wait it's a config file !
  ASSERT_TRUE(buffer.reserve(1024));

  image = new unsigned char [fiveM];
  srand(time(NULL));
  for (int i = 0; i < fiveM; i++)
    image[i] = rand() % 256;

  buffer.write(image, fiveM);
  resultImage = (unsigned char*)buffer.data() + 150 + 1024;

  for (int i = 0; i < fiveM; i++)
    EXPECT_EQ(image[i], resultImage[i]) << "Original image and and actual differ at index " << i;

  ::memset(reservedSpace1, 0, 150);
  ::memcpy(reservedSpace1, str.c_str(), str.size());
}
