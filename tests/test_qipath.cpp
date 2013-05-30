/*
 * Copyright (c) 2012 Aldebaran Robotics. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the COPYING file.
 */

#include <fstream>
#include <numeric>

#include <gtest/gtest.h>
#define BOOST_FILESYSTEM_VERSION 3
#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>
#include <locale>

#include "../src/sdklayout.hpp"
#include <qi/application.hpp>
#include <qi/qi.hpp>
#include <qi/os.hpp>
#include <qi/path.hpp>
#include "../src/utils.hpp"
#include <iostream>
#include <sstream>
#include <boost/scoped_ptr.hpp>

namespace bfs = boost::filesystem;

bfs::path absPath(const std::string& pPath)
{
  return bfs::absolute(bfs::system_complete(pPath)).make_preferred();
}

boost::filesystem::path normalize(boost::filesystem::path path1,
                                  boost::filesystem::path path2)
{
  if (*path2.begin() == ".")
    return path1;
  if (*path2.begin() == "..")
    return path1.parent_path();
  else
    return path1 /= path2;
}


std::string normalizePath(const std::string& path)
{
  boost::filesystem::path p(path, qi::unicodeFacet());

  p = std::accumulate(p.begin(), p.end(), boost::filesystem::path(), normalize);

  return p.make_preferred().string(qi::unicodeFacet());
}

boost::filesystem::path getHomePath()
{
  std::string p = bfs::absolute(qi::os::home()).string(qi::unicodeFacet());
  boost::to_lower(p);
  return bfs::path(p, qi::unicodeFacet());
}

TEST(qiPath, callingInit)
{
  bfs::path expected(normalizePath(absPath(std::string(::testing::internal::GetArgvs()[0])).string(qi::unicodeFacet())), qi::unicodeFacet());
  expected = expected.parent_path().parent_path();

  std::string actual = qi::path::sdkPrefix();
  std::string expect = expected.string(qi::unicodeFacet());
  boost::to_lower(expect);
  boost::to_lower(actual);

  ASSERT_EQ(expect, actual);
}

TEST(qiPath, callingInit2)
{
  const char *pgm = "build/sdk";
  bfs::path expected(absPath("build/sdk"));

  std::string actual = qi::SDKLayout(pgm).sdkPrefix();
  std::string expect = expected.string(qi::unicodeFacet());
  boost::to_lower(expect);
  boost::to_lower(actual);

  ASSERT_EQ(expect, actual);
}

#ifdef __linux__
TEST(qiPath, callingInit3)
{
  bfs::path expected(absPath("build/sdk"));

  std::string actual = qi::SDKLayout("build/sdk").sdkPrefix();
  std::string expect = expected.string(qi::unicodeFacet());
  boost::to_lower(expect);
  boost::to_lower(actual);

  ASSERT_EQ(expect, actual);
}
#endif


TEST(qiPath, AddPrefixesPath)
{
  qi::SDKLayout sdkl = qi::SDKLayout();

  sdkl.addOptionalSdkPrefix("build/sdk");
  sdkl.addOptionalSdkPrefix("debug");

  std::vector<std::string> prefixes = sdkl.getSdkPrefixes();
  std::vector<std::string> expected;

  expected.push_back(sdkl.sdkPrefix());
  expected.push_back(absPath("build/sdk").string(qi::unicodeFacet()));
  expected.push_back(absPath("debug").string(qi::unicodeFacet()));

  for (unsigned int i = 0; i < expected.size(); ++i)
    boost::to_lower(expected[i]);

  for (unsigned int i = 0; i < prefixes.size(); ++i)
    boost::to_lower(prefixes[i]);

  ASSERT_TRUE(expected == prefixes);
}

