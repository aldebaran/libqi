/*
 * Copyright (c) 2012 Aldebaran Robotics. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the COPYING file.
 */

#include <gtest/gtest.h>
#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>
#include <locale>

#include <qi/qi.hpp>

#ifdef _MSC_VER
# pragma warning( push )
// truncation of constant value when building char* objects
# pragma warning( disable : 4309 )
#endif

#ifndef __APPLE__
TEST(qiPathTests, pathUTF8)
{
  //unicode but only with weird char
  char utf8[]     = { (char) 0xC5, (char) 0xAA, (char) 0x6E, (char) 0xC4, (char) 0xAD, (char) 0x63, (char) 0xC5, (char) 0x8D, (char) 0x64, (char) 0x65, (char) 0xCC, (char) 0xBD, (char) 0 };
  wchar_t utf16[] = { 0x016A, 0x006E, 0x012D, 0x0063, 0x014D, 0x0064, 0x0065, 0x033D, 0 };

  std::string utf8xx(utf8);
  std::wstring utf16xx(utf16);

  boost::filesystem::path path(utf8xx, qi::unicodeFacet());
  ASSERT_EQ(utf16xx, path.wstring(qi::unicodeFacet()));
  ASSERT_EQ(utf8xx, path.string(qi::unicodeFacet()));
}

TEST(qiPathTests, pathUTF8Too)
{
  // create a path containing "/hé" in utf-8, and verify 16-bit api gives utf-16
  char utf8[] = { (char) 47, (char) 104, (char) 195, (char) 169, (char) 0};
  wchar_t utf16[] = {47, 104, 233, 0};

  std::string utf8xx(utf8);
  std::wstring utf16xx(utf16);

  boost::filesystem::path path(utf8xx, qi::unicodeFacet());
  ASSERT_EQ(utf16xx, path.wstring(qi::unicodeFacet()));
  ASSERT_EQ(utf8xx, path.string(qi::unicodeFacet()));
}

TEST(qiPathTests, pathUTF16)
{
    // create a path containing "/hé" in utf-16, and verify 8-bit api gives utf-8
  wchar_t utf16[] = {47, 104, 233, 0};
  char utf8[] = {(char) 47, (char) 104, (char) 195, (char) 169, (char) 0};

  std::wstring utf16xx(utf16);
  std::string utf8xx(utf8);
  boost::filesystem::path path(utf16xx, qi::unicodeFacet());

  ASSERT_EQ(utf16xx, path.wstring(qi::unicodeFacet()));
  ASSERT_EQ(utf8xx, path.string(qi::unicodeFacet()));
}
#endif


#if 0
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
    // create a path containing "/hÃ©" in utf-16, and verify 8-bit api gives utf-8
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

#include <QString>
TEST(qiPathTests, conversionFromUTF16)
{
  // 中文 -> china
  wchar_t utf16c[] = {'/', 't', 'm', 'p', '/', 0x4E2D, 0x6587, 0};
  QChar utf16cc[] = {'/', 't', 'm', 'p', '/', 0x4E2D, 0x6587, 0};

  boost::filesystem::path bUtf16(utf16c, qi::unicodeFacet());
  const char *bUtf8char = bUtf16.string(qi::unicodeFacet()).c_str();
  std::string bUtf8String(bUtf8char);

  QString qUtf16 = QString((QChar*)utf16cc, 7);
  const char *qUtf8char = qUtf16.toUtf8().constData();
  std::string qUtf8String(qUtf8char);

  ASSERT_EQ(bUtf8String, qUtf8String);
}

int main(int argc, char* argv[])
{
  qi::init(argc, argv);
  std::locale loc(std::locale(), &qi::unicodeFacet());
  std::locale::global(loc);

  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
#endif

#ifdef _MSC_VER
# pragma warning( pop )
#endif
