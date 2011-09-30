/*
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2010, 2011 Aldebaran Robotics
*/

#include <gtest/gtest.h>
#include <qimessaging/value.hpp>
#include <qimessaging/serialization.hpp>

#include <vector>
#include <map>


//typedef boost::make_recursive_variant<
//      int
//    , std::vector< boost::recursive_variant_ >
//    >::type int_tree_t;


//typedef boost::make_recursive_variant<bool,
//                                      char,
//                                      int,
//                                      float,
//                                      double,
//                                      std::list<boost::recursive_variant_> ,
//                                      std::vector<boost::recursive_variant_> ,
//                                      std::map<boost::recursive_variant_> ,
//                                     > Value;

class ALMemoryTest {

  template <typename T>
  T get(std::string name) {
    qi::Value v;
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

  qi::Value v(b);
  EXPECT_EQ(true, v.toBool());
  v.setValue(c);
  EXPECT_EQ(42, v.toChar());
}


//struct Point2D {
//  int x;
//  int y;
//}

//QI_REFLECT(Point2D, (int x)(int y));

//TEST(TestValue, StructType) {
//  qi::Value v;
//  Point2D p;


//  p = pref.get<Point2D>("caca");

//  qi::Value v = pref.get("caca");

//  p = v.to<Point2D>();

//  int i = v.toInt();

//}