TEST(qiPath, ClearPrefixesPath)
{
  qi::SDKLayout sdkl = qi::SDKLayout();

  sdkl.addOptionalSdkPrefix("build/sdk");
  sdkl.addOptionalSdkPrefix("debug");

  std::vector<std::string> prefixes = sdkl.getSdkPrefixes();
  std::vector<std::string> expected;

  expected.push_back(sdkl.sdkPrefix());
  expected.push_back(absPath("build/sdk").string(qi::unicodeFacet()));
  expected.push_back(absPath("debug").string(qi::unicodeFacet()));

  for (unsigned int i = 0; i < expected.size(); ++i)
    boost::to_lower(expected[i]);

  for (unsigned int i = 0; i < prefixes.size(); ++i)
    boost::to_lower(prefixes[i]);

  ASSERT_TRUE(expected == prefixes);

  sdkl.clearOptionalSdkPrefix();
  expected.clear();
  expected.push_back(sdkl.sdkPrefix());

  prefixes = sdkl.getSdkPrefixes();

  for (unsigned int i = 0; i < expected.size(); ++i)
    boost::to_lower(expected[i]);

  for (unsigned int i = 0; i < prefixes.size(); ++i)
    boost::to_lower(prefixes[i]);

  ASSERT_TRUE(expected == prefixes);
}

#ifndef _WIN32
TEST(qiPath, FindBin)
{
  qi::SDKLayout sdkl = qi::SDKLayout();

  bfs::path expected(sdkl.sdkPrefix(), qi::unicodeFacet());
  std::string exp;

  expected = expected / "bin/test_qipath";
  expected = expected.make_preferred();

  std::string binPath1 = sdkl.findBin("test_qipath");
  std::string binPath2 = sdkl.findBin("qithatreallydoesnotexistsplease");

  exp = expected.string(qi::unicodeFacet());
  boost::to_lower(exp);
  boost::to_lower(binPath1);
  boost::to_lower(binPath2);

  ASSERT_EQ(exp, binPath1);
  ASSERT_EQ("", binPath2);
}
#endif

#ifndef _MSC_VER
TEST(qiPath, GetLinuxBinPaths)
{
  qi::SDKLayout sdkl = qi::SDKLayout();
  bfs::path prefix(sdkl.sdkPrefix(), qi::unicodeFacet());

  std::vector<std::string> expected;
  std::vector<std::string> binPaths = sdkl.binPaths();

  expected.push_back((prefix / "bin").make_preferred().string(qi::unicodeFacet()));

  for (unsigned int i = 0; i < expected.size(); ++i)
    boost::to_lower(expected[i]);

  for (unsigned int i = 0; i < binPaths.size(); ++i)
    boost::to_lower(binPaths[i]);

  ASSERT_TRUE(expected == binPaths);
}

TEST(qiPath, GetLinuxlibPaths)
{
  qi::SDKLayout sdkl = qi::SDKLayout();
  bfs::path prefix(sdkl.sdkPrefix(), qi::unicodeFacet());

  std::vector<std::string> expected;
  std::vector<std::string> binPaths = sdkl.libPaths();

  expected.push_back((prefix / "lib").make_preferred().string(qi::unicodeFacet()));

  for (unsigned int i = 0; i < expected.size(); ++i)
    boost::to_lower(expected[i]);

  for (unsigned int i = 0; i < binPaths.size(); ++i)
    boost::to_lower(binPaths[i]);

  ASSERT_TRUE(expected == binPaths);
}
#endif

#ifndef _WIN32
TEST(qiPath, callingGetUserDataPath)
{
  bfs::path expected(getHomePath() / ".local" / "share" / "foo" / "foo.data");

  std::string actual = qi::path::userWritableDataPath("foo", "foo.data");
  std::string expect = expected.string(qi::unicodeFacet());
  boost::to_lower(expect);
  boost::to_lower(actual);

  ASSERT_EQ(expect, actual);
}
#else
TEST(qiPath, callingGetUserDataPath)
{
  std::string appdata = qi::os::getenv("AppData");
  boost::to_lower(appdata);
  bfs::path envUserAppData(appdata, qi::unicodeFacet());
  bfs::path expected = envUserAppData / "foo" / "foo.data";

  std::string actual = qi::path::userWritableDataPath("foo", "foo.data");
  std::string expect = expected.string(qi::unicodeFacet());
  boost::to_lower(expect);
  boost::to_lower(actual);

  ASSERT_EQ(expect, actual);
}
#endif

