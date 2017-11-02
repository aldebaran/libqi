/*
 * Copyright (c) 2012 Aldebaran Robotics. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the COPYING file.
 */

#include <boost/filesystem/fstream.hpp>
#include <numeric>

#include <gtest/gtest.h>
#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>
#include <locale>

#include "../../src/sdklayout.hpp"
#include <qi/application.hpp>
#include <qi/os.hpp>
#include <qi/path.hpp>
#include <qi/log.hpp>
#include "../../src/utils.hpp"
#include <sstream>
#include <boost/scoped_ptr.hpp>

#ifdef _WIN32
  #include <windows.h>
#endif


qiLogCategory("test.qi.path");

qi::Path argpath;

namespace bfs = boost::filesystem;

bfs::path absPath(const bfs::path& pPath)
{
  return bfs::absolute(bfs::system_complete(pPath)).make_preferred();
}

boost::filesystem::path getHomePath()
{
  std::string p = bfs::absolute(qi::os::home()).string(qi::unicodeFacet());
  boost::to_lower(p);
  return bfs::path(p, qi::unicodeFacet());
}

TEST(qiPath, callingInit)
{
  const qi::Path expected = qi::path::detail::normalize(absPath(argpath)).parent().parent();

  std::string actual = qi::path::sdkPrefix();
  std::string expect = expected.str();
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
  qi::SDKLayout sdkl;

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

  ASSERT_EQ(expected, prefixes);
}

