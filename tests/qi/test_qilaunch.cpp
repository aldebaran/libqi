#include <random>
#include <gtest/gtest.h>
#include <boost/filesystem.hpp>
#include <boost/predef/os.h>
#include <qi/application.hpp>
#include <qi/log.hpp>
#include <qi/os.hpp>
#include <qi/path.hpp>
#include <qi/testutils/testutils.hpp>

#include <errno.h>

#ifdef _WIN32
# include <windows.h>
# include <process.h>
# include <signal.h>
#else
# include <sys/wait.h>
#endif

/*
 * Copyright (c) 2012 Aldebaran Robotics. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the COPYING file.
 */
// TODO2: Some tests to check the consume stack

extern std::string binDir;
extern std::string loopBinDir;

TEST(spawnvp, CmdWithNoArgs)
{
  char* args[] = {(char*)binDir.c_str(), NULL};

  int status = 0;

  int childPid = qi::os::spawnvp(args);
  int error = qi::os::waitpid(childPid, &status);

  EXPECT_EQ(0, error) << "error: " << error;
  EXPECT_EQ(42, status) << "status: " << status;
}

TEST(spawnvp, CmdWithArgs)
{
  char* args[] = {(char*)binDir.c_str(), (char*)"23", NULL};

  int status = 0;

  int childPid = qi::os::spawnvp(args);
  int error = qi::os::waitpid(childPid, &status);

  EXPECT_EQ(0, error) << "error: " << error;
  EXPECT_EQ(23, status) << "status: " << status;
}

TEST(spawnvp, CmdWithMultiArgs)
{
  char* args[] = {(char*)binDir.c_str(), (char*)"23", (char*)"-1", NULL};

  int status = 0;

  int childPid = qi::os::spawnvp(args);
  int error = qi::os::waitpid(childPid, &status);

  EXPECT_EQ(0, error) << "error: " << error;
  EXPECT_EQ(22, status) << "status: " << status;
}

TEST(spawnvp, InvalidBin)
{
  char* args[] = {(char*)"test42", NULL};

  int status = 0;

  int childPid = qi::os::spawnvp(args);
  EXPECT_EQ(-1, childPid);
  int error = qi::os::waitpid(childPid, &status);

  EXPECT_EQ(0, error) << "error: " << error;
  EXPECT_EQ(127, status) << "status: " << status;
}


TEST(spawnlp, CmdWithNoArgs)
{
  std::string bin = binDir.c_str();

  int status = 0;

  int childPid = qi::os::spawnlp(bin.c_str(), NULL);
  int error = qi::os::waitpid(childPid, &status);

  EXPECT_EQ(0, error) << "error: " << error;
  EXPECT_EQ(42, status) << "status: " << status;
}

TEST(spawnlp, CmdWithArgs)
{
  std::string bin = binDir.c_str();
  std::string arg1 = "23";
  int status = 0;

  int childPid = qi::os::spawnlp(bin.c_str(), arg1.c_str(), NULL);
  int error = qi::os::waitpid(childPid, &status);

  EXPECT_EQ(0, error) << "error: " << error;
  EXPECT_EQ(23, status) << "status: " << status;
}

TEST(spawnlp, CmdWithMultiArgs)
{
  std::string bin = binDir.c_str();
  std::string arg1 = "23";
  std::string arg2 = "-1";

  int status = 0;

  int childPid = qi::os::spawnlp(bin.c_str(), arg2.c_str(), arg1.c_str(), NULL);
  int error = qi::os::waitpid(childPid, &status);

  EXPECT_EQ(0, error) << "error: " << error;
  EXPECT_EQ(22, status) << "status: " << status;
}

TEST(spawnlp, Environment)
{
  // The check_env executable exits with rectode 1 if
  // the FOO environment var is not equal to BAR.
  std::string checkEnv = qi::path::findBin("check_env");
  int status = 0;
  qi::os::setenv("FOO", "BAR");
  int childPid = qi::os::spawnlp(checkEnv.c_str(), NULL);
  int error = qi::os::waitpid(childPid, &status);

  EXPECT_EQ(0, status) << "waitpid failed with retcode:   " << error;
  EXPECT_EQ(0, status) << "check_env failed with rectode: " << status;
}