#ifndef _WIN32
TEST(qiPath, LinuxConfigPaths)
{
  qi::SDKLayout* sdkl = new qi::SDKLayout();

  bfs::path expected(sdkl->sdkPrefix(), qi::unicodeFacet());
  std::vector<std::string> actualPrefsPaths = sdkl->confPaths("foo");

  bfs::path writeablePath(getHomePath() / ".config" / "foo") ;

  std::vector<std::string> expectedPrefPaths;
  expectedPrefPaths.push_back(writeablePath.string(qi::unicodeFacet()));
  expectedPrefPaths.push_back((expected / "etc/foo").make_preferred().string(qi::unicodeFacet()));
  expectedPrefPaths.push_back((expected / "etc").make_preferred().string(qi::unicodeFacet()));
  expectedPrefPaths.push_back((expected / "preferences/foo").make_preferred().string(qi::unicodeFacet()));
  expectedPrefPaths.push_back((expected / "preferences").make_preferred().string(qi::unicodeFacet()));
  expectedPrefPaths.push_back(absPath("/etc/foo").string(qi::unicodeFacet()));

  for (unsigned int i = 0; i < expectedPrefPaths.size(); ++i)
    boost::to_lower(expectedPrefPaths[i]);

  for (unsigned int i = 0; i < actualPrefsPaths.size(); ++i)
    boost::to_lower(actualPrefsPaths[i]);

  ASSERT_TRUE(expectedPrefPaths == actualPrefsPaths);

  std::string actual = sdkl->userWritableConfPath("foo", "");
  std::string expect = writeablePath.string(qi::unicodeFacet());
  boost::to_lower(expect);
  boost::to_lower(actual);

  ASSERT_EQ(expect, actual);

  delete sdkl;
}
#endif

TEST(qiPath, dataPaths)
{
  qi::SDKLayout* sdkl = new qi::SDKLayout();

  std::vector<std::string> expectedPrefPaths;

 #ifndef _WIN32
  bfs::path writeablePath(getHomePath() / ".local" / "share" / "foo");
 #else
  std::string envUserAppData = qi::os::getenv("AppData");
  bfs::path writeablePath(envUserAppData, qi::unicodeFacet());
  writeablePath = writeablePath / "foo";
 #endif

  bfs::path expected(sdkl->sdkPrefix(), qi::unicodeFacet());
  expectedPrefPaths.push_back(writeablePath.string(qi::unicodeFacet()));
  expectedPrefPaths.push_back((expected / "share/foo").make_preferred().string(qi::unicodeFacet()));

  std::vector<std::string> actualPrefsPaths = sdkl->dataPaths("foo");

  for (unsigned int i = 0; i < actualPrefsPaths.size(); ++i)
    boost::to_lower(actualPrefsPaths[i]);

  for (unsigned int i = 0; i < expectedPrefPaths.size(); ++i)
    boost::to_lower(expectedPrefPaths[i]);

  ASSERT_TRUE(expectedPrefPaths == actualPrefsPaths);
  std::string actual = sdkl->userWritableDataPath("foo", "");
  std::string expect = writeablePath.string(qi::unicodeFacet());
  boost::to_lower(expect);
  boost::to_lower(actual);

  ASSERT_EQ(expect, actual);

  delete sdkl;
}

TEST(qiPath, customWritablePath)
{
  const char* args = { "build/sdk/bin/foo" };
  qi::SDKLayout* sdkl = new qi::SDKLayout(args);

  std::string p, r, e;

  p = "/tmp/chiche/";
  qi::os::setenv("QI_WRITABLE_PATH", p.c_str());
  r = sdkl->userWritableConfPath("foo", "foo");
  e = fsconcat(p, "config", "foo", "foo");
  ASSERT_EQ(r, e);
  r = sdkl->userWritableDataPath("foo", "foo");
  e = fsconcat(p, "data", "foo", "foo");
  ASSERT_EQ(r, e);
  qi::os::setenv("QI_WRITABLE_PATH", "");

  p = "/tmp/42/";
  sdkl->setWritablePath(p);
  r = sdkl->userWritableConfPath("foo", "foo");
  e = fsconcat(p, "config", "foo", "foo");
  ASSERT_EQ(r, e);
  r = sdkl->userWritableDataPath("foo", "foo");
  e = fsconcat(p, "data", "foo", "foo");
  ASSERT_EQ(r, e);
}

