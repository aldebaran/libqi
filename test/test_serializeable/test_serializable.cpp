#include <gtest/gtest.h>  // gtest must be included first...!
#include <string>
#include <qi/serialization/serializable.hpp>
#include <qi/serialization/serializer.hpp>

using namespace qi::serialization;


struct Inner : Serializable {
  Inner() : text("hello"), number(42) {}

  std::string text;
  int number;

  void accept(Serializer& v) {
    v.visit(text);
    v.visit(number);
  }
};


struct Outer : Serializable {
  Outer() : floatNumber(42.2f) {}

  float floatNumber;
  Inner inner;

  void accept(Serializer& v) {
    v.visit(floatNumber);
    v.visit(inner);
    v.visit(inner);
  }
};

struct Hard : Serializable {
  std::vector<std::map<std::string, int> > content;

  void accept(Serializer& v) {
     v.visit(content);
  }
};

struct Harder : Serializable {
  std::vector<std::map<std::string, Inner> > content;

  void accept(Serializer& v) {
     v.visit(content);
  }
};

TEST(testSerializable, simpleStruct) {
  Inner t;
  t.number = 1;
  t.text = "hello world";

  qi::serialization::Message m;
  Serializer serializer(ACTION_SERIALIZE, m);
  t.accept(serializer);

  Serializer deserializer(ACTION_DESERIALIZE, m);
  Inner t2;
  t2.accept(deserializer);
}

TEST(testSerializable, multiLayer) {
  Outer t;
  t.floatNumber = 1.111f;
  t.inner.number = 1;
  t.inner.text = "shtuff";

  qi::serialization::Message m;
  Serializer serializer(ACTION_SERIALIZE, m);
  t.accept(serializer);

  Serializer deserializer(ACTION_DESERIALIZE, m);
  Outer t2;
  t2.accept(deserializer);
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
  t.accept(serializer);

  Serializer deserializer(ACTION_DESERIALIZE, m);
  Hard t2;
  t2.accept(deserializer);
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
  t.accept(serializer);

  Serializer deserializer(ACTION_DESERIALIZE, m);
  Harder t2;
  t2.accept(deserializer);
}