TEST(spawnlp, InvalidBin)
{
  std::string bin = "test42";
  int status = 0;

  int childPid = qi::os::spawnlp(bin.c_str(), NULL);
  EXPECT_EQ(-1, childPid);
  int error = qi::os::waitpid(childPid, &status);

  EXPECT_EQ(0, error) << "error: " << error;
  EXPECT_EQ(127, status) << "status: " << status;
}

TEST(kill, Terminate)
{
  int status = 0;
  int alive = -1;
  int killed = -1;
  int dead = 0;

  int childPid = qi::os::spawnlp(loopBinDir.c_str(), NULL);
  ASSERT_NE(-1, childPid);

  if (childPid != -1)
  {
    // is it alive?
    alive = qi::os::kill(childPid, 0);

    // let's kill it
    killed = qi::os::kill(childPid, SIGTERM);
    qi::os::waitpid(childPid, &status);

    // is it dead?
    dead = qi::os::kill(childPid, 0);
  }

  EXPECT_EQ(0, alive) << "alive: " << alive;
  EXPECT_EQ(0, killed) << "killed: " << killed;
  EXPECT_EQ(-1, dead) << "dead: " << dead;
}

#ifdef _MSC_VER
# define SIGKILL 9 // not defined on windows and ignored by kill
#endif

TEST(kill, Kill)
{
  int status = 0;
  int alive = -1;
  int killed = -1;
  int dead = 0;

  int childPid = qi::os::spawnlp(loopBinDir.c_str(), NULL);
  ASSERT_NE(-1, childPid);

  if (childPid != -1)
  {
    // is it alive?
    alive = qi::os::kill(childPid, 0);

    // let's kill it
    killed = qi::os::kill(childPid, SIGKILL);
    qi::os::waitpid(childPid, &status);

    // is it dead?
    dead = qi::os::kill(childPid, 0);
  }

  EXPECT_EQ(0, alive) << "alive: " << alive;
  EXPECT_EQ(0, killed) << "killed: " << killed;
  EXPECT_EQ(-1, dead) << "dead: " << dead;
}

TEST(system, CmdWithNoArgs)
{
  std::string bin = binDir;

  int status = qi::os::system(bin.c_str());

  EXPECT_EQ(42, status) << "status: " << status;
}

TEST(system, CmdWithArgs)
{
  std::string bin = binDir + " 23";

  int status = qi::os::system(bin.c_str());

  EXPECT_EQ(23, status) << "status: " << status;
}

TEST(system, CmdWithMultiArgs)
{
  std::string bin = binDir + " 23 -1";

  int status = qi::os::system(bin.c_str());

  EXPECT_EQ(22, status) << "status: " << status;
}

TEST(system, InvalidBin)
{
  std::string bin = "test42";

  int status = qi::os::system(bin.c_str());

 #ifdef _WIN32
  EXPECT_EQ(1, status) << "status: " << status;
 #else
  EXPECT_EQ(127, status) << "status: " << status;
 #endif
}

//============================================================================
// Tests for checking isProcessRunning
//============================================================================
namespace
{

const std::default_random_engine& randEngine()
{
  static const auto randEngine = [] {
    std::random_device rd;
    std::seed_seq seq{ rd(), rd(), rd(), rd(), rd(), rd(), rd(), rd() };
    return std::default_random_engine{ seq };
  }();
  return randEngine;
}

char randomProcessChar()
{
  static const char* characters =
  "0123456789"
  "-_ "
  "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
  "abcdefghijklmnopqrstuvwxyz";
  static int nofChars = strlen(characters) - 1;
  std::uniform_int_distribution<int> dis{ 1, nofChars };
  auto engine = randEngine();
  return characters[dis(engine)];
}

std::string randomProcessName()
{
  std::string str;
  int length = 2 + rand() % 10;
  for(int i = 0; i < length; ++i)
  {
    str.push_back(randomProcessChar());
  }
  return str;
}

int randomWrongPid()
{
  std::uniform_int_distribution<int> dis{ 100000 };
  auto engine = randEngine();
  return dis(engine);
}

} // ends anonymous namespace