TEST(qiPath, readingWritingfindConfigs)
{
  const char* args = { (char *) "build/sdk/bin/foo" };
  qi::SDKLayout* sdkl = new qi::SDKLayout(args);

 #ifndef _WIN32
  bfs::path writeablePath(bfs::absolute(qi::os::home()) / ".config" / "foo" / "foo.cfg");
 #else
  std::string userAppData = qi::os::getenv("AppData");
  bfs::path writeablePath(bfs::absolute(userAppData) / "foo" / "foo.cfg");
 #endif

  std::string fooCfg = sdkl->userWritableConfPath("foo", "foo.cfg");
  std::ofstream ofs;
  ofs.open(fooCfg.c_str(), std::fstream::out | std::fstream::trunc);
  ASSERT_FALSE (ofs.bad()) << "could not open" << fooCfg;
  ofs << "Hi, this is foo" << std::endl;
  ofs.close();

  fooCfg = sdkl->findConf("foo", "foo.cfg");
  std::string expect = writeablePath.string(qi::unicodeFacet());
  boost::to_lower(expect);
  boost::to_lower(fooCfg);
  ASSERT_EQ(expect, fooCfg);

  std::cout << "removing: " << fooCfg << std::endl;
  remove(fooCfg.c_str());

  std::string noCfgExisting = sdkl->findConf("foo", "bar.cfg");
  ASSERT_EQ(std::string(), noCfgExisting);

  delete sdkl;
}


TEST(qiPath, readingWritingFindData)
{
  const char* args = { (char *) "build/sdk/bin/foo" };
  qi::SDKLayout* sdkl = new qi::SDKLayout(args);

 #ifndef _WIN32
  bfs::path writeablePath(bfs::absolute(qi::os::home()) / ".local" / "share" / "foo" / "foo.dat");
 #else
  std::string userAppData = qi::os::getenv("AppData");
  bfs::path writeablePath(bfs::absolute(userAppData) / "foo" / "foo.dat");
 #endif

  std::string fooDat = sdkl->userWritableDataPath("foo", "foo.dat");
  std::ofstream ofs;
  ofs.open(fooDat.c_str(), std::fstream::out | std::fstream::trunc);
  ASSERT_FALSE (ofs.bad()) << "could not open" << fooDat;
  ofs << "Hi, this is foo" << std::endl;
  ofs.close();

  fooDat = sdkl->findData("foo", "foo.dat");
  std::string expect = writeablePath.string(qi::unicodeFacet());
  boost::to_lower(expect);
  boost::to_lower(fooDat);
  ASSERT_EQ(expect, fooDat);

  std::cout << "removing: " << fooDat << std::endl;
  remove(fooDat.c_str());

  std::string noDataExisting = sdkl->findData("foo", "bar.dat");
  ASSERT_EQ(std::string(), noDataExisting);

  delete sdkl;
}


void writeData(std::string path)
{
  std::cout << "creating: " << path << std::endl;
  std::ofstream ofs;
  ofs.open(path.c_str(), std::fstream::out | std::fstream::trunc);
  ofs << path;
  ofs.close();
}

// helper function to populate application data directories for testing
// purposes.
void createData(std::string root, std::string listing)
{
  std::istringstream stream(listing);
  std::string line;
  while(std::getline(stream, line))
  {
    std::string file_string(fsconcat(root, line));
    bfs::path file_path(file_string, qi::unicodeFacet());
    bfs::create_directories(file_path.parent_path());
    writeData(file_string);
  }
}

void createData(bfs::path root, std::string listing)
{
  return createData(root.string(qi::unicodeFacet()), listing);
}

// a fixture which creates files and directories for the findData and listData
// tests. The tests should not add nor remove files or directories.
class qiPathData : public ::testing::Test
{
protected:
  static void SetUpTestCase();
  static void TearDownTestCase();
  static bfs::path sdkShareFoo;
  static bfs::path optSdkPrefix;
  static bfs::path optSdkShareFoo;
  static bfs::path userShareFooBazDat;
  static bfs::path userShareFooFooDat;
  static bfs::path userShareFooUserDat;
};

bfs::path qiPathData::sdkShareFoo;
bfs::path qiPathData::optSdkPrefix;
bfs::path qiPathData::optSdkShareFoo;
bfs::path qiPathData::userShareFooBazDat;
bfs::path qiPathData::userShareFooFooDat;
bfs::path qiPathData::userShareFooUserDat;

