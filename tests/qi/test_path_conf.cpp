#include <gtest/gtest.h>
#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>

#include <qi/path.hpp>
#include <qi/os.hpp>
#include <qi/path_conf.hpp>


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
  const boost::filesystem::path foo_sdk = _tmp / "foo/sdk";
  boost::filesystem::path foo_path_conf = _tmp / "foo/sdk/share/qi/";
  boost::filesystem::create_directories(foo_path_conf);
  foo_path_conf /= "path.conf";
  boost::filesystem::ofstream ofs(foo_path_conf);
  ofs << "# This is a test" << std::endl
      << "" << std::endl
      << foo_sdk.string()
      << std::endl;
  ofs.close();
  std::vector<std::string> actual = qi::path::parseQiPathConf(foo_sdk.string());
  std::vector<std::string> expected;
  expected.push_back(foo_sdk.string());
  ASSERT_EQ(actual, expected);
}

TEST_F(PathConfTest, RecursiveTest)
{
  // bar depends on foo,
  // foo's path.conf contains some path in foo sources
  const boost::filesystem::path foo_sdk = _tmp / "foo/sdk";
  const boost::filesystem::path foo_src = _tmp / "foo/src";
  boost::filesystem::create_directories(foo_src);
  const boost::filesystem::path bar_sdk = _tmp / "bar/sdk";
  boost::filesystem::path foo_path_conf = _tmp / "foo/sdk/share/qi/";
  boost::filesystem::path bar_path_conf = _tmp / "bar/sdk/share/qi/";
  boost::filesystem::create_directories(foo_path_conf);
  boost::filesystem::create_directories(bar_path_conf);
  foo_path_conf /= "path.conf";
  bar_path_conf /= "path.conf";
  boost::filesystem::ofstream ofs(foo_path_conf);
  ofs << "# This is foo/sdk/path.conf" << std::endl
      << "" << std::endl
      << foo_sdk.string() << std::endl
      << foo_src.string() << std::endl;
  ofs.close();
  ofs.open(bar_path_conf);
  ofs << "# This is a bar/sdk/path.conf" << std::endl
      << "" << std::endl
      << foo_sdk.string() << std::endl;
  ofs.close();
  std::vector<std::string> actual = qi::path::parseQiPathConf(bar_sdk.string());
  std::vector<std::string> expected;
  expected.push_back(foo_sdk.string());
  expected.push_back(foo_src.string());
  ASSERT_EQ(actual, expected);
}

TEST_F(PathConfTest, CircularTest)
{
  // bar depends on foo,
  // and foo depends on bar ...
  const boost::filesystem::path foo_sdk = _tmp / "foo/sdk";
  const boost::filesystem::path bar_sdk = _tmp / "bar/sdk";
  boost::filesystem::path foo_path_conf = _tmp / "foo/sdk/share/qi/";
  boost::filesystem::path bar_path_conf = _tmp / "bar/sdk/share/qi/";
  boost::filesystem::create_directories(foo_path_conf);
  boost::filesystem::create_directories(bar_path_conf);
  foo_path_conf /= "path.conf";
  bar_path_conf /= "path.conf";
  boost::filesystem::ofstream ofs(foo_path_conf);
  ofs << "# This foo/sdk/bar.conf" << std::endl
      << bar_sdk.string() << std::endl;
  ofs.close();
  ofs.open(bar_path_conf);
  ofs << "# This is a bar/sdk/path.conf" << std::endl
      << "" << std::endl
      << foo_sdk.string() << std::endl;
  ofs.close();
  std::vector<std::string> actual = qi::path::parseQiPathConf(bar_sdk.string());
  std::vector<std::string> expected;
  expected.push_back(foo_sdk.string());
  expected.push_back(bar_sdk.string());
  ASSERT_EQ(actual, expected);
}

TEST_F(PathConfTest, KeepOrderTest)
{
  const boost::filesystem::path fooPath = _tmp / "foo";
  boost::filesystem::path pathConf = fooPath / "share/qi/";
  boost::filesystem::create_directories(pathConf);
  pathConf /= "path.conf";
  const boost::filesystem::path aPath = _tmp / "a";
  const boost::filesystem::path bPath = _tmp / "b";
  boost::filesystem::create_directories(aPath);
  boost::filesystem::create_directories(bPath);
  boost::filesystem::ofstream ofs(pathConf);
  ofs << bPath.string() << std::endl
      << aPath.string() << std::endl;
  std::vector<std::string> expected;
  expected.push_back(bPath.string());
  expected.push_back(aPath.string());
  std::vector<std::string> actual = qi::path::parseQiPathConf(fooPath.string());
  ASSERT_EQ(actual, expected);
}
