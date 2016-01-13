/*
** Copyright (C) 2014 Aldebaran Robotics
** See COPYING for the license
*/

#include <gtest/gtest.h>

#include <qi/type/metaobject.hpp>
#include <src/type/metaobject_p.hpp>
#include <qi/application.hpp>


qi::GenericFunctionParameters args(
  qi::AutoAnyReference p1=qi::AutoAnyReference(),
  qi::AutoAnyReference p2=qi::AutoAnyReference(),
  qi::AutoAnyReference p3=qi::AutoAnyReference())
{
  qi::GenericFunctionParameters res;
  if (p1.type()) res.push_back(p1); else return res;
  if (p2.type()) res.push_back(p2); else return res;
  if (p3.type()) res.push_back(p3); else return res;
  return res;
}

TEST(TestMetaObject, findMethod)
{
  qi::MetaObjectBuilder b;
  unsigned int f   = b.addMethod("i", "f", "(i)");
  unsigned int g1  = b.addMethod("i", "g", "(i)");
  unsigned int g2  = b.addMethod("i", "g", "(ii)");
  unsigned int h1i = b.addMethod("i", "h", "(i)");
  unsigned int h1s = b.addMethod("i", "h", "(s)");
  unsigned int h2  = b.addMethod("i", "h", "(ii)");

  qi::MetaObject mo = b.metaObject();
  bool canCache;
  int mid;
  mid = mo.findMethod("f", args(1), &canCache);
  EXPECT_EQ(mid, (int)f); EXPECT_TRUE(canCache);
  mid = mo.findMethod("g", args(1), &canCache);
  EXPECT_EQ(mid, (int)g1); EXPECT_TRUE(canCache);
  mid = mo.findMethod("g", args(1, 1), &canCache);
  EXPECT_EQ(mid, (int)g2); EXPECT_TRUE(canCache);
  // no garantee is made on result of findmethod(g, "foo"), so not tested
  mid = mo.findMethod("h", args(1), &canCache);
  EXPECT_EQ(mid, (int)h1i); EXPECT_FALSE(canCache);
  mid = mo.findMethod("h", args("foo"), &canCache);
  EXPECT_EQ(mid, (int)h1s); EXPECT_FALSE(canCache);
  mid = mo.findMethod("h", args(1, 1), &canCache);
  EXPECT_EQ(mid, (int)h2); EXPECT_TRUE(canCache);

  mid = mo.findMethod("h::(i)", args(1), &canCache);
  EXPECT_EQ(mid, (int)h1i); EXPECT_TRUE(canCache);

  // check null canCache
  mo.findMethod("h::(i)", args(1), 0);
  mid = mo.findMethod("h", args("foo"), 0);
  EXPECT_TRUE(true);
}

TEST(TestMetaObject, SHA1Digest_construction)
{
  qi::MetaObjectPrivate::SHA1Digest d;
  for (const auto v : d.sha1Digest)
  {
    EXPECT_EQ(0u, v);
  }
}

TEST(TestMetaObject, SHA1Digest_less_then_op)
{
  qi::MetaObjectPrivate::SHA1Digest d1;
  d1.sha1Digest[0] = 1;
  d1.sha1Digest[1] = 9;

  qi::MetaObjectPrivate::SHA1Digest d2;
  d2.sha1Digest[0] = 2;

  EXPECT_TRUE(d1 < d2);
  EXPECT_FALSE(d1 < d1);
}

TEST(TestMetaObject, default_constructed_mos_are_equal)
{
  qi::MetaObject mo1;
  qi::MetaObject mo2;
  EXPECT_FALSE(mo1 < mo2);
  EXPECT_FALSE(mo2 < mo1);
}

TEST(TestMetaObject, copied_mos_are_equal)
{
  qi::MetaObjectBuilder b;
  b.addMethod("i", "f", "(i)");

  qi::MetaObject mo1 = b.metaObject();
  qi::MetaObject mo2 = mo1;

  EXPECT_FALSE(mo1 < mo2);
  EXPECT_FALSE(mo2 < mo1);
}

TEST(TestMetaObject, independent_mos_are_different)
{
  qi::MetaObjectBuilder b1;
  b1.addMethod("i", "f", "(i)");
  b1.setDescription("first mo");

  qi::MetaObject mo1 = b1.metaObject();

  qi::MetaObjectBuilder b2;
  b2.addMethod("i", "f", "(i)");
  b2.setDescription("second mo");

  qi::MetaObject mo2 = b2.metaObject();

  EXPECT_TRUE(mo1 < mo2 || mo2 < mo1);
}

TEST(TestMetaObject, independent_mos_with_the_same_content_are_equal)
{
  qi::MetaObjectBuilder b1;
  b1.addMethod("i", "f", "(i)");
  b1.setDescription("my_mo");

  qi::MetaObject mo1 = b1.metaObject();

  qi::MetaObjectBuilder b2;
  b2.addMethod("i", "f", "(i)");
  b2.setDescription("my_mo");

  qi::MetaObject mo2 = b2.metaObject();

  EXPECT_FALSE(mo1 < mo2);
  EXPECT_FALSE(mo2 < mo1);
}
