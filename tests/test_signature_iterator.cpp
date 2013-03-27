/*
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2010, 2011 Aldebaran Robotics
*/

#include <gtest/gtest.h>
#include <qitype/signature.hpp>

#include <vector>
#include <map>

//bool verif_iter(const qi::Signature::iterator it,
//                const std::string &sig,
//                qi::Signature::Type type,
//                bool isComplex,
//                bool isPointer)

#define verif_iter(_it, _sig, _type, _hasChildren) \
{\
  EXPECT_STREQ(_sig, _it.signature().c_str());\
  EXPECT_TRUE(_hasChildren == _it.hasChildren()); \
  EXPECT_EQ(qi::Signature::_type, _it.type());\
}


void verif_bad(const qi::Signature::iterator it)
{
  qi::Signature::iterator ite;
  qi::Signature s;

  EXPECT_STREQ("", it.signature().c_str());
  EXPECT_FALSE(it.hasChildren());
  EXPECT_EQ(qi::Signature::Type_None, it.type());
  EXPECT_TRUE(it == ite);
  EXPECT_TRUE(it == s.end());
}

TEST(TestSignatureIterator, Simple) {

  qi::Signature::iterator it;

  qi::Signature signature("(is)");
  qi::Signature sig = signature.begin().children();

  EXPECT_TRUE(signature.isValid());
  EXPECT_TRUE(sig.isValid());
  EXPECT_STREQ("is", sig.toString().c_str());
  it = sig.begin();
  verif_iter(it, "i", Type_Int32, false);

  ++it;
  verif_iter(it, "s", Type_String, false);

  ++it;
  verif_bad(it);

  it++;
  verif_bad(it);
}

TEST(TestSignatureIterator, STL) {
  qi::Signature::iterator it;

  qi::Signature sig1("[iiss]");
  //TODO EXPECT_FALSE(sig1.isValid());
  it = sig1.begin();
}

TEST(TestSignatureIterator, Empty) {
  qi::Signature::iterator it;
  qi::Signature sig3("");
  it = sig3.begin();
  verif_iter(it, "", Type_None, false);
  EXPECT_TRUE(it == sig3.end());
  EXPECT_STREQ("", sig3.toString().c_str());
}

#if 0

TEST(TestSignatureIterator, Protobuf) {
  qi::Signature::iterator it;

  qi::Signature sig1("@toto@*i");
  it = sig1.begin();

  EXPECT_STREQ("@toto@*i", it.raw_signature);
  EXPECT_FALSE(it.raw_child_1);
  EXPECT_FALSE(it.raw_child_2);
  EXPECT_TRUE(it.pointer);
  EXPECT_EQ("@toto@*", it.signature());
  EXPECT_EQ("",        it.child_1());
  EXPECT_EQ("",        it.child_2());

  EXPECT_TRUE(it != sig1.end());
  ++it;
  EXPECT_STREQ("i", it.raw_signature);
  EXPECT_FALSE(it.raw_child_1);
  EXPECT_FALSE(it.raw_child_2);
  EXPECT_FALSE(it.pointer);
  EXPECT_EQ("i", it.signature());
  EXPECT_EQ("",  it.child_1());
  EXPECT_EQ("",  it.child_2());

  EXPECT_TRUE(it != sig1.end());
  ++it;
  EXPECT_TRUE(it == sig1.end());
}

std::string *space_i(int space) {
  std::string ret;
  for (i = 0; i < space; ++i)
    ret += " ";
  return ret;
};

void sig_print(const qi::Signature &sig, indent = 0) {
  qi::Signature::iterator it;

  for (it = sig.begin(); it != sig.end(); ++it) {
    if (it->simple()) {
      std::cout << space_i(indent) << "e:(" << it->type() << "): " << *it << std::endl;
    }
    else {
      std::cout << space_i(indent) << "c:(" << it->type() << "): " << *it << std::endl;
      qi::Signature subsig(*it);
      sig_print(subsig, indent++);
    }
  }

};

TEST(TestSignatureIterator, Complexx) {
  qi::Signature sig("ii{s[s]}");

  qi::Signature::iterator it;


}
#endif
