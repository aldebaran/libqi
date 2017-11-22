/*
** Copyright (C) 2014-2017 Softbank Robotics Europe
** See COPYING for the license
*/

#include <gtest/gtest.h>

#include <src/messaging/streamcontext.hpp>

TEST(TestStreamContext, sendCacheSetInsertTwice)
{
  qi::StreamContext ctx;
  qi::MetaObjectBuilder b;
  b.setDescription("my_mo");
  qi::MetaObject mo = b.metaObject();

  std::pair<unsigned int, bool> res1 = ctx.sendCacheSet(mo);
  EXPECT_TRUE(res1.second);

  std::pair<unsigned int, bool> res2 = ctx.sendCacheSet(mo);
  EXPECT_FALSE(res2.second);
  EXPECT_EQ(res1.first, res2.first);
}

TEST(TestStreamContext, sendCacheSetInsertTwoDifferent)
{
  qi::StreamContext ctx;
  qi::MetaObjectBuilder b;
  b.setDescription("my_mo1");
  qi::MetaObject mo1 = b.metaObject();
  b.setDescription("my_mo2");
  qi::MetaObject mo2 = b.metaObject();

  std::pair<unsigned int, bool> res1 = ctx.sendCacheSet(mo1);
  EXPECT_TRUE(res1.second);

  std::pair<unsigned int, bool> res2 = ctx.sendCacheSet(mo2);
  EXPECT_TRUE(res2.second);
  EXPECT_NE(res1.first, res2.first);
}
