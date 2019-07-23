/*
 * Copyright (c) 2012, 2013 Aldebaran Robotics. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the COPYING file.
 */

#ifdef _WIN32
# include <winsock2.h>
# include <process.h>  // for _getpid
# include <windows.h>
#else
# include <arpa/inet.h>
# include <unistd.h> // for getpid
#endif
#ifdef ANDROID
# include <sys/socket.h>
#endif
#include <limits>
#include <boost/filesystem/fstream.hpp>
#include <cstdio>
#include <future>

#include <gtest/gtest.h>
#include <boost/filesystem.hpp>
#include <boost/uuid/uuid_io.hpp>

#include <qi/path.hpp>
#include <qi/os.hpp>
#include <ka/macro.hpp>
#include <ka/scoped.hpp>
#include "testutils/testutils.hpp"

static std::chrono::milliseconds timeout()
{
  return std::chrono::milliseconds{1000};
}

KA_WARNING_PUSH()
// truncation of constant value when building char* objects
KA_WARNING_DISABLE(4309, )

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
const char QiOSTests::a_accent[4] =  { (char) '/', (char) 0xc3, (char) 0xa9, (char) '\0' } ;

TEST_F(QiOSTests, LowLevelAccent)
{
  // Try to retrieve the file.
  // The name must be in utf-8 to be retrieved on unix.
  ASSERT_TRUE(boost::filesystem::exists(a_newPath))
    << a_newPath.string() << std::endl;
}

TEST(QiOs, fnmatch)
{
  EXPECT_TRUE(qi::os::fnmatch("fnmatch", "fnmatch"));
  EXPECT_FALSE(qi::os::fnmatch("fnmatc", "fnmatch"));
  EXPECT_TRUE(qi::os::fnmatch("fn*ch", "fnmatch"));
  EXPECT_FALSE(qi::os::fnmatch("fz*ch", "fnmatch"));
  EXPECT_TRUE(qi::os::fnmatch("fn???ch", "fnmatch"));
  EXPECT_FALSE(qi::os::fnmatch("fn?z?ch", "fnmatch"));
}

// TODO: us qi::time when it's available :)
TEST(QiOs, sleep)
{
KA_WARNING_PUSH()
KA_WARNING_DISABLE(4996, deprecated-declarations) // ignore use of deprecated overloads.
  qi::os::sleep(1);
KA_WARNING_POP()
}

TEST(QiOs, msleep)
{
KA_WARNING_PUSH()
KA_WARNING_DISABLE(4996, deprecated-declarations) // ignore use of deprecated overloads.
  qi::os::msleep(1000);
KA_WARNING_POP()
}

TEST(QiOs, currentThreadName)
{
#if !defined(_WIN32)
  qi::os::setCurrentThreadName("Iamathread");
  ASSERT_EQ(std::string("Iamathread"), qi::os::currentThreadName());
#endif
}

TEST(QiOs, MemoryUsage)
{
  ASSERT_TRUE(qi::os::memoryUsage(qi::os::getpid()) > 0);
  ASSERT_EQ(0u, qi::os::memoryUsage(0));
}

