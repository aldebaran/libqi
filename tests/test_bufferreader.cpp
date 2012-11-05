/*
 * Author(s):
 * - Nicolas Cornu <ncornu@aldebaran-robotics.com>
 *
 *  Copyright (c) 2012 Aldebaran Robotics. All rights reserved.
 */

#include <gtest/gtest.h>
#include <qi/qi.hpp>
#include <qi/buffer.hpp>
#include <qi/bufferreader.hpp>
#include <stdexcept>

TEST(TestBufferReader, TestSubBuffer)
{
  qi::Buffer buffer;
  char tmpStr1[] = "My ";
  char tmpStr2[] = "string!";
  char tmpSubStr[] = "beautiful ";

  buffer.write(tmpStr1, sizeof(tmpStr1));
  {
    qi::Buffer subBuffer;
    subBuffer.write(tmpSubStr, sizeof(tmpSubStr));
    buffer.addSubBuffer(subBuffer);
  }
  buffer.write(tmpStr2, sizeof(tmpStr2));

  qi::BufferReader reader1(buffer);
  ASSERT_EQ(reader1.position(), 0u);
  ASSERT_FALSE(reader1.hasSubBuffer());

  char *resultStr = new char[sizeof(tmpStr1) + sizeof(tmpSubStr) + sizeof(tmpStr2) - 2];

  size_t result = reader1.read(resultStr, sizeof(tmpStr1));
  ASSERT_EQ(result, sizeof(tmpStr1));
  ASSERT_EQ(sizeof(tmpStr1), reader1.position());

  ASSERT_TRUE(reader1.hasSubBuffer());

  {
    try {
      qi::BufferReader reader2(reader1.subBuffer());

      result = reader2.read(resultStr+sizeof(tmpStr1)-1, sizeof(tmpSubStr));
      ASSERT_EQ(result, sizeof(tmpSubStr));
    } catch (std::runtime_error& e) {
      ASSERT_STREQ("", e.what());
    }
  }

  result = reader1.read(resultStr+sizeof(tmpStr1)-1+sizeof(tmpSubStr)-1, sizeof(tmpStr2));

  ASSERT_EQ(result, sizeof(tmpStr2));

  ASSERT_STREQ("My beautiful string!", resultStr);

  delete[] resultStr;
}

TEST(TestBufferReader, TestPeekSeek)
{
  qi::Buffer buffer;
  char tmpStr1[] = "Blabla";

  buffer.write(tmpStr1, sizeof(tmpStr1));

  qi::BufferReader reader(buffer);
  ASSERT_EQ(0u, reader.position());

  bool bResult = reader.seek(100);
  ASSERT_FALSE(bResult);
  ASSERT_EQ(0u, reader.position());

  bResult = reader.seek(3u);
  ASSERT_TRUE(bResult);
  ASSERT_EQ(3u, reader.position());

  const char *str = (const char*)reader.peek(100u);
  ASSERT_EQ(str, (const char*)0);

  str = (const char*)reader.peek(3u);
  ASSERT_NE(str, (const char*)0);

  ASSERT_STREQ("bla", str);
}
