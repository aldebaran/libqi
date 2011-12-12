/**
* @author Julien Fremy
* Copyright (c) 2011 Aldebaran Robotics  All Rights Reserved
*/

#ifdef WIN32
# include <process.h>  // for getpid
#else
# include <unistd.h> // for getpid
#endif

#include <gtest/gtest.h>
#include <boost/filesystem.hpp>

#include <qi/path.hpp>
#include <qi/os.hpp>

class QiOSTests: public ::testing::Test
{
public:
  QiOSTests()
    : a_newPath()
  {
  }

protected:
  void SetUp() {
    a_newPath = qi::os::tmpdir("QiOsTest");
    a_newPath /= a_accent;
    FILE* fileHandle = qi::os::fopen(a_newPath.string().c_str(), "w");
    fclose(fileHandle);
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
  //boost::filesystem::path localPath = mkdtemp("QiOsTestLLAccent");
  //localPath /= a_accent;
  ASSERT_TRUE(boost::filesystem::exists(a_newPath))
    << a_newPath.string() << std::endl
    << a_newPath.string();
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
