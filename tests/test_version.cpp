/*
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2010 Aldebaran Robotics
*/

#include <gtest/gtest.h>
#include <qi/version.hpp>


TEST(TestVersion, TestVersionExtract) {
  EXPECT_EQ("11.22.33"    , qi::version::extract("blabmal11.22.33.sdsdd")            );
  EXPECT_EQ("1.2.3"       , qi::version::extract("1.2.3.sdsdd")                      );
  EXPECT_EQ("11.22.33"    , qi::version::extract("11.22.33.sdsdsd332.3234.35")       );
  EXPECT_EQ("11.22.33-rc1", qi::version::extract("sdsd11.22.33-rc1sdsdsd332.3234.35"));
  EXPECT_EQ("11.22.33"    , qi::version::extract("sdsd11.22.33-r1sdsdsd332.3234.35") );
  EXPECT_EQ("11.22.33-oe2", qi::version::extract("sdsd11.22.33-oe2sdsdsd332.3234.35"));
  EXPECT_EQ("1.2.3"       , qi::version::extract("1.2.3")                            );
  EXPECT_EQ("1.2.3"       , qi::version::extract("1.2.3.")                           );
  EXPECT_EQ("1.2.3"       , qi::version::extract("1.2.3.-rc12")                      );
  EXPECT_EQ("1.2.3"       , qi::version::extract("1.2.3.-oe12")                      );
}

TEST(TestVersion, TestVersionCompare) {
  EXPECT_EQ( 0, qi::version::compare("1.2.3", "1.2.3")     );
  EXPECT_EQ( 1, qi::version::compare("1.20.3", "1.2.3")    );
  EXPECT_EQ( 1, qi::version::compare("0"     , "")         );
  EXPECT_EQ( 1, qi::version::compare("1.2..3", "1.2.3")    );
  EXPECT_EQ( 1, qi::version::compare("1.2.-.3", "1.2.3")   );
  EXPECT_EQ( 1, qi::version::compare("1.2.3.0", "1.2.3")   );
  EXPECT_EQ(-1, qi::version::compare("1.2.3.0", "1.2.a3")  );
  EXPECT_EQ(-1, qi::version::compare("1.2.3.0", "1.2.3a")  );
  EXPECT_EQ(-1, qi::version::compare("1.2.3.0", "1a1.2.3") );
  EXPECT_EQ( 1, qi::version::compare("10.2.3.0", "1a1.2.3"));

  EXPECT_EQ(-1, qi::version::compare("1&.2.3", "1&&&&.2.3"));

  EXPECT_EQ( 0, qi::version::compare("a", "a") );
  EXPECT_EQ( 1, qi::version::compare("aa", "a"));
  EXPECT_EQ(-1, qi::version::compare("aa", "b"));
  EXPECT_EQ(-1, qi::version::compare("01", "2"));
  EXPECT_EQ( 1, qi::version::compare("a0", "a"));

  EXPECT_EQ( 1, qi::version::compare("1a0", "1.0"));
  EXPECT_EQ( 1, qi::version::compare("a0a", "a.a"));
  EXPECT_EQ( 1, qi::version::compare("1a0", "1.1"));
  EXPECT_EQ( 1, qi::version::compare("a0a", "a.b"));

}

// It's important that the << operator is defined in the SAME
// namespace that defines Foo.  C++'s look-up rules rely on that.
::std::ostream& operator<<(::std::ostream& os, const std::vector<std::string>& foo) {
  std::vector<std::string>::const_iterator it;

  for (it = foo.begin(); it != foo.end(); ++it)
  {
    os << *it;
    if (it + 1 != foo.end())
      os << ", ";
  }
  return os;
}


TEST(TestVersion, TestVersionExplode) {
  std::vector<std::string> result1;
  std::vector<std::string> result2;
  std::vector<std::string> result3;

  result1.push_back("1");
  result1.push_back(".");
  result1.push_back("2");
  result1.push_back(".");
  result1.push_back("3");
  EXPECT_EQ(result1, qi::version::explode("1.2.3"));

  result2.push_back("1111");
  result2.push_back(".");
  result2.push_back("AaQZzzasesdSD");
  result2.push_back(".");
  result2.push_back("2");
  result2.push_back(".");
  result2.push_back(".");
  result2.push_back("3");
  result2.push_back("aaaaa");
  result2.push_back("*");
  result2.push_back("'");
  EXPECT_EQ(result2, qi::version::explode("1111.AaQZzzasesdSD.2..3aaaaa*'"));

  result3.push_back("1");
  result3.push_back(".");
  result3.push_back("2");
  result3.push_back(".");
  result3.push_back("3");
  result3.push_back("-");
  result3.push_back("oe");
  result3.push_back("2");
  EXPECT_EQ(result3, qi::version::explode("1.2.3-oe2"));
}
