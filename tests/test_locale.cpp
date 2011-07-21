/**
 * Copyright (c) 2011 Aldebaran Robotics
 */

#include <gtest/gtest.h>
#include <boost/algorithm/string.hpp>
#include <locale>

#include <qi/application.hpp>
#include <qi/locale.hpp>

TEST(qiLocale, utf8_utf16_1)
{
  //unicode but only with weird char
  char utf8[]     = { 0xC5, 0xAA, 0x6E, 0xC4, 0xAD, 0x63, 0xC5, 0x8D, 0x64, 0x65, 0xCC, 0xBD, 0 };
  wchar_t utf16[] = { 0x016A, 0x006E, 0x012D, 0x0063, 0x014D, 0x0064, 0x0065, 0x033D, 0 };

  std::string utf8xx(utf8);
  std::wstring utf16xx(utf16);
  std::string utf8yy(utf16xx.length(), ' ');
  std::wstring utf16yy(utf8xx.length(), L' ');

  std::cout  << "utf8  :" << utf8  << std::endl;
  std::wcout << "utf16 :" << utf16 << std::endl;

  std::copy(utf8xx.begin(), utf8xx.end(), utf16yy.begin());
  std::copy(utf16xx.begin(), utf16xx.end(), utf8yy.begin());
  EXPECT_EQ(utf16xx, utf16yy);
  EXPECT_EQ(utf8xx, utf8yy);
}

TEST(qiLocale, utf8_utf16_3)
{
    // create a path containing "/hé" in utf-16, and verify 8-bit api gives utf-8
  wchar_t utf16[] = {47, 104, 233, 0};
  char utf8[] = {47, 104, 195, 169, 0};

  std::string utf8xx(utf8);
  std::wstring utf16xx(utf16);
  std::string utf8yy(utf16xx.length(), ' ');
  std::wstring utf16yy(utf8xx.length(), L' ');

  std::copy(utf8xx.begin(), utf8xx.end(), utf16yy.begin());
  std::copy(utf16xx.begin(), utf16xx.end(), utf8yy.begin());
  EXPECT_EQ(utf16xx, utf16yy);
  EXPECT_EQ(utf8xx, utf8yy);
}


int main(int argc, char* argv[])
{
  qi::init(argc, argv);
  std::locale loc(std::locale(), &qi::unicodeFacet());
  std::locale::global(loc);

  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