TEST(QiOs, timeValCtor)
{
  qi::os::timeval t0 = qi::os::timeval();
  // t0 members are initialized to 0
  EXPECT_EQ(0, t0.tv_sec);
  EXPECT_EQ(0, t0.tv_usec);
  qi::os::timeval t1;
  // t1 members are initialized to 0
  EXPECT_EQ(0, t1.tv_sec);
  EXPECT_EQ(0, t1.tv_usec);

  // t2 members are not normalized
  qi::os::timeval t2(-1, -2);
  EXPECT_EQ(-1, t2.tv_sec);
  EXPECT_EQ(-2, t2.tv_usec);

  qi::Seconds s(123456789);
  qi::MicroSeconds us(123456);
  qi::NanoSeconds ns(789);
  qi::Duration d_us(s + us);
  qi::Duration d_ns(d_us + ns);
  qi::int64_t normalized_s = s.count();
  qi::int64_t normalized_us = us.count();

  // positive
  {
    qi::os::timeval tv0(d_us);
    EXPECT_EQ(normalized_s, tv0.tv_sec);
    EXPECT_EQ(normalized_us, tv0.tv_usec);

    qi::Duration d_back = qi::Seconds(tv0.tv_sec) +
                          qi::MicroSeconds(tv0.tv_usec);
    EXPECT_TRUE(d_us - d_back < qi::MicroSeconds(1));
    qi::os::timeval tv1(d_us);
    EXPECT_EQ(normalized_s, tv1.tv_sec);
    EXPECT_EQ(normalized_us, tv1.tv_usec);
  }
  // negative
  d_us = -d_us;
  d_ns = -d_ns;
  normalized_s = -s.count() - 1;
  normalized_us = -us.count() + 1000000;
  {
    qi::os::timeval tv0(d_us);
    EXPECT_EQ(normalized_s, tv0.tv_sec);
    EXPECT_EQ(normalized_us, tv0.tv_usec);

    qi::Duration d_back = qi::Seconds(tv0.tv_sec) +
                          qi::MicroSeconds(tv0.tv_usec);
    EXPECT_TRUE(d_us - d_back < qi::MicroSeconds(1));
    qi::os::timeval tv1(d_us);
    EXPECT_EQ(normalized_s, tv1.tv_sec);
    EXPECT_EQ(normalized_us, tv1.tv_usec);
  }
}



TEST(QiOs, timeValOperator)
{
  qi::os::timeval t0;
  t0.tv_sec = 2000;
  t0.tv_usec = 999999;

  qi::os::timeval t1;
  t1.tv_sec = 2000;
  t1.tv_usec = 2000;

  qi::os::timeval t2;
  t2.tv_sec = 10;
  t2.tv_usec = 10;

  // same values as t2, but not normalized
  qi::int64_t delta = 4;
  qi::os::timeval t2a;
  t2a.tv_sec = 10 - delta;
  t2a.tv_usec = 10 + delta*1000*1000;

  delta = -1;
  qi::os::timeval t2b;
  t2b.tv_sec = 10 - delta;
  t2b.tv_usec = 10 + delta*1000*1000;

  delta = 4000;
  qi::os::timeval t2c;
  t2c.tv_sec = 10 - delta;
  t2c.tv_usec = 10 + delta*1000*1000;

  delta = std::numeric_limits<qi::int64_t>::max()/(1000*1000);
  qi::os::timeval t2d;
  t2d.tv_sec = 10 - delta;
  t2d.tv_usec = 10 + delta*1000*1000;

  qi::os::timeval res;

  res = t0 + t2;
  EXPECT_EQ(2011, res.tv_sec);
  EXPECT_EQ(9, res.tv_usec);

  res = t1 + t2;
  EXPECT_EQ(2010, res.tv_sec);
  EXPECT_EQ(2010, res.tv_usec);

  res = t1 + t2a;
  EXPECT_EQ(2010, res.tv_sec);
  EXPECT_EQ(2010, res.tv_usec);
  res = t1 + t2b;
  EXPECT_EQ(2010, res.tv_sec);
  EXPECT_EQ(2010, res.tv_usec);
  res = t1 + t2c;
  EXPECT_EQ(2010, res.tv_sec);
  EXPECT_EQ(2010, res.tv_usec);
  res = t1 + t2d;
  EXPECT_EQ(2010, res.tv_sec);
  EXPECT_EQ(2010, res.tv_usec);

  // check overflow, when summing two (not normalized) values with
  // lots of usec
  // Won't pass because currently the implementation does not
  // normalize before summing.
  // res = t2d + t2d;
  // EXPECT_EQ(20, res.tv_sec);
  // EXPECT_EQ(20, res.tv_usec);

  res =  t1 - t2;
  EXPECT_EQ(1990, res.tv_sec);
  EXPECT_EQ(1990, res.tv_usec);

  res = t2 - t1;
  // result gets normalized
  // -1990*1000*1000 - 1990 == -1991*1000*1000 + 1000*1000 - 1990
  EXPECT_EQ(-1991, res.tv_sec);
  EXPECT_EQ(1000*1000 - 1990, res.tv_usec);

  res = t2 + 10L;
  EXPECT_EQ(10, res.tv_sec);
  EXPECT_EQ(20, res.tv_usec);

  res = t2 + 1000000L;
  ASSERT_EQ(11, res.tv_sec);
  ASSERT_EQ(10, res.tv_usec);

  res = t2 - 10L;
  EXPECT_EQ(10, res.tv_sec);
  EXPECT_EQ(0, res.tv_usec);

  res = t2 - 1000000L;
  ASSERT_EQ(9, res.tv_sec);
  ASSERT_EQ(10, res.tv_usec);
}

