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
#include <qi/qi.hpp>
#include <qi/os.hpp>
#include <qi/path.hpp>
#include "../src/utils.hpp"

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


TEST(qiPath, dataInSubfolder)
{
  // Ugly hack because Gtest mess up with argc/argv ...
  //std::string binary = std::string(::testing::internal::GetArgvs()[0]);
  //const char* args = { (char *) binary.c_str() };
  qi::SDKLayout* sdkl = new qi::SDKLayout();

  bfs::path prefix(sdkl->sdkPrefix(), qi::unicodeFacet());
  bfs::path fooShare(prefix / "share" / "foo");
  boost::filesystem::create_directories(fooShare / "bar");
  std::string bazData = (fooShare / "bar" / "baz.dat").string(qi::unicodeFacet());
  std::ofstream ofs;
  ofs.open(bazData.c_str(), std::fstream::out | std::fstream::trunc);
  ofs << "This is baz data";
  ofs.close();

  std::string actual = sdkl->findData("foo", "bar/baz.dat");
  std::string expect = bazData;
  boost::to_lower(expect);
  boost::to_lower(actual);

  ASSERT_EQ(expect, actual);

  std::cout << "removing: " << bazData << std::endl;
  remove(bazData.c_str());

  delete sdkl;
}


int main(int argc, char* argv[])
{
  qi::init(argc, argv);
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