TEST(QiOs, isProcessRunningCrazyPid)
{
  ASSERT_FALSE(qi::os::isProcessRunning(randomWrongPid(), randomProcessName()));
}

TEST(QiOs, isProcessRunningCrazyPidNoName)
{
  ASSERT_FALSE(qi::os::isProcessRunning(randomWrongPid()));
}

TEST(QiOs, isProcessRunningWrongName)
{
  const std::string processName = randomProcessName();
  ASSERT_FALSE(qi::os::isProcessRunning(1, processName))
      << "random process with name " << processName << " is said to be running";
}

TEST(QiOs, isProcessRunningRealProcessWithSpaces)
{
  const std::string executable("test launchloop with spaces");
  std::string executablePath = qi::path::findBin(executable);
  const test::ScopedProcess p{executablePath};
  ASSERT_TRUE(qi::os::isProcessRunning(p.pid(), executable))
      << executablePath << " was not found running";
}

TEST(QiOs, isProcessRunningRealProcess)
{
  const std::string executable("testlaunchloop");
  const std::string executablePath = qi::path::findBin(executable);
  const test::ScopedProcess p{executablePath};
  ASSERT_TRUE(qi::os::isProcessRunning(p.pid(), executable))
      << executablePath << " was not found running";
}

TEST(QiOs, isProcessRunningRealProcessNoName)
{
  const std::string executable("testlaunchloop");
  const std::string executablePath = qi::path::findBin(executable);
  const test::ScopedProcess p{executablePath};
  ASSERT_TRUE(qi::os::isProcessRunning(p.pid()));
}

TEST(QiOs, isProcessRunningRealProcessWithArgs)
{
  const std::string executable("testlaunchloop");
  const std::string executablePath = qi::path::findBin(executable);
  const std::vector<std::string> args { "nan", "mais", "allo", "quoi" };
  const test::ScopedProcess p{executablePath, args};
  ASSERT_TRUE(qi::os::isProcessRunning(p.pid(), executable));
}

TEST(QiOs, isProcessRunningRealProcessWithFilePathArg)
{
  const std::string executable("testlaunchloop");
  const std::string executablePath = qi::path::findBin(executable);
  const std::vector<std::string> args {"/nan/mais/allo/quoi" };
  const test::ScopedProcess p{executablePath, args};
  ASSERT_TRUE(qi::os::isProcessRunning(p.pid(), executable));
}

TEST(QiOs, isProcessRunningRealProcessWithArgsUnicode)
{
  const std::string originalExecutable { "testlaunchloop" };
  const std::string originalExecutablePath = qi::path::findBin(originalExecutable);
  // we'll copy the originalExecutable to a unique file inside a unique direcory.
  // we re-use the unique directory name to build a unique file name.
  const qi::Path tmp = qi::Path(qi::os::mktmpdir());
  // japanese ideograms (specified by their unicode code point)
  // as an utf-8-encoded string (does not work on VS).
  //char utf8[] = u8"-\u30e6\u30cb\u30b3\u30fc\u30c9";
  // The same ideograms, specified by their utf-8 encoding
  char utf8[] = "-\xe3\x83\xa6\xe3\x83\x8b\xe3\x82\xb3\xe3\x83\xbc\xe3\x83\x89";
  const std::string executable = tmp.filename() + utf8;
  std::string executableWithExtension = executable;
#if BOOST_OS_WINDOWS && defined(NDEBUG)
    executableWithExtension += ".exe";
#elif BOOST_OS_WINDOWS && !defined(NDEBUG)
    executableWithExtension += "_d.exe";
#endif

  const qi::Path executablePathWithExtension =
          tmp / executableWithExtension;
  const qi::path::ScopedFile executableFile(executablePathWithExtension);
  boost::filesystem::copy(
        qi::Path(originalExecutablePath).bfsPath(),
        executablePathWithExtension.bfsPath());

  const std::vector<std::string> args { "nan", "mais", "allo", "quoi" };
  const test::ScopedProcess p{executablePathWithExtension.str(), args};
  ASSERT_TRUE(qi::os::isProcessRunning(p.pid(), executable));
}
// Tests for isProcessRunning end ============================================
