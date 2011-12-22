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

#ifdef WIN32
# include <process.h>  // for getpid
#else
# include <unistd.h> // for getpid
#endif

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
    a_newPath = qi::os::tmpdir("QiOsTest");
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
