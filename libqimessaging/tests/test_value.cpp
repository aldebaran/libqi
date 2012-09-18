/*
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2010, 2011 Aldebaran Robotics
*/

#include <gtest/gtest.h>
#include <qimessaging/genericvalue.hpp>
#include <qimessaging/datastream.hpp>
#include <vector>
#include <map>


class ALMemoryTest {

  template <typename T>
  T get(std::string QI_UNUSED(name)) {
    qi::GenericValue v;
    //v = get(name);
    //return v.to<T>();
  }

};

TEST(TestValue, BasicType) {
  bool  b = true;
  char  c = 42;
  int   i = 42;
  float f = 42.;
  double d = 42.;

  qi::GenericValue v(b);
  EXPECT_EQ(true, v.toBool());
  v.clear();
  EXPECT_FALSE(v.toBool());
  v.setValue(c);
  EXPECT_EQ(42, v.toChar());
  v.setValue(i);
  EXPECT_EQ(42, v.toInt32());
  v.setValue(f);
  EXPECT_EQ(42., v.toFloat());
  v.setValue(d);
  EXPECT_EQ(42., v.toDouble());
}


TEST(TestValue, WowType) {
  qi::GenericValue v(qi::GenericValue::List);
  qi::GenericValue v2("titi toto");
  qi::ValueList *vl = v.value<qi::ValueList>();
  vl->push_back(v2);

  qi::ValueList::iterator it;

  it = vl->begin();
  EXPECT_STREQ("titi toto", it->toString().c_str());
}

TEST(TestValue, TestSerialize) {
  qi::GenericValue v(qi::GenericValue::Map);
  qi::GenericValue v2;

  qi::ValueMap *vm1 = v.value<qi::ValueMap>();
  (*vm1)["toto"] = qi::GenericValue("coco");
  (*vm1)["tata"] = qi::GenericValue("caca");

  qi::Buffer     buf;
  qi::DataStream ms(buf);

  ms << v;
  ms >> v2;

  qi::ValueMap *vm = v2.value<qi::ValueMap>();
  qi::ValueMap::iterator it;

  it = vm->find("toto");
  EXPECT_STREQ("toto", it->first.c_str());
  EXPECT_STREQ("coco", it->second.toString().c_str());
  it = vm->find("tata");
  EXPECT_STREQ("tata", it->first.c_str());
  EXPECT_STREQ("caca", it->second.toString().c_str());
  EXPECT_EQ((unsigned int)2, vm->size());
}

TEST(TestValue, TestSerializeVector) {
  qi::GenericValue v(qi::GenericValue::Vector);
  qi::GenericValue v2;

  v.value<qi::ValueVector>()->push_back(qi::GenericValue("coco"));
  v.value<qi::ValueVector>()->push_back(qi::GenericValue("caca"));

  qi::Buffer     buf;
  qi::DataStream ms(buf);

  ms << v;
  ms >> v2;

  qi::ValueVector *vm = v2.value<qi::ValueVector>();
  qi::ValueVector::iterator it;

  it = vm->begin();
  EXPECT_STREQ("coco", it->toString().c_str());
  ++it;
  EXPECT_STREQ("caca", it->toString().c_str());
  ++it;
  EXPECT_EQ(1, vm->end() == it);
}

struct Point2D {
  int x;
  int y;
};

#if 0
QI_REFLECT(Point2D, ((int, x))((int, y)));

TEST(TestValue, StructType) {
  qi::GenericValue v;
  Point2D p;
  Point2D p2;
  p.x = 41;
  p.y = 42;

  v.setValue<Point2D>(p);
  p2 = v.value<Point2D>();

  EXPECT_EQ(41, p.x);
  EXPECT_EQ(41, p2.x);
  EXPECT_EQ(42, p.y);
  EXPECT_EQ(42, p2.y);

}


#endif
