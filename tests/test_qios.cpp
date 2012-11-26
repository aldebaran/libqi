/*
 * Copyright (c) 2012 Aldebaran Robotics. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the COPYING file.
 */

#ifdef WIN32
# include <winsock2.h>
# include <process.h>  // for getpid
#else
# include <arpa/inet.h>
# include <unistd.h> // for getpid
#endif
#ifdef ANDROID
# include <sys/socket.h>
#endif

#include <fstream>
#include <cstdio>

#include <gtest/gtest.h>
#include <boost/filesystem.hpp>

#include <qi/path.hpp>
#include <qi/os.hpp>
#include <qi/qi.hpp>
#include <qi/os.hpp>

#ifdef _MSC_VER
# pragma warning( push )
// truncation of constant value when building char* objects
# pragma warning( disable : 4309 )
#endif
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

// "" in utf-8 w. french locale.
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

TEST(QiOs, msleep)
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

  EXPECT_TRUE(boost::filesystem::exists(p));
  EXPECT_TRUE(boost::filesystem::is_directory(p));

  pp.append("tmpfile", qi::unicodeFacet());
  tempfile = pp.string(qi::unicodeFacet());

  FILE *f = qi::os::fopen(tempfile.c_str(), "w+");
  EXPECT_TRUE(f != NULL);
  fclose(f);

  EXPECT_TRUE(boost::filesystem::exists(pp));
  EXPECT_TRUE(boost::filesystem::is_regular_file(pp));

  boost::filesystem::directory_iterator it(p);

  EXPECT_EQ(pp, it->path());
  it++;

  EXPECT_TRUE((boost::filesystem::directory_iterator() == it));

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

TEST(QiOs, get_host_name)
{
  std::string temp = qi::os::gethostname();
  EXPECT_NE(std::string(), temp);
}

bool freeportbind(unsigned short port, int &sock)
{
  struct sockaddr_in name;
  name.sin_family = AF_INET;
  name.sin_addr.s_addr = htonl(INADDR_ANY);
  sock = ::socket(AF_INET, SOCK_STREAM, 0);
  name.sin_port = htons(port);

  return (::bind(sock, (struct sockaddr *)&name, sizeof(name))) != 0;
}

TEST(QiOs, free_port)
{
  int sock;
  unsigned short port = qi::os::findAvailablePort(9559);
  int success = freeportbind(port, sock);

  if (success == -1)
    EXPECT_TRUE(false);
  else
#ifndef WIN32
    ::close(sock);
#else
    ::closesocket(sock);
#endif

  EXPECT_TRUE(true);
}


TEST(QiOs, free_port_multiple_connection)
{
  int sock1;
  unsigned short port1 = qi::os::findAvailablePort(9559);
  int success1 = freeportbind(port1, sock1);
  int sock2;
  unsigned short port2 = qi::os::findAvailablePort(9559);
  int success2 = freeportbind(port2, sock2);
  int sock3;
  unsigned short port3 = qi::os::findAvailablePort(9559);
  int success3 = freeportbind(port3, sock3);

  if (success1 == -1)
    EXPECT_TRUE(false);
  else
#ifndef WIN32
    ::close(sock1);
#else
    ::closesocket(sock1);
#endif
  if (success2 == -1)
    EXPECT_TRUE(false);
  else
#ifndef WIN32
    ::close(sock2);
#else
    ::closesocket(sock2);
#endif
  if (success3 == -1)
    EXPECT_TRUE(false);
  else
#ifndef WIN32
    ::close(sock3);
#else
    ::closesocket(sock3);
#endif

  EXPECT_TRUE(true);
}

TEST(QiOs, hostIPAddrs)
{
  std::map<std::string, std::vector<std::string> > ifsMap;

  ifsMap = qi::os::hostIPAddrs();
  ASSERT_TRUE(ifsMap.empty() == false);
}

TEST(QiOs, getMachineId)
{
  int status = 0;
  std::string bin = qi::path::findBin("check_machineid");
  int childPid = qi::os::spawnlp(bin.c_str(), NULL);
  ASSERT_NE(-1, childPid);

  int error = qi::os::waitpid(childPid, &status);
  EXPECT_EQ(0, error);
  EXPECT_EQ(0, status);

  std::string uuid1 = qi::os::getMachineId();
  std::string uuid2;

  std::string uuid2FileName = (qi::os::tmp()).append("machine_id_test_42");
  std::ifstream uuid2file(uuid2FileName.c_str());

  ASSERT_TRUE(uuid2file != NULL);

  uuid2file >> uuid2;
  uuid2file.close();

  ASSERT_TRUE(uuid1.compare(uuid2) == 0);
}

#ifdef _MSC_VER
# pragma warning( pop )
#endif
