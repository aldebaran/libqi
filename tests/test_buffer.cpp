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