void qiPathData::SetUpTestCase()
{
  boost::scoped_ptr<qi::SDKLayout> sdkl(new qi::SDKLayout());

  // the sdk dir of the program that is currently running
  bfs::path prefix(sdkl->sdkPrefix(), qi::unicodeFacet());
  qiPathData::sdkShareFoo = prefix / "share" / "foo";
  bfs::remove_all(sdkShareFoo);
  std::string listing = "bar.dat\n"
                        "bar.dat.tmp\n"
                        "bar/baz.dat\n"
                        "bar/baz_dat\n"
                        "baz.dat\n"
                        "baz/baz.dat\n"
                        "nasty_evil-ch.ars.dat\n"
                        "dir.dat/check_directories_are_skipped";
  createData(sdkShareFoo, listing);

  // an optional complementary sdk dir
  optSdkPrefix = bfs::path(qi::os::tmpdir("optSdk"), qi::unicodeFacet());
  optSdkShareFoo = optSdkPrefix / "share" / "foo";
  listing = "bar.dat\n"
            "foo.dat\n"
            "bar/baz.dat\n"
            "bam/baz.dat\n"
            "opt.dat";
  createData(optSdkShareFoo, listing);

  // the user dir
  userShareFooBazDat = sdkl->userWritableDataPath("foo", "baz.dat");
  writeData(userShareFooBazDat.string(qi::unicodeFacet()));
  userShareFooFooDat = sdkl->userWritableDataPath("foo", "foo.dat");
  writeData(userShareFooFooDat.string(qi::unicodeFacet()));
  userShareFooUserDat = sdkl->userWritableDataPath("foo", "user.dat");
  writeData(userShareFooUserDat.string(qi::unicodeFacet()));
}


void qiPathData::TearDownTestCase()
{
  std::cout << "removing: " << sdkShareFoo << std::endl;
  bfs::remove_all(sdkShareFoo);

  std::cout << "removing: " << optSdkPrefix << std::endl;
  bfs::remove_all(optSdkPrefix);

  std::cout << "removing: " << userShareFooBazDat << std::endl;
  bfs::remove(userShareFooBazDat);

  std::cout << "removing: " << userShareFooFooDat << std::endl;
  bfs::remove(userShareFooFooDat);

  std::cout << "removing: " << userShareFooUserDat << std::endl;
  bfs::remove(userShareFooUserDat);
}


// function to help writing compact tests
bool isInVector(boost::filesystem::path path,
                const std::vector<std::string> &vector)
{
  std::vector<std::string>::const_iterator it =
      std::find(vector.begin(), vector.end(),
                path.make_preferred().string(qi::unicodeFacet()));
  return it != vector.end();
}

TEST_F(qiPathData, findDataInSubfolder)
{
  boost::scoped_ptr<qi::SDKLayout> sdkl(new qi::SDKLayout());
  std::string actual = sdkl->findData("foo", "bar/baz.dat");
  std::string expected = (sdkShareFoo / "bar/baz.dat").make_preferred().string(qi::unicodeFacet());
  ASSERT_EQ(expected, actual);
}

TEST_F(qiPathData, listDataWithOptSdk)
{
  qi::SDKLayout* sdkl = new qi::SDKLayout();
  sdkl->addOptionalSdkPrefix(optSdkPrefix.string(qi::unicodeFacet()).c_str());

  std::vector<std::string> actual = sdkl->listData("foo", "*.dat");
  EXPECT_EQ(6, actual.size());

  // user.dat is only available in user.
  EXPECT_TRUE(isInVector(userShareFooUserDat, actual));

  // foo.dat is available in optSdk and user.
  EXPECT_TRUE(isInVector(userShareFooFooDat, actual));

  // baz.dat is available in sdk and user.
  EXPECT_TRUE(isInVector(userShareFooBazDat, actual));

  // bar.dat is available in optSdk and sdk
  EXPECT_TRUE(isInVector(sdkShareFoo / "bar.dat", actual));

  // nasty-evil_ch.ars.dat is only available in sdk
  EXPECT_TRUE(isInVector(sdkShareFoo / "nasty_evil-ch.ars.dat", actual));

  // opt.dat is only available in sdk
  EXPECT_TRUE(isInVector(optSdkShareFoo / "opt.dat", actual));
}