TEST(qiPath, ClearPrefixesPath)
{
  qi::SDKLayout sdkl;

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

TEST(qiPath, FindLib)
{
  ASSERT_FALSE(qi::path::findLib("qi").empty());
}

#ifndef _WIN32
TEST(qiPath, FindBin)
{
  qi::SDKLayout sdkl;

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
  qi::SDKLayout sdkl;
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
  qi::SDKLayout sdkl;
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
  qi::SDKLayout sdkl;

  bfs::path expected(sdkl.sdkPrefix(), qi::unicodeFacet());
  std::vector<std::string> actualPrefsPaths = sdkl.confPaths("foo");

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

  std::string actual = sdkl.userWritableConfPath("foo", "");
  std::string expect = writeablePath.string(qi::unicodeFacet());
  boost::to_lower(expect);
  boost::to_lower(actual);

  ASSERT_EQ(expect, actual);
}
#endif

TEST(qiPath, dataPaths)
{
  qi::SDKLayout sdkl;

  std::vector<std::string> expectedPrefPaths;

 #ifndef _WIN32
  bfs::path writeablePath(getHomePath() / ".local" / "share" / "foo");
 #else
  std::string envUserAppData = qi::os::getenv("AppData");
  bfs::path writeablePath(envUserAppData, qi::unicodeFacet());
  writeablePath = writeablePath / "foo";
 #endif

  bfs::path expected(sdkl.sdkPrefix(), qi::unicodeFacet());
  expectedPrefPaths.push_back(writeablePath.string(qi::unicodeFacet()));
  expectedPrefPaths.push_back((expected / "share/foo").make_preferred().string(qi::unicodeFacet()));

  std::vector<std::string> actualPrefsPaths = sdkl.dataPaths("foo");

  for (unsigned int i = 0; i < actualPrefsPaths.size(); ++i)
    boost::to_lower(actualPrefsPaths[i]);

  for (unsigned int i = 0; i < expectedPrefPaths.size(); ++i)
    boost::to_lower(expectedPrefPaths[i]);

  ASSERT_TRUE(expectedPrefPaths == actualPrefsPaths);
  std::string actual = sdkl.userWritableDataPath("foo", "");
  std::string expect = writeablePath.string(qi::unicodeFacet());
  boost::to_lower(expect);
  boost::to_lower(actual);

  ASSERT_EQ(expect, actual);
}

TEST(qiPath, customWritablePath)
{
  const char* args = { "build/sdk/bin/foo" };
  qi::SDKLayout sdkl(args);

  std::string p, r, e;

  p = "/tmp/chiche/";
  qi::os::setenv("QI_WRITABLE_PATH", p.c_str());
  r = sdkl.userWritableConfPath("foo", "foo");
  e = fsconcat(p, "config", "foo", "foo");
  ASSERT_EQ(r, e);
  r = sdkl.userWritableDataPath("foo", "foo");
  e = fsconcat(p, "data", "foo", "foo");
  ASSERT_EQ(r, e);
  qi::os::setenv("QI_WRITABLE_PATH", "");

  p = "/tmp/42/";
  sdkl.setWritablePath(p);
  r = sdkl.userWritableConfPath("foo", "foo");
  e = fsconcat(p, "config", "foo", "foo");
  ASSERT_EQ(r, e);
  r = sdkl.userWritableDataPath("foo", "foo");
  e = fsconcat(p, "data", "foo", "foo");
  ASSERT_EQ(r, e);
}

TEST(qiPath, readingWritingfindConfigs)
{
  const char* args = { (char *) "build/sdk/bin/foo" };
  qi::SDKLayout sdkl(args);

 #ifndef _WIN32
  bfs::path writeablePath(bfs::absolute(qi::os::home()) / ".config" / "foo" / "foo.cfg");
 #else
  std::string userAppData = qi::os::getenv("AppData");
  bfs::path writeablePath(bfs::absolute(userAppData) / "foo" / "foo.cfg");
 #endif

  qi::Path fooCfgPath = sdkl.userWritableConfPath("foo", "foo.cfg");
  bfs::ofstream ofs;
  ofs.open(fooCfgPath, std::fstream::out | std::fstream::trunc);
  ASSERT_FALSE(ofs.bad()) << "could not open" << fooCfgPath;
  ofs << "Hi, this is foo" << std::endl;
  ofs.close();

  std::string fooCfg = sdkl.findConf("foo", "foo.cfg");
  std::string expect = writeablePath.string(qi::unicodeFacet());
  boost::to_lower(expect);
  boost::to_lower(fooCfg);
  ASSERT_EQ(expect, fooCfg);
  ASSERT_TRUE(sdkl.findConf("foo", "foo.cfg", true).empty());

  std::cout << "removing: " << fooCfg << std::endl;
  remove(fooCfg.c_str());

  std::string noCfgExisting = sdkl.findConf("foo", "bar.cfg");
  ASSERT_EQ(std::string(), noCfgExisting);
}


TEST(qiPath, readingWritingFindData)
{
  const char* args = { (char *) "build/sdk/bin/foo" };
  qi::SDKLayout sdkl(args);

 #ifndef _WIN32
  bfs::path writeablePath(bfs::absolute(qi::os::home()) / ".local" / "share" / "foo" / "foo.dat");
 #else
  std::string userAppData = qi::os::getenv("AppData");
  bfs::path writeablePath(bfs::absolute(userAppData) / "foo" / "foo.dat");
 #endif

  std::string fooDat = sdkl.userWritableDataPath("foo", "foo.dat");
  bfs::ofstream ofs;
  ofs.open(fooDat.c_str(), std::fstream::out | std::fstream::trunc);
  ASSERT_FALSE (ofs.bad()) << "could not open" << fooDat;
  ofs << "Hi, this is foo" << std::endl;
  ofs.close();

  fooDat = sdkl.findData("foo", "foo.dat");
  std::string expect = writeablePath.string(qi::unicodeFacet());
  boost::to_lower(expect);
  boost::to_lower(fooDat);
  ASSERT_EQ(expect, fooDat);

  std::cout << "removing: " << fooDat << std::endl;
  remove(fooDat.c_str());

  std::string noDataExisting = sdkl.findData("foo", "bar.dat");
  ASSERT_EQ(std::string(), noDataExisting);
}

void writeData(const qi::Path& path)
{
  std::cout << "creating: " << path << std::endl;
  bfs::ofstream ofs;
  ofs.open(path, std::fstream::out | std::fstream::trunc);
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
    const bfs::path file_path(file_string, qi::unicodeFacet());
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
  static bfs::path optSdkShareFuu;
  static bfs::path userShareFooBazDat;
  static bfs::path userShareFooFooDat;
  static bfs::path userShareFooUserDat;
};

bfs::path qiPathData::sdkShareFoo;
bfs::path qiPathData::optSdkPrefix;
bfs::path qiPathData::optSdkShareFoo;
bfs::path qiPathData::optSdkShareFuu;
bfs::path qiPathData::userShareFooBazDat;
bfs::path qiPathData::userShareFooFooDat;
bfs::path qiPathData::userShareFooUserDat;

void qiPathData::SetUpTestCase()
{
  qi::SDKLayout sdkl;

  // the sdk dir of the program that is currently running
  bfs::path prefix(sdkl.sdkPrefix(), qi::unicodeFacet());
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
  optSdkPrefix = bfs::path(qi::os::mktmpdir("optSdk"), qi::unicodeFacet());
  optSdkShareFoo = optSdkPrefix / "share" / "foo";
  listing = "bar.dat\n"
            "foo.dat\n"
            "bar/baz.dat\n"
            "bam/baz.dat\n"
            "opt.dat";
  createData(optSdkShareFoo, listing);

  // an other optional complementary sdk dir
  optSdkShareFuu = optSdkPrefix / "share" / "fuu";
  listing = "fuu.dat";
  createData(optSdkShareFuu, listing);

  // the user dir
  userShareFooBazDat = sdkl.userWritableDataPath("foo", "baz.dat");
  writeData(userShareFooBazDat.string(qi::unicodeFacet()));
  userShareFooFooDat = sdkl.userWritableDataPath("foo", "foo.dat");
  writeData(userShareFooFooDat.string(qi::unicodeFacet()));
  userShareFooUserDat = sdkl.userWritableDataPath("foo", "user.dat");
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
  qi::SDKLayout sdkl;
  std::string actual = sdkl.findData("foo", "bar/baz.dat");
  std::string expected = (sdkShareFoo / "bar/baz.dat").make_preferred().string(qi::unicodeFacet());
  ASSERT_EQ(expected, actual);
}

TEST_F(qiPathData, listDataWithOptSdk)
{
  qi::SDKLayout sdkl;
  sdkl.addOptionalSdkPrefix(optSdkPrefix.string(qi::unicodeFacet()).c_str());

  std::vector<std::string> actual = sdkl.listData("foo", "*.dat");
  EXPECT_EQ(6u, actual.size());

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

  actual = sdkl.listData("fuu", "*.dat");
  EXPECT_EQ(1u, actual.size());

  // fuu.dat is only available in sdk
  EXPECT_TRUE(isInVector(optSdkShareFuu / "fuu.dat", actual));
}

TEST_F(qiPathData, listDataInSubFolder)
{
  qi::SDKLayout sdkl;
  std::string expected = (sdkShareFoo / "bar/baz.dat").make_preferred().string(qi::unicodeFacet());
  std::vector<std::string> actual;

  actual = sdkl.listData("foo", "bar/baz.dat");
  ASSERT_EQ(1u, actual.size());
  EXPECT_EQ(actual[0], expected);

#ifdef _WIN32
  actual = sdkl.listData("foo", "bar\\baz.dat");
  ASSERT_EQ(1u, actual.size());
  EXPECT_EQ(actual[0], expected);
#endif

  actual = sdkl.listData("foo", "*bar/baz.dat");
  ASSERT_EQ(1u, actual.size());
  EXPECT_EQ(actual[0], expected);

  actual = sdkl.listData("foo", "/bar/baz.dat");
  ASSERT_EQ(1u, actual.size());
  EXPECT_EQ(actual[0], expected);

  // Application names ending with "/" are not really supported.
  // However, that would work with findData, so let support it with listData
  // too.
  actual = sdkl.listData("foo/", "bar/baz.dat");
  ASSERT_EQ(1u, actual.size());
  EXPECT_EQ(actual[0], expected);

  // Well, in the following case, findData would work but listData does not
  // because the string "foo//bar/baz.dat" does not match "foo/bar/baz.dat".
  // A solution would be to normalize paths in fsconcat, but that would have
  // consequences on all qi::path.
  //actual = sdkl.listData("foo/", "/bar/baz.dat");
  //ASSERT_EQ(1, actual.size());
  //EXPECT_EQ(actual[0], expected);

  actual = sdkl.listData("foo", "/*bar/baz.dat");
  ASSERT_EQ(1u, actual.size());
  EXPECT_EQ(actual[0], expected);

  actual = sdkl.listData("foo", "*/bar/baz.dat");
  EXPECT_EQ(0u, actual.size());
}

TEST_F(qiPathData, listDataInSubFolderWithOptSdk)
{
  qi::SDKLayout sdkl;
  sdkl.addOptionalSdkPrefix(optSdkPrefix.string(qi::unicodeFacet()).c_str());

  std::vector<std::string> actual = sdkl.listData("foo", "ba?/*.dat");
  EXPECT_EQ(3u, actual.size());

  EXPECT_TRUE(isInVector(sdkShareFoo / "bar/baz.dat", actual));
  EXPECT_TRUE(isInVector(sdkShareFoo / "baz/baz.dat", actual));
  EXPECT_TRUE(isInVector(optSdkShareFoo / "bam/baz.dat", actual));
}

TEST_F(qiPathData, findDataExcludingUserDataPath)
{
  qi::SDKLayout sdkl;
  std::string actual = sdkl.findData("foo", "baz.dat", true);
  std::string expected = (sdkShareFoo / "baz.dat").make_preferred().string(qi::unicodeFacet());
  ASSERT_EQ(expected, actual);
}


TEST_F(qiPathData, findDataDir)
{
  qi::SDKLayout sdkl;
  std::string expected = (sdkShareFoo / "bar").make_preferred().string(qi::unicodeFacet());

  std::string barDir = sdkl.findData("foo", "bar");
  EXPECT_EQ(barDir, expected);

  std::vector<std::string> barDirMatches = sdkl.listData("foo", "bar");
  EXPECT_TRUE(barDirMatches.empty()); // listData discards directories
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

// a fixture which creates a file in unicode path and ensure it is
// compatible to native API
class qiPathDos: public ::testing::Test
{
protected:
  static void createFile(const bfs::path& path, const std::string& content);

protected:
  static std::string _folderNameASCII;
  static std::string _fileNameASCII;
  static bfs::path _folderPathASCII;

  static std::string _folderNameUnicode;
  static std::string _fileNameUnicode;
  static bfs::path _folderPathUnicode;

  static std::string _testString;
};

void qiPathDos::createFile(const bfs::path& path, const std::string& content)
{
  FILE* f = qi::os::fopen(path.string(qi::unicodeFacet()).c_str(), "w+");
  fprintf(f, "%s", content.c_str());
  fflush(f);
  fclose(f);
}

std::string qiPathDos::_folderNameASCII;
std::string qiPathDos::_fileNameASCII;
bfs::path qiPathDos::_folderPathASCII;

std::string qiPathDos::_folderNameUnicode;
std::string qiPathDos::_fileNameUnicode;
bfs::path qiPathDos::_folderPathUnicode;

std::string qiPathDos::_testString;

class qiPathFrench: public qiPathDos
{
protected:
  static void SetUpTestCase();
  static void TearDownTestCase();
};

void qiPathFrench::SetUpTestCase()
{
#ifdef _WIN32
  SetThreadLocale(0x000C); // fr-FR
#else
  setlocale(LC_ALL, "fr_FR");
#endif

  _folderNameASCII = "dossier";
  _fileNameASCII = "fichier.txt";
  _folderPathASCII = bfs::path(qi::os::tmp()) / bfs::path(_folderNameASCII);

  _folderNameUnicode = "dôssïér";
  _fileNameUnicode = "fîçhïèr.txt";
  _folderPathUnicode = bfs::path(qi::os::tmp()) / bfs::path(_folderNameUnicode,
                                                            qi::unicodeFacet());

  _testString = std::string("toutvabien");

  // create text files with string to read
  bfs::path filePath = _folderPathASCII / bfs::path(_fileNameASCII);
  bfs::create_directory(_folderPathASCII);
  qiPathDos::createFile(filePath, _testString);

  filePath = _folderPathUnicode / bfs::path(_fileNameUnicode,
                                            qi::unicodeFacet());
  bfs::create_directory(_folderPathUnicode);
  qiPathDos::createFile(filePath, _testString);
}

void qiPathFrench::TearDownTestCase()
{
  std::cout << "removing: " << _folderPathASCII << std::endl;
  bfs::remove_all(_folderPathASCII);

  std::cout << "removing: " << _folderPathUnicode << std::endl;
  bfs::remove_all(_folderPathUnicode);
}

TEST_F(qiPathFrench, convertFromASCII)
{
  bfs::path path = _folderPathASCII / bfs::path(_fileNameASCII);
  const qi::Path pathStr = qi::path::convertToDosPath(path.string());

  std::cout << "path: " << pathStr << std::endl;

  bfs::ifstream ifs(pathStr);
  ASSERT_TRUE(ifs.is_open());

  std::string line;
  ifs >> line;
  ASSERT_EQ(_testString, line);
}

TEST_F(qiPathFrench, convertFromUnicode)
{
  bfs::path path = _folderPathUnicode / bfs::path(_fileNameUnicode,
                                                  qi::unicodeFacet());

  // try with default system conversion
  std::string pathStr = path.string();
  std::cout << "path: " << pathStr.c_str() << std::endl;
  std::ifstream ifs(pathStr.c_str());
  // All given chars have an equivalent in both CP-1252 and ISO 8859-1 so it works
  EXPECT_TRUE(ifs.is_open());

  // try with unicode conversion
  pathStr = path.string(qi::unicodeFacet());
  std::cout << "path: " << pathStr.c_str() << std::endl;
  std::ifstream ifs2(pathStr.c_str());
  // this is showing that even if try unicode charset to decode the path,
  // results are differents between win32 and unix because some characters are represented using more than 1 byte
#ifdef _WIN32
  EXPECT_FALSE(ifs2.is_open());
#else
  EXPECT_TRUE(ifs2.is_open());
#endif

  // try with DOS conversion
  pathStr = qi::path::convertToDosPath(path.string(qi::unicodeFacet()));
  std::cout << "path: " << pathStr.c_str() << std::endl;
  std::ifstream ifs3(pathStr.c_str());
  ASSERT_TRUE(ifs3.is_open());

  std::string line;
  ifs3 >> line;
  ASSERT_EQ(_testString, line);
}

class qiPathChinese: public qiPathDos
{
protected:
  static void SetUpTestCase();
  static void TearDownTestCase();
};

void qiPathChinese::SetUpTestCase()
{
#ifdef _WIN32
  SetThreadLocale(0x0804); // zh-CN
#else
  setlocale(LC_ALL, "zh_CN");
#endif

  _folderNameASCII = "folder";
  _fileNameASCII = "file.txt";
  _folderPathASCII = bfs::path(qi::os::tmp()) / bfs::path(_folderNameASCII);

  _folderNameUnicode = "夾"; // chinese translation for "folder"
  _fileNameUnicode = "文件.txt"; // chinese translation for "file"
  _folderPathUnicode = bfs::path(qi::os::tmp()) / bfs::path(_folderNameUnicode,
                                                            qi::unicodeFacet());

  _testString = std::string("一切都很好"); // chinese translation of "tout va bien"

  // create text files with string to read
  bfs::path filePath = _folderPathASCII / bfs::path(_fileNameASCII);
  bfs::create_directory(_folderPathASCII);
  createFile(filePath, _testString);

  filePath = _folderPathUnicode / bfs::path(_fileNameUnicode,
                                            qi::unicodeFacet());
  bfs::create_directory(_folderPathUnicode);
  createFile(filePath, _testString);
}

void qiPathChinese::TearDownTestCase()
{
  std::cout << "removing: " << _folderPathASCII << std::endl;
  bfs::remove_all(_folderPathASCII);

  std::cout << "removing: " << _folderPathUnicode << std::endl;
  bfs::remove_all(_folderPathUnicode);
}

TEST_F(qiPathChinese, convertFromASCII)
{
  bfs::path path = _folderPathASCII / bfs::path(_fileNameASCII);
  std::string pathStr = qi::path::convertToDosPath(path.string());

  std::cout << "path: " << pathStr.c_str() << std::endl;

  std::ifstream ifs(pathStr.c_str());
  ASSERT_TRUE(ifs.is_open());

  std::string line;
  ifs >> line;
  ASSERT_EQ(_testString, line);
}

TEST_F(qiPathChinese, convertFromUnicode)
{
  bfs::path path = _folderPathUnicode / bfs::path(_fileNameUnicode,
                                                  qi::unicodeFacet());

  // try with default system conversion
  std::string pathStr = path.string();
  std::cout << "path: " << pathStr.c_str() << std::endl;
  std::ifstream ifs(pathStr.c_str());
  // as unix doesn't care of characters but bytes, charset doesn't care, it will success anyway
  // on Windows, as characters are not present un default charset, it fails anyway
#ifdef _WIN32
  EXPECT_FALSE(ifs.is_open());
#else
  EXPECT_TRUE(ifs.is_open());
#endif

  // try with unicode conversion
  pathStr = path.string(qi::unicodeFacet());
  std::cout << "path: " << pathStr.c_str() << std::endl;
  std::ifstream ifs2(pathStr.c_str());
  // this is showing that even if try unicode charset to decode the path,
  // results are differents between win32 and unix because some characters are represented using more than 1 byte
#ifdef _WIN32
  EXPECT_FALSE(ifs2.is_open());
#else
  EXPECT_TRUE(ifs2.is_open());
#endif

  // try with DOS conversion
  pathStr = qi::path::convertToDosPath( path.string(qi::unicodeFacet()));
  std::cout << "path: " << pathStr.c_str() << std::endl;
  std::ifstream ifs3(pathStr.c_str());
  ASSERT_TRUE(ifs3.is_open());

  std::string line;
  ifs3 >> line;
  ASSERT_EQ(_testString, line);
}

TEST(qiPathClass, testPimpl)
{
  qi::Path a("a");
  qi::Path aa(a);
  qi::Path aaa = aa;
  qi::Path b("b");
  EXPECT_EQ("a", a.str());
  EXPECT_EQ("a", aa.str());
  a /= b;
  EXPECT_NE("a", a.str());  //value are different on windows/linux so... just testing it's not "a"
  EXPECT_EQ("a",   aa.str());
  EXPECT_EQ("a",   aaa.str());
}

TEST(qiPathClass, canBeLogged)
{
  const qi::Path path("kikoo.lol");
  std::cout << "Logging path: " << path << std::endl;
  qiLogInfo() << "Logging path: " << path;
  std::cout << "Logging absolute path: " << path.absolute() << std::endl;
  qiLogInfo() << "Logging absolute path: " << path.absolute();
}

class qiPathLib : public ::testing::Test
{
protected:
  static void SetUpTestCase();
  static void TearDownTestCase();
  static bfs::path sdkLibPath;
};

bfs::path qiPathLib::sdkLibPath;

void qiPathLib::SetUpTestCase()
{
  qi::SDKLayout sdkl;

  // the sdk dir of the program that is currently running
  bfs::path prefix(sdkl.sdkPrefix(), qi::unicodeFacet());
  qiPathLib::sdkLibPath = prefix / "lib";
  std::string listing = "test.so\n"
                        "test.dll\n"
                        "test.a\n"
                        "test.dylib\n"
                        "bar/baz.dat\n";
  createData(sdkLibPath, listing);
}

void qiPathLib::TearDownTestCase()
{
  qi::SDKLayout sdkl;

  // the sdk dir of the program that is currently running
  bfs::path prefix(sdkl.sdkPrefix(), qi::unicodeFacet());
  qiPathLib::sdkLibPath = prefix / "lib";

  std::vector<std::string> files;
  files.push_back((sdkLibPath / "test.so").make_preferred().string(qi::unicodeFacet()));
  files.push_back((sdkLibPath / "test.dll").make_preferred().string(qi::unicodeFacet()));
  files.push_back((sdkLibPath / "test.a").make_preferred().string(qi::unicodeFacet()));
  files.push_back((sdkLibPath / "test.dylib").make_preferred().string(qi::unicodeFacet()));
  files.push_back((sdkLibPath / "bar/baz.dat").make_preferred().string(qi::unicodeFacet()));

  for (unsigned i = 0; i < files.size(); ++i)
  {
    std::string file = files.at(i);
    std::cout << "removing: " << file << std::endl;
    bfs::remove_all(file);
  }
}

TEST_F(qiPathLib, listLib)
{
  qi::SDKLayout sdkl;
  bfs::path prefix(sdkl.sdkPrefix(), qi::unicodeFacet());

  std::vector<std::string> expected;

#ifdef __APPLE__
  expected.push_back((prefix / "lib" / "test.dylib").make_preferred().string(qi::unicodeFacet()));
#endif
#ifndef _WIN32
  expected.push_back((prefix / "lib" / "test.so").make_preferred().string(qi::unicodeFacet()));
#endif
#ifdef _WIN32
  expected.push_back((prefix / "lib" / "test.dll").make_preferred().string(qi::unicodeFacet()));
#endif

  std::vector<std::string> actual = sdkl.listLib("", "test.*");
  ASSERT_EQ(expected, actual);
}

TEST(qiPath, ScopedDir)
{
  qi::Path tmpPath;
  {
    qi::path::ScopedDir tmpdir;
    tmpPath = tmpdir.path();
    ASSERT_TRUE(tmpPath.exists());
  }
  ASSERT_FALSE(tmpPath.exists());
}

int main(int argc, char* argv[])
{
  // qibuild sets this variable, but it's easier to test qipath without it
  qi::os::setenv("QI_ADDITIONAL_SDK_PREFIXES", "");
  qi::Application app(argc, argv);
  ::testing::InitGoogleTest(&argc, argv);
#ifdef _WIN32
  {
    std::vector<WCHAR> filename(2048, 0);
    GetModuleFileNameW(NULL, filename.data(), filename.size());
    argpath = bfs::path(filename);
  }
#else
  argpath = bfs::path(argv[0]);
#endif

  return RUN_ALL_TESTS();
}
