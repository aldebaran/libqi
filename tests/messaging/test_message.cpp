#include <string>
#include <algorithm>
#include <gtest/gtest.h>
#include <qi/application.hpp>
#include "src/messaging/message.hpp"

TEST(TestMessage, CopiesAreDistinct)
{
  using namespace qi;
  Message m0(Message::Type_Call, MessageAddress{509, 2, 3, 105});
  Message m1(m0);
  ASSERT_EQ(m0, m1);

  m1.setId(m0.id() + 1);
  ASSERT_NE(m0, m1);

  m1 = m0;
  ASSERT_EQ(m0, m1);

  int i = 5;
  Buffer buf;
  buf.write(&i, sizeof(int));
  m1.setBuffer(buf);
  ASSERT_EQ(sizeof(int), m1.buffer().size());
  ASSERT_NE(m0, m1);

  m0 = m1;
  ASSERT_EQ(sizeof(int), m1.buffer().size());
  ASSERT_EQ(sizeof(int), m0.buffer().size());
  ASSERT_EQ(m0, m1);
  ASSERT_EQ(m0.buffer().size(), m1.buffer().size());
  ASSERT_EQ(m0.buffer().totalSize(), m1.buffer().totalSize());

  m0 = {};
  ASSERT_EQ(sizeof(int), m1.buffer().size());
  ASSERT_EQ(0u, m0.buffer().size());
  ASSERT_NE(m0, m1);
  ASSERT_NE(m0.buffer().size(), m1.buffer().size());
  ASSERT_NE(m0.buffer().totalSize(), m1.buffer().totalSize());

  Buffer b(buf);
  ASSERT_EQ(sizeof(int), b.size());
  ASSERT_EQ(sizeof(int), buf.size());
  ASSERT_EQ(buf.size(), b.size());
  ASSERT_EQ(buf.totalSize(), b.totalSize());

  Buffer bb(std::move(buf));
  ASSERT_EQ(sizeof(int), bb.size());
  ASSERT_EQ(0u, buf.size());
  ASSERT_NE(buf.size(), bb.size());
  ASSERT_NE(buf.totalSize(), bb.totalSize());

}