TEST_F(qiPathData, listDataInSubFolder)
{
  boost::scoped_ptr<qi::SDKLayout> sdkl(new qi::SDKLayout());
  std::string expected = (sdkShareFoo / "bar/baz.dat").make_preferred().string(qi::unicodeFacet());
  std::vector<std::string> actual;

  actual = sdkl->listData("foo", "bar/baz.dat");
  ASSERT_EQ(1, actual.size());
  EXPECT_EQ(actual[0], expected);

#ifdef _WIN32
  actual = sdkl->listData("foo", "bar\\baz.dat");
  ASSERT_EQ(1, actual.size());
  EXPECT_EQ(actual[0], expected);
#endif

  actual = sdkl->listData("foo", "*bar/baz.dat");
  ASSERT_EQ(1, actual.size());
  EXPECT_EQ(actual[0], expected);

  actual = sdkl->listData("foo", "/bar/baz.dat");
  ASSERT_EQ(1, actual.size());
  EXPECT_EQ(actual[0], expected);

  // Application names ending with "/" are not really supported.
  // However, that would work with findData, so let support it with listData
  // too.
  actual = sdkl->listData("foo/", "bar/baz.dat");
  ASSERT_EQ(1, actual.size());
  EXPECT_EQ(actual[0], expected);

  // Well, in the following case, findData would work but listData does not
  // because the string "foo//bar/baz.dat" does not match "foo/bar/baz.dat".
  // A solution would be to normalize paths in fsconcat, but that would have
  // consequences on all qi::path.
  //actual = sdkl->listData("foo/", "/bar/baz.dat");
  //ASSERT_EQ(1, actual.size());
  //EXPECT_EQ(actual[0], expected);

  actual = sdkl->listData("foo", "/*bar/baz.dat");
  ASSERT_EQ(1, actual.size());
  EXPECT_EQ(actual[0], expected);

  actual = sdkl->listData("foo", "*/bar/baz.dat");
  EXPECT_EQ(0, actual.size());
}

TEST_F(qiPathData, listDataInSubFolderWithOptSdk)
{
  boost::scoped_ptr<qi::SDKLayout> sdkl(new qi::SDKLayout());
  sdkl->addOptionalSdkPrefix(optSdkPrefix.string(qi::unicodeFacet()).c_str());

  std::vector<std::string> actual = sdkl->listData("foo", "ba?/*.dat");
  EXPECT_EQ(3, actual.size());

  EXPECT_TRUE(isInVector(sdkShareFoo / "bar/baz.dat", actual));
  EXPECT_TRUE(isInVector(sdkShareFoo / "baz/baz.dat", actual));
  EXPECT_TRUE(isInVector(optSdkShareFoo / "bam/baz.dat", actual));
}

int main(int argc, char* argv[])
{
  qi::Application app(argc, argv);
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}

TEST(qiPath, filesystemConcat)
{
  std::string s0 = fsconcat("/toto", "tata");
  ASSERT_EQ(boost::filesystem::path("/toto/tata").make_preferred().string(qi::unicodeFacet()), s0);
  std::string s1 = fsconcat("/toto/", "tata");
  ASSERT_EQ(boost::filesystem::path("/toto/tata").make_preferred().string(qi::unicodeFacet()), s1);
  std::string s2 = fsconcat("toto/", "tata");
  ASSERT_EQ(boost::filesystem::path("toto/tata").make_preferred().string(qi::unicodeFacet()), s2);
  std::string s3 = fsconcat("toto", "tata");
  ASSERT_EQ(boost::filesystem::path("toto/tata").make_preferred().string(qi::unicodeFacet()), s3);

  std::string s4 = fsconcat("/toto", "/tata");
  ASSERT_EQ(boost::filesystem::path("/toto/tata").make_preferred().string(qi::unicodeFacet()), s4);
  std::string s5 = fsconcat("/toto/", "/tata/");
  ASSERT_EQ(boost::filesystem::path("/toto//tata/").make_preferred().string(qi::unicodeFacet()), s5);
  std::string s6 = fsconcat("toto/", "tata/");
  ASSERT_EQ(boost::filesystem::path("toto/tata/").make_preferred().string(qi::unicodeFacet()), s6);
  std::string s7 = fsconcat("toto", "tata");
  ASSERT_EQ(boost::filesystem::path("toto/tata").make_preferred().string(qi::unicodeFacet()), s7);

  std::string s8 = fsconcat("toto", "tata", "tutu", "titi", "tete", "tyty");
  ASSERT_EQ(boost::filesystem::path("toto/tata/tutu/titi/tete/tyty").make_preferred().string(qi::unicodeFacet()), s8);
}
