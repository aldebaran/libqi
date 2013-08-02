#include <gtest/gtest.h>
#include <boost/filesystem.hpp>
#include <fstream>

#include <qi/path.hpp>
#include <qi/os.hpp>

#include "src/path_conf.hpp"

class PathConfTest: public ::testing::Test {
  protected:
    virtual void SetUp()
    {
      _tmp = boost::filesystem::path(qi::os::mktmpdir("test-path-conf"));
      boost::filesystem::create_directories(_tmp);
    }

    virtual void TearDown()
    {
      boost::filesystem::remove_all(_tmp);
    }
    boost::filesystem::path _tmp;
};


TEST_F(PathConfTest, SimpleTest)
{
  boost::filesystem::path foo_sdk = _tmp / "foo/sdk";
  boost::filesystem::path foo_path_conf = _tmp / "foo/sdk/share/qi/";
  boost::filesystem::create_directories(foo_path_conf);
  foo_path_conf /= "path.conf";
  std::ofstream ofs(foo_path_conf.string().c_str());
  ofs << "# This is a test" << std::endl
      << "" << std::endl
      << foo_sdk.string()
      << std::endl;
  ofs.close();
  std::set<std::string> actual = qi::path::detail::parseQiPathConf(foo_sdk.string());
  std::set<std::string> expected;
  expected.insert(foo_sdk.string());
  ASSERT_EQ(actual, expected);
}

TEST_F(PathConfTest, RecursiveTest)
{
  // bar depends on foo,
  // foo's path.conf contains some path in foo sources
  boost::filesystem::path foo_sdk = _tmp / "foo/sdk";
  boost::filesystem::path foo_src = _tmp / "foo/src";
  boost::filesystem::create_directories(foo_src);
  boost::filesystem::path bar_sdk = _tmp / "bar/sdk";
  boost::filesystem::path foo_path_conf = _tmp / "foo/sdk/share/qi/";
  boost::filesystem::path bar_path_conf = _tmp / "bar/sdk/share/qi/";
  boost::filesystem::create_directories(foo_path_conf);
  boost::filesystem::create_directories(bar_path_conf);
  foo_path_conf /= "path.conf";
  bar_path_conf /= "path.conf";
  std::ofstream ofs(foo_path_conf.string().c_str());
  ofs << "# This is foo/sdk/path.conf" << std::endl
      << "" << std::endl
      << foo_sdk.string() << std::endl
      << foo_src.string() << std::endl;
  ofs.close();
  ofs.open(bar_path_conf.string().c_str());
  ofs << "# This is a bar/sdk/path.conf" << std::endl
      << "" << std::endl
      << foo_sdk.string() << std::endl;
  ofs.close();
  std::set<std::string> actual = qi::path::detail::parseQiPathConf(bar_sdk.string());
  std::set<std::string> expected;
  expected.insert(foo_sdk.string());
  expected.insert(foo_src.string());
  ASSERT_EQ(actual, expected);
}

TEST_F(PathConfTest, CircularTest)
{
  // bar depends on foo,
  // and foo depends on bar ...
  boost::filesystem::path foo_sdk = _tmp / "foo/sdk";
  boost::filesystem::path bar_sdk = _tmp / "bar/sdk";
  boost::filesystem::path foo_path_conf = _tmp / "foo/sdk/share/qi/";
  boost::filesystem::path bar_path_conf = _tmp / "bar/sdk/share/qi/";
  boost::filesystem::create_directories(foo_path_conf);
  boost::filesystem::create_directories(bar_path_conf);
  foo_path_conf /= "path.conf";
  bar_path_conf /= "path.conf";
  std::ofstream ofs(foo_path_conf.string().c_str());
  ofs << "# This foo/sdk/bar.conf" << std::endl
      << bar_sdk.string() << std::endl;
  ofs.close();
  ofs.open(bar_path_conf.string().c_str());
  ofs << "# This is a bar/sdk/path.conf" << std::endl
      << "" << std::endl
      << foo_sdk.string() << std::endl;
  ofs.close();
  std::set<std::string> actual = qi::path::detail::parseQiPathConf(bar_sdk.string());
  std::set<std::string> expected;
  expected.insert(foo_sdk.string());
  expected.insert(bar_sdk.string());
  ASSERT_EQ(actual, expected);
}