TEST(QiOs, DISABLED_timeValOperator)
{
  qi::int64_t delta = std::numeric_limits<qi::int64_t>::max()/(1000*1000);
  qi::os::timeval t2d;
  t2d.tv_sec = 10 - delta;
  t2d.tv_usec = 10 + delta*1000*1000;

  qi::os::timeval res;

  // check overflow, when summing two (not normalized) values with
  // lots of usec
  // Won't pass because currently the implementation does not
  // normalize before summing.
  res = t2d + t2d;
  EXPECT_EQ(20, res.tv_sec);
  EXPECT_EQ(20, res.tv_usec);
}

TEST(QiOs, env)
{
  int ret = qi::os::setenv("TITI", "TUTU");
  ASSERT_FALSE(ret);
  EXPECT_EQ("TUTU", qi::os::getenv("TITI"));
}

TEST(QiOs, envParam)
{
  int ret = qi::os::setenv("TITI", "45");
  ASSERT_FALSE(ret);
  EXPECT_EQ(45, qi::os::getEnvParam<int>("TITI", -1));
  EXPECT_EQ('z', qi::os::getEnvParam<char>("TOTO", 'z'));
}

TEST(QiOs, Unsetenv)
{
  ASSERT_EQ(0, qi::os::setenv("cookies", "good"));
  EXPECT_EQ("good", qi::os::getenv("cookies"));
  ASSERT_EQ(0, qi::os::unsetenv("cookies"));
  EXPECT_TRUE(qi::os::getenv("cookies").empty());
}

TEST(QiOs, UnsetenvInexistentVariable)
{
  ASSERT_EQ(0, qi::os::unsetenv("a_variable_name_that_probably_does_not_exist"));
}

TEST(QiOs, UnsetenvTwice)
{
  ASSERT_EQ(0, qi::os::setenv("cookies", "good"));
  ASSERT_EQ(0, qi::os::unsetenv("cookies"));
  ASSERT_EQ(0, qi::os::unsetenv("cookies"));
}

TEST(QiOs, getpid)
{
// getpid is deprecated on windows, we should use _getpid instead
#ifdef _WIN32
  ASSERT_EQ(_getpid(), qi::os::getpid());
#else
  ASSERT_EQ(getpid(), qi::os::getpid());
#endif
}

TEST(QiOs, Home)
{
#ifdef ANDROID
  // Home always fails on Android as it is not accessible.
  EXPECT_THROW(qi::os::home(), std::runtime_error);
#else
  // On any other platform, this should not throw.
  EXPECT_NO_THROW(qi::os::home());
#endif
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
  char utf8[]     = { (char) 0xC5, (char) 0xAA, (char) 0x6E, (char) 0xC4, (char) 0xAD, (char) 0x63, (char) 0xC5, (char) 0x8D, (char) 0x64, (char) 0x65, (char) 0xCC, (char) 0xBD, (char) 0 };

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
#ifdef ANDROID
  EXPECT_THROW(qi::os::gethostname(), std::runtime_error);
#else
  std::string temp = qi::os::gethostname();
  EXPECT_NE(std::string(), temp);
#endif
}

bool freeportbind(unsigned short port, int &sock)
{
  struct sockaddr_in name;
  name.sin_family = AF_INET;
  name.sin_addr.s_addr = htonl(INADDR_ANY);
  sock = static_cast<int>(::socket(AF_INET, SOCK_STREAM, 0));
  name.sin_port = htons(port);

  return (::bind(sock, (struct sockaddr *)&name, sizeof(name))) != 0;
}

