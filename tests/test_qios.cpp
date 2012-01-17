/*
 * Copyright (c) 2012 Aldebaran Robotics. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the COPYING file.
 */

#ifdef WIN32
# include <process.h>  // for getpid
#else
# include <unistd.h> // for getpid
#endif
#include <cstdio>

#include <gtest/gtest.h>
#include <boost/filesystem.hpp>

#include <qi/path.hpp>
#include <qi/os.hpp>
#include <qi/qi.hpp>

class QiOSTests: public ::testing::Test
{
public:
  QiOSTests()
    : a_newPath()
  {
  }

protected:
  void SetUp() {
    a_newPath = qi::os::mktmpdir("QiOsTest");
    a_newPath.append(a_accent, qi::unicodeFacet());
    FILE* fileHandle = qi::os::fopen(a_newPath.string(qi::unicodeFacet()).c_str(), "w");
    fclose(fileHandle);
//    QString pouet = QDir::tempPath() + "/" + QString::fromUtf8(a_accent);
//    FILE* fileHandle = qi::os::fopen(pouet.toUtf8().data(), "w");
//    fclose(fileHandle);
  }

  void TearDown() {
    if(boost::filesystem::exists(a_newPath)) {
      try {
        boost::filesystem::remove_all(a_newPath.parent_path());
      } catch (std::exception &) {
      }
    }
  }

public:
  boost::filesystem::path a_newPath;
  static const char       a_accent[4];
};

// "é" in utf-8 w. french locale.
const char QiOSTests::a_accent[4] =  { '/', 0xc3, 0xa9, '\0' } ;

TEST_F(QiOSTests, LowLevelAccent)
{
  // Try to retrieve the file.
  // The name must be in utf-8 to be retrieved on unix.
  ASSERT_TRUE(boost::filesystem::exists(a_newPath))
    << a_newPath.string() << std::endl;
}


// TODO: us qi::time when it's available :)
TEST(QiOs, sleep)
{
  qi::os::sleep(1);
}

TEST(QiOS, msleep)
{
  qi::os::msleep(1000);
}

TEST(QiOs, env)
{
  int ret = qi::os::setenv("TITI", "TUTU");
  ASSERT_FALSE(ret);
  EXPECT_EQ("TUTU", qi::os::getenv("TITI"));
}

TEST(QiOs, getpid)
{
  ASSERT_EQ(getpid(), qi::os::getpid());
}

TEST(QiOs, tmp)
{
  std::string temp = qi::os::tmp();

  ASSERT_NE(temp, std::string(""));

  EXPECT_TRUE(boost::filesystem::exists(temp));
  EXPECT_TRUE(boost::filesystem::is_directory(temp));
}


//check if the folder exists and is writable
void test_writable_and_empty(std::string fdir) {
  std::string             tempfile;
  boost::filesystem::path p(fdir, qi::unicodeFacet());
  boost::filesystem::path pp(fdir, qi::unicodeFacet());

  EXPECT_TRUE(boost::filesystem::exists(fdir));
  EXPECT_TRUE(boost::filesystem::is_directory(fdir));

  pp.append("tmpfile", qi::unicodeFacet());
  tempfile = pp.string(qi::unicodeFacet());

  FILE *f = qi::os::fopen(tempfile.c_str(), "w+");
  EXPECT_TRUE((void *)f);
  fclose(f);

  EXPECT_TRUE(boost::filesystem::exists(tempfile));
  EXPECT_TRUE(boost::filesystem::is_regular_file(tempfile));

  boost::filesystem::directory_iterator it(p);

  EXPECT_EQ(pp, it->path());
  it++;

  EXPECT_EQ(boost::filesystem::directory_iterator(), it);

}


void clean_dir(std::string fdir) {
  if(boost::filesystem::exists(fdir)) {
    //fail is dir is not removable => dir should be removable
    boost::filesystem::remove_all(fdir);
  }
}

TEST(QiOs, tmpdir_noprefix)
{
  std::string temp = qi::os::mktmpdir();
  test_writable_and_empty(temp);
  clean_dir(temp);
}

TEST(QiOs, tmpdir_tmpdir)
{
  std::string temp1 = qi::os::mktmpdir();
  std::string temp2 = qi::os::mktmpdir();
  EXPECT_NE(temp1, temp2);
  clean_dir(temp1);
  clean_dir(temp2);
}

TEST(QiOs, tmpdir_parent)
{
  std::string temp = qi::os::mktmpdir("plaf");

  boost::filesystem::path ppa(temp, qi::unicodeFacet());
  ppa = ppa.parent_path();

  boost::filesystem::path ppatmp(qi::os::tmp(), qi::unicodeFacet());
  EXPECT_TRUE(boost::filesystem::equivalent(ppa, ppatmp));

  clean_dir(temp);
}

TEST(QiOs, tmpdir_prefix)
{
  std::string temp = qi::os::mktmpdir("plaf");

  test_writable_and_empty(temp);

  boost::filesystem::path pp(temp, qi::unicodeFacet());
  std::string tempfname = pp.filename().string(qi::unicodeFacet());

  EXPECT_EQ(tempfname[0], 'p');
  EXPECT_EQ(tempfname[1], 'l');
  EXPECT_EQ(tempfname[2], 'a');
  EXPECT_EQ(tempfname[3], 'f');

  clean_dir(temp);
}

TEST(QiOs, tmpdir_prefix_accentuated)
{
  char utf8[]     = { 0xC5, 0xAA, 0x6E, 0xC4, 0xAD, 0x63, 0xC5, 0x8D, 0x64, 0x65, 0xCC, 0xBD, 0 };

  std::string temp = qi::os::mktmpdir(utf8);

  test_writable_and_empty(temp);
  clean_dir(temp);
}

TEST(QiOs, tmpdir_prefix_zero)
{
  std::string temp = qi::os::mktmpdir(0);
  test_writable_and_empty(temp);
  clean_dir(temp);
}
