/*
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2010, 2011 Aldebaran Robotics
*/

#include <gtest/gtest.h>
#include <qimessaging/signature.hpp>

#include <vector>
#include <map>

//bool verif_iter(const qi::Signature::iterator it,
//                const std::string &sig,
//                qi::Signature::Type type,
//                bool isComplex,
//                bool isPointer)

#define verif_iter(_it, _sig, _type, _hasChildren, _isPointer) \
{\
  EXPECT_STREQ(_sig, _it.signature().c_str());\
  EXPECT_EQ(_hasChildren, _it.hasChildren()); \
  EXPECT_EQ(_isPointer, _it.pointer());\
  EXPECT_EQ(qi::Signature::_type, _it.type());\
}


void verif_bad(const qi::Signature::iterator it)
{
  qi::Signature::iterator ite;
  qi::Signature s;

  EXPECT_STREQ("", it.signature().c_str());
  EXPECT_FALSE(it.hasChildren());
  EXPECT_FALSE(it.pointer());
  EXPECT_EQ(qi::Signature::None, it.type());
  EXPECT_TRUE(it == ite);
  EXPECT_TRUE(it == s.end());
}

TEST(TestSignatureIterator, Simple) {

  qi::Signature::iterator it;

  qi::Signature sig("is");
  EXPECT_TRUE(sig.isValid());
  it = sig.begin();
  verif_iter(it, "i", Int, false, false);

  ++it;
  verif_iter(it, "s", String, false, false);

  ++it;
  verif_bad(it);

  it++;
  verif_bad(it);
}

TEST(TestSignatureIterator, STL) {
  qi::Signature::iterator it;

  qi::Signature sig1("[iiss]");
  //TODO EXPECT_FALSE(sig1.isValid());

  qi::Signature sig2("[i]*");
  EXPECT_TRUE(sig1.isValid());
  it = sig2.begin();
  verif_iter(it, "[i]*", List, true, true);

  qi::Signature sig3("{is}*");
  it = sig3.begin();
  verif_iter(it, "{is}*", Map, true, true);

  qi::Signature sig4("{is}**");
  it = sig4.begin();
  verif_iter(it, "{is}**", Map, true, 2);


  qi::Signature subsig = it.children();
  it = subsig.begin();
  verif_iter(it, "i", Int, false, false);
  //it++;
  it++;
  verif_iter(it, "s", String, false, false);

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