TEST(QiOs, free_port)
{
  int sock;
  unsigned short port = qi::os::findAvailablePort(9559);
  ASSERT_GT(port, 0);
  int success = freeportbind(port, sock);

  if (success == -1)
    EXPECT_TRUE(false);
  else
#ifndef _WIN32
    ::close(sock);
#else
    ::closesocket(sock);
#endif

  EXPECT_TRUE(true);
}

TEST(QiOs, free_port2)
{
  int sock;
  unsigned short port = qi::os::findAvailablePort(54010);
  ASSERT_GT(port, 0);
  int success = freeportbind(port, sock);

  if (success == -1)
    EXPECT_TRUE(false);
  else
#ifndef _WIN32
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
  ASSERT_GT(port1, 0);
  int success1 = freeportbind(port1, sock1);
  int sock2;
  unsigned short port2 = qi::os::findAvailablePort(9559);
  ASSERT_GT(port2, 0);
  int success2 = freeportbind(port2, sock2);
  int sock3;
  unsigned short port3 = qi::os::findAvailablePort(9559);
  ASSERT_GT(port3, 0);
  int success3 = freeportbind(port3, sock3);

  if (success1 == -1)
    EXPECT_TRUE(false);
  else
#ifndef _WIN32
    ::close(sock1);
#else
    ::closesocket(sock1);
#endif
  if (success2 == -1)
    EXPECT_TRUE(false);
  else
#ifndef _WIN32
    ::close(sock2);
#else
    ::closesocket(sock2);
#endif
  if (success3 == -1)
    EXPECT_TRUE(false);
  else
#ifndef _WIN32
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

TEST(QiOs, concurrentHostIPAddrs)
{
  using IfsMap = std::map<std::string, std::vector<std::string>>;
  const auto n = 200;

  std::vector<std::future<IfsMap>> futureMaps;
  futureMaps.reserve(n);

  for (auto i = 0; i < n; ++i)
  {
    std::cout << i << std::endl;
    futureMaps.push_back(std::async(std::launch::async,
                                    [] () {
      return qi::os::hostIPAddrs(false);
    }));
  }

  for (auto i = 0; i < n; ++i)
  {
    auto &future = futureMaps[i];
    future.wait();
    std::cout << "future " << i << " set" << std::endl;
    auto ifsMap = future.get();
    ASSERT_FALSE(ifsMap.empty());
    for (const auto &it : ifsMap)
    {
      std::cout << it.first << " : " << std::endl;
      for (const auto &tmp : it.second)
      {
        std::cout << tmp << "; ";
      }
      std::cout << std::endl;
    }
  }
}

TEST(QiOs, sequentialHostIPAddrs)
{
  const auto n = 10000;
  for (auto i = 0; i < n; ++i)
  {
    std::cout << i << std::endl;
    auto ifsMap = qi::os::hostIPAddrs(false);
    ASSERT_FALSE(ifsMap.empty());
  }
}

TEST(QiOs, MachineIdIsAlwaysTheSame)
{
  EXPECT_EQ(qi::os::getMachineId(), qi::os::getMachineId());
}

TEST(QiOs, MachineIdIsNotEmpty)
{
  EXPECT_FALSE(qi::os::getMachineId().empty());
}

TEST(QiOs, MachineIdIsNotNull)
{
  EXPECT_NE("00000000-0000-0000-0000-000000000000", qi::os::getMachineId());
}

// On Android, we cannot ensure that a machine-id will be shared among
// different processes, so this test is disabled for this platform.
#ifndef ANDROID
TEST(QiOs, MachineIdIsSharedBetweenProcesses)
{
  int status = 0;
  std::string bin = qi::path::findBin("check_machineid");
  ASSERT_FALSE(bin.empty());
  int childPid = qi::os::spawnlp(bin.c_str(), NULL);
  ASSERT_NE(-1, childPid);

  int error = qi::os::waitpid(childPid, &status);
  EXPECT_EQ(0, error);
  EXPECT_EQ(0, status);

  std::string uuid1 = qi::os::getMachineId();
  std::string uuid2;

  const qi::Path uuid2FileName = (qi::os::tmp()).append("machine_id_test_42");
  boost::filesystem::ifstream uuid2file(uuid2FileName);

  ASSERT_TRUE(uuid2file);

  uuid2file >> uuid2;
  uuid2file.close();

  ASSERT_TRUE(uuid1.compare(uuid2) == 0);
}
#endif

TEST(QiOs, getMachineIdAsUuid)
{
  using namespace qi;
  std::ostringstream ss;
  ss << os::getMachineIdAsUuid();
  EXPECT_EQ(ss.str(), os::getMachineId());
}

TEST(QiOs, getMachineIdAsUuidConst)
{
  using namespace qi;
  EXPECT_EQ(os::getMachineIdAsUuid(), os::getMachineIdAsUuid());
}

TEST(Os, getProcessUuidConstantInAProcess)
{
  using namespace qi::os;
  EXPECT_EQ(getProcessUuid(), getProcessUuid());
}

// To check that the process uuid really changes with a different process, we
// launch a helper executable that prints its process uuid in a file.
// We then read the process uuid from the file, remove the file and assert that
// the read uuid is different than ours.
TEST(Os, getProcessUuidDifferentInDifferentProcesses)
{
  using namespace qi::os;
  const auto parentProcessUuid = getProcessUuid();
  const auto childFilename = to_string(parentProcessUuid) + ".txt";
  {
    test::ScopedProcess process{
      qi::path::findBin("print_process_uuid"), {childFilename}, timeout()};
  }
  const auto _ = ka::scoped([=]() {
    boost::filesystem::remove(childFilename);
  });
  boost::filesystem::ifstream childFile{childFilename};
  qi::Uuid childProcessUuid;
  childFile >> childProcessUuid;
  EXPECT_NE(childProcessUuid, parentProcessUuid);
}

#if  defined(_WIN32) || defined(__linux__)
TEST(QiOs, CPUAffinity)
#else
TEST(QiOs, DISABLED_CPUAffinity)
#endif
{
  std::vector<int> cpus;
  long nprocs_max = -1;

  cpus.push_back(0);
  cpus.push_back(1);
  ASSERT_TRUE(qi::os::setCurrentThreadCPUAffinity(cpus));

  nprocs_max = qi::os::numberOfCPUs();
  ASSERT_TRUE(nprocs_max > 0);

  cpus.clear();
  cpus.push_back(nprocs_max + 1);
  ASSERT_FALSE(qi::os::setCurrentThreadCPUAffinity(cpus));
}

TEST(QiOs, dlerror)
{
  qi::os::dlerror(); // Reset errno value
  const char *error0 = qi::os::dlerror();
  // expect NULL since no error has occurred since initialization
  EXPECT_TRUE(error0 == NULL) << "Expected NULL, got: " << error0;

  qi::os::dlerror(); // Reset errno value
  qi::os::dlopen("failure.so");
  EXPECT_TRUE(qi::os::dlerror() != NULL) << "Expected error got NULL";
  const char *error1 = qi::os::dlerror(); // reset
  // expect NULL since no error has occurred since last dlerror call
  EXPECT_TRUE(error1 == NULL) << "Expected NULL, got: " << error1;

#ifndef __linux__
  // dlclose segfault on linux if an invalid pointer is given
  qi::os::dlerror(); // Reset errno value
  EXPECT_NE(0, qi::os::dlclose((void*) 123));
  const char* error2 = qi::os::dlerror();
  EXPECT_NE((const char*) NULL, error2);
#endif
}

TEST(QiOs, ptrUid)
{
  using namespace qi;
  void* ptr = &ptr;
  const auto ptruid = os::ptrUid(ptr);
  const auto expectedPtrUid = PtrUid(os::getMachineIdAsUuid(), os::getProcessUuid(), ptr);
  ASSERT_EQ(expectedPtrUid, ptruid);
}

KA_WARNING_POP()
