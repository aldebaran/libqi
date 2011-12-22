/*
 * Copyright (c) 2011, Aldebaran Robotics
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the Aldebaran Robotics nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL Aldebaran Robotics BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <gtest/gtest.h>
#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>
#include <locale>

#include <qi/qi.hpp>

#ifndef __APPLE__
TEST(qiPathTests, pathUTF8)
{
  //unicode but only with weird char
  char utf8[]     = { 0xC5, 0xAA, 0x6E, 0xC4, 0xAD, 0x63, 0xC5, 0x8D, 0x64, 0x65, 0xCC, 0xBD, 0 };
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
  char utf8[] = {47, 104, 195, 169, 0};
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
  char utf8[] = {47, 104, 195, 169, 0};

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

