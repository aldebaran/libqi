#include <gtest/gtest.h>  // gtest must be included first...!
#include <string>
#include <qi/serialization/serializable.hpp>
#include <qi/serialization/serializer.hpp>

using namespace qi::serialization;


struct Inner : Serializable {
  Inner() : text("hello"), number(42) {}

  std::string text;
  int number;

  void serialize(Serializer& v) {
    v.visit(text);
    v.visit(number);
  }
};


struct Outer : Serializable {
  Outer() : floatNumber(42.2f) {}

  float floatNumber;
  Inner inner;

  void serialize(Serializer& v) {
    v.visit(floatNumber);
    v.visit(inner);
    v.visit(inner);
  }
};

struct Hard : Serializable {
  std::vector<std::map<std::string, int> > content;

  void serialize(Serializer& v) {
     v.visit(content);
  }
};

struct Harder : Serializable {
  std::vector<std::map<std::string, Inner> > content;

  void serialize(Serializer& v) {
     v.visit(content);
  }
};

TEST(testSerializable, simpleStruct) {
  Inner t;
  t.number = 1;
  t.text = "hello world";

  qi::serialization::Message m;
  Serializer serializer(ACTION_SERIALIZE, m);
  t.serialize(serializer);

  Serializer deserializer(ACTION_DESERIALIZE, m);
  Inner t2;
  t2.serialize(deserializer);
}

TEST(testSerializable, multiLayer) {
  Outer t;
  t.floatNumber = 1.111f;
  t.inner.number = 1;
  t.inner.text = "shtuff";

  qi::serialization::Message m;
  Serializer serializer(ACTION_SERIALIZE, m);
  t.serialize(serializer);

  Serializer deserializer(ACTION_DESERIALIZE, m);
  Outer t2;
  t2.serialize(deserializer);
}

TEST(testSerializable, hard) {
  Hard t;
  std::vector<std::map<std::string, int> > shtuff;
  std::map<std::string, int> map;
  map.insert(std::make_pair("oink", 1));
  map.insert(std::make_pair("oink2", 2));
  shtuff.push_back(map);
  t.content = shtuff;

  qi::serialization::Message m;
  Serializer serializer(ACTION_SERIALIZE, m);
  t.serialize(serializer);

  Serializer deserializer(ACTION_DESERIALIZE, m);
  Hard t2;
  t2.serialize(deserializer);
}


TEST(testSerializable, harder) {
  Harder t;
  std::vector<std::map<std::string, Inner> > shtuff;
  std::map<std::string, Inner> map;
  Inner inner1;
  inner1.number = 5;
  Inner inner2;
  inner2.number = 6;
  map.insert(std::make_pair("oink", inner1));
  map.insert(std::make_pair("oink2", inner2));
  shtuff.push_back(map);
  t.content = shtuff;

  qi::serialization::Message m;
  Serializer serializer(ACTION_SERIALIZE, m);
  t.serialize(serializer);

  Serializer deserializer(ACTION_DESERIALIZE, m);
  Harder t2;
  t2.serialize(deserializer);
}

struct Point2D : Serializable {
  int x;
  int y;

  void serialize(Serializer& s) {
    s.visit(x, y);
  }
};

struct TimeStamp : Serializable {
  int seconds;
  int nanoseconds;

  void serialize(Serializer& s) {
    s.visit(seconds);
    s.visit(nanoseconds);
  }
};

struct StampedPoint2D : Serializable {
  Point2D point;
  TimeStamp time;

  void serialize(Serializer& s) {
    s.visit(point, time);
  }
};

TEST(testSerializable, points) {
  Point2D point;
  point.x = 1;
  point.y = 2;

  TimeStamp timestamp;
  timestamp.seconds = 10001;
  timestamp.nanoseconds = 999;

  StampedPoint2D stampedPoint;
  stampedPoint.point = point;
  stampedPoint.time = timestamp;

  // ---- code that will never be seen, and needs a little clearup
  qi::serialization::Message pointMessage;
  Serializer s1(ACTION_SERIALIZE, pointMessage);
  point.serialize(s1);

  qi::serialization::Message stampedPointMessage;
  Serializer s2(ACTION_SERIALIZE, stampedPointMessage);
  stampedPoint.serialize(s2);

  qi::serialization::Message stampedPointMessageAgain;
  Serializer s3(ACTION_SERIALIZE, stampedPointMessageAgain);
  stampedPoint.serialize(s3);

  // -- Get a Point2D from a Point2D message
  Serializer s4(ACTION_DESERIALIZE, pointMessage);
  Point2D resultPoint1;
  resultPoint1.serialize(s4);

  // -- Get a Point2D from a StampedPoint2D message
  Serializer s5(ACTION_DESERIALIZE, stampedPointMessageAgain);
  Point2D resultPoint2;
  // This only works because the first two serialized fields are x and y
  resultPoint2.serialize(s5);

  // -- Get a StampedPoint2D from a StampedPoint2D message
  Serializer s6(ACTION_DESERIALIZE, stampedPointMessage);
  StampedPoint2D resultStampedPoint;
  resultStampedPoint.serialize(s6);
  // ---------------------------------------------------

  ASSERT_EQ(point.x, resultPoint1.x);
  ASSERT_EQ(point.y, resultPoint1.y);
  ASSERT_EQ(point.x, resultPoint2.x);
  ASSERT_EQ(point.y, resultPoint2.y);
  ASSERT_EQ(point.x, resultStampedPoint.point.x);
  ASSERT_EQ(point.y, resultStampedPoint.point.y);
  ASSERT_EQ(timestamp.seconds, resultStampedPoint.time.seconds);
  ASSERT_EQ(timestamp.nanoseconds, resultStampedPoint.time.nanoseconds);
}