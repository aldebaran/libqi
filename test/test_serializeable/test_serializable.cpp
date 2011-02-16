#include <gtest/gtest.h>  // gtest must be included first...!
#include <string>
#include <qi/serialization/serializeable.hpp>

using namespace qi::serialization;


class Reflective {
protected:
  std::string signature;

}

struct Inner2 : Reflective<Inner2> {
  Inner2() 
    : metaInformation(&mystring, &myint)
  {
  }
}


struct Inner : IVisitable {
  Inner() : text("hello"), number(42) {}

  std::string text;
  int number;

  void accept(IVisitor& v) {
    v.visit(text);
    v.visit(number);
  }
};



_class Inner().Int<&Inner::number>;

struct Outer : IVisitable {
  Outer() : floatNumber(42.2f) {}

  float floatNumber;
  Inner inner;

  void accept(IVisitor& v) {
    v.visit(floatNumber);
    v.visit(inner);
    qi::serialization<Inner>
  }
};

TEST(testSerializable, simpleStruct) {
  Inner t;
  t.number = 1;
  t.text = "hello world";
  PrintVisitor v;
  t.accept(v);

  qi::serialization::Message m;
  SerializeVisitor sv(m);
  t.accept(sv);

  DeSerializeVisitor dsv(m);
  Inner t2;
  t2.accept(dsv);
}

TEST(testSerializable, multiLayer) {
  Outer t;
  PrintVisitor v;
  t.accept(v);
}