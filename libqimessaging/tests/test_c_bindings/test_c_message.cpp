/*
** Author(s):
**  - Pierre Roullon <proullon@aldebaran-robotics.com>
**
** Copyright (C) 2012 Aldebaran Robotics
*/

#include <qimessaging/c/api_c.h>
#include <qimessaging/c/qi_c.h>
#include <gtest/gtest.h>

TEST(TestMessage, SimpleOperation)
{
  qi_message_t*  m = qi_message_create();

  // Yeah, laugh, sounds easy...Beware.
  qi_message_destroy(m);
}

TEST(TestMessage, SimpleWrite)
{
  qi_message_t* m = qi_message_create();
  int magic = 0x42DEAD42;


  // SIGNED TYPES
  qi_message_write_bool(m, true);
  qi_message_write_int8(m, 'a');
  qi_message_write_int8(m, -64);
  qi_message_write_int16(m, 19);
  qi_message_write_int16(m, -19);
  qi_message_write_int32(m, 1123132123);
  qi_message_write_int32(m, -1123132123);
  qi_message_write_int64(m, 4294967297);
  qi_message_write_int64(m, -4294967297);

  // UNSIGNED TYPES
  qi_message_write_uint8(m, 'a');
  qi_message_write_uint16(m, 19);
  qi_message_write_uint32(m, 1123132123);
  qi_message_write_uint64(m, 4294967297);

  // SPECIAL ONES
  qi_message_write_float(m, 3.14957f);
  qi_message_write_double(m, 2.42);
  qi_message_write_string(m, "Easter Egg\t");
  qi_message_write_raw(m, (char *) &magic, 4);

  qi_message_destroy(m);
}

TEST(TestMessage, SimpleRead)
{
  qi_message_t* m = qi_message_create();
  int magic = 0x42DEAD42;
  unsigned int size = 0;
  char *data = (char *) malloc(4);
  memcpy(data, &magic, 4);
  magic = 0;
  ASSERT_TRUE(data != 0);

  // SIGNED TYPES
  qi_message_write_bool(m, true);
  qi_message_write_int8(m, 97);
  qi_message_write_int8(m, -64);
  qi_message_write_int16(m, 19);
  qi_message_write_int16(m, -19);
  qi_message_write_int32(m, 1123132123);
  qi_message_write_int32(m, -1123132123);
  qi_message_write_int64(m, 4294967297);
  qi_message_write_int64(m, -4294967297);
  // UNSIGNED TYPES
  qi_message_write_uint8(m, 'a');
  qi_message_write_uint16(m, 19);
  qi_message_write_uint32(m, 1123132123);
  qi_message_write_uint64(m, 4294967297);
  // SPECIAL ONES
  qi_message_write_float(m, 3.14957f);
  qi_message_write_double(m, 2.42);
  qi_message_write_string(m, "Easter Egg\t");
  qi_message_write_raw(m, data, 4);


  ASSERT_TRUE(qi_message_read_bool(m));
  ASSERT_EQ(97, qi_message_read_int8(m));
  ASSERT_EQ(-64, qi_message_read_int8(m));
  ASSERT_EQ(19, qi_message_read_int16(m));
  ASSERT_EQ(-19, qi_message_read_int16(m));
  ASSERT_EQ(1123132123, qi_message_read_int32(m));
  ASSERT_EQ(-1123132123, qi_message_read_int32(m));
  ASSERT_TRUE(qi_message_read_int64(m) == 4294967297);
  ASSERT_TRUE(qi_message_read_int64(m) == -4294967297);
  ASSERT_TRUE(qi_message_read_int8(m) == 'a');
  ASSERT_TRUE(qi_message_read_uint16(m) == 19);
  ASSERT_TRUE(qi_message_read_uint32(m) == 1123132123);
  ASSERT_TRUE(qi_message_read_uint64(m) == 4294967297);
  ASSERT_EQ(3.14957f, qi_message_read_float(m));
  ASSERT_EQ(2.42, qi_message_read_double(m));
  ASSERT_EQ(0, strcmp(qi_message_read_string(m), "Easter Egg\t"));
  data = qi_message_read_raw(m, &size);
  memcpy(&magic, data, 4);
  ASSERT_EQ(0x42DEAD42, magic);
  ASSERT_EQ(4, size);

  qi_message_destroy(m);
}

TEST(TestMessage, TestList)
{
  ASSERT_TRUE(false);
}

TEST(TestMessage, TestTuple)
{
  ASSERT_TRUE(false);
}

TEST(TestMessage, TestMap)
{
  ASSERT_TRUE(false);
}
