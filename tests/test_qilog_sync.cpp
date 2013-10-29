/*
 * Copyright (c) 2012, 2013 Aldebaran Robotics. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the COPYING file.
 */

// Disable one to test the disabled macros
#define NO_QI_INFO

#include <cstring>

#include <gtest/gtest.h>
#include <boost/function.hpp>
#include <boost/bind.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/thread.hpp>

#include <qi/log.hpp>
#include <qi/atomic.hpp>

#include "../src/log_p.hpp"


TEST(log, logsync)
{
  qi::log::init(qi::LogLevel_Info, 0, true);
  atexit(qi::log::destroy);

   for (int i = 0; i < 1000; i++)
     qiLogFatal("core.log.test1", "%d\n", i);
   // Just a test to check for compilation warning.
   if (true)
     qiLogInfo() << "canard";
}

TEST(log, ifCorrectness)
{
  qiLogCategory("test");
  bool ok = true;
  if (true)
    qiLogError("qi.test") << "coin";
  else
    ok = false;
  EXPECT_TRUE(ok);
  if (true)
    qiLogErrorF("qi.test");
  else
    ok = false;
  EXPECT_TRUE(ok);
  ok = false;
  if (false)
    qiLogError("qi.test") << "coin";
  else
    ok = true;
  EXPECT_TRUE(ok);
  ok = false;
  if (false)
    qiLogErrorF("qi.test");
  else
    ok = true;
  EXPECT_TRUE(ok);

  ok = true;
  if (true)
    qiLogInfo("qi.test") << "coin";
  else
    ok = false;
  EXPECT_TRUE(ok);
  if (true)
    qiLogInfoF("qi.test");
  else
    ok = false;
  EXPECT_TRUE(ok);
  ok = false;
  if (false)
    qiLogInfo("qi.test") << "coin";
  else
    ok = true;
  EXPECT_TRUE(ok);
  ok = false;
  if (false)
    qiLogInfoF("qi.test");
  else
    ok = true;
  EXPECT_TRUE(ok);
}

void copy(std::string& dest, const char* src)
{
  dest = src;
}

TEST(log, formatting)
{
  qiLogCategory("qi.test");
  std::string lastMessage;
  qi::log::removeLogHandler("consoleloghandler");
  qi::log::addLogHandler("copy", boost::bind(&copy, boost::ref(lastMessage), _4));
  qiLogError("qi.test") << "coin";
  EXPECT_EQ("coin", lastMessage);
  qiLogError("qi.test") << "coin " << 42;
  EXPECT_EQ("coin 42", lastMessage);
  qiLogErrorF("coin");
  EXPECT_EQ("coin", lastMessage);
  qiLogErrorF("coin %s", 42);
  EXPECT_EQ("coin 42", lastMessage);
  qiLogErrorF("coin %s", "42");
  EXPECT_EQ("coin 42", lastMessage);
  qiLogError() << "coin " << 42;
  EXPECT_EQ("coin 42", lastMessage);
  qiLogError("qi.test", "coin 42");
  EXPECT_EQ("coin 42", lastMessage);
  qiLogError("qi.test", "coin %s", "42");
  EXPECT_EQ("coin 42", lastMessage);
  qiLogError("qi.test", "coin %s %s %s", "42", 42, "42");
  EXPECT_EQ("coin 42 42 42", lastMessage);
  qiLogError("qi.test", "coin %s", 42);
  EXPECT_EQ("coin 42", lastMessage);

  // Test with invalid formats
  qiLogErrorF("coin %s", 42, 51);
  EXPECT_EQ("coin 42", lastMessage);
  qiLogErrorF("coin %s%s", 42);
  EXPECT_EQ("coin 42", lastMessage);
  qi::log::removeLogHandler("copy");
}

void set (const char* cat, bool& b)
{
  //remove log from the logger itself
  if (std::string(cat) == "qi.log")
    return;
  b = true;
}

TEST(log, filteringChange)
{
  #define YES EXPECT_TRUE(tag); tag = false
  #define NO  EXPECT_TRUE(!tag); tag = false // yes false
  bool tag = false;
  qi::log::SubscriberId id = qi::log::addLogHandler("set", boost::bind(&set, _3, boost::ref(tag)));
  qiLogDebug("init.test2") << "DLoL2";
  qiLogInfo("init.test2") << "ILoL2";
  qiLogVerbose("init.test2") << "VLoL2";

  qi::log::setCategory("init.*", qi::LogLevel_Silent, id);
  qi::log::setCategory("ini*", qi::LogLevel_Debug, id);
  qi::log::setCategory("init.*", qi::LogLevel_Verbose, id);

  tag = false;
  qiLogDebug("init.test") << "DLoL";
  YES;
  qiLogWarning("init.test") << "WLoL";
  YES;
  qiLogVerbose("init.test") << "VLoL";
  YES;

  qiLogDebug("init.test2") << "DLoL2";
  YES;
  qiLogWarning("init.test2") << "WLoL2";
  YES;
  qiLogVerbose("init.test2") << "VLoL2";
  YES;
}

TEST(log, filtering)
{
  #define YES EXPECT_TRUE(tag); tag = false
  #define NO  EXPECT_TRUE(!tag); tag = false // yes false
  bool tag = false;
  qi::log::SubscriberId id = qi::log::addLogHandler("set", boost::bind(&set, _3, boost::ref(tag)));
  qiLogError("qi.test") << "coin";
  YES;
  NO; // ensure reset works
  // global level
  qiLogError("init.yes") << "coin";
  YES;
  qiLogError("initglob.yes") << "coin";
  YES;
  qi::log::setCategory("init.yes", qi::LogLevel_Info, id);
  qi::log::setCategory("init.no", qi::LogLevel_Info, id);
  qi::log::setCategory("initglob.*", qi::LogLevel_Info, id);
  qi::log::setVerbosity(qi::LogLevel_Silent, id);
  // Test that global level is applied, but is overriden by all setCategory
  qiLogError("qi.test") << "coin";
  NO;
  qiLogError("init.yes") << "coin";
  YES;
  qiLogError("init.no") << "coin";
  YES;
  qiLogError("initglob.yes") << "coin";
  YES;
  qiLogError("initglob.no") << "coin";
  YES;
  {
    qiLogCategory("qi.test");
    qiLogErrorF("coin");
    NO;
  }
  qi::log::setVerbosity(qi::LogLevel_Error, id);
  qiLogError("qi.test") << "coin";
  YES;
  qiLogError("init.yes") << "coin";
  YES;
  qiLogError("init.no") << "coin";
  YES;
  qi::log::disableCategory("qi.test", id);
  qiLogError("qi.test") << "coin";
  NO;
  qi::log::enableCategory("qi.test", id);
  qiLogError("qi.test") << "coin";
  YES;
  {
    qiLogCategory("qi.test");
    qiLogErrorF("coin");
    YES;
  }
  qi::log::setVerbosity(qi::LogLevel_Silent, id);
  // Test that global level is applied, but is overriden by all setCategory
  qiLogError("qi.test") << "coin";
  YES; //enableCategory overrides setVerbosity
  qiLogError("qi.newCat") << "coin";
  NO;
  qiLogError("init.yes") << "coin";
  YES;
  qiLogError("init.no") << "coin";
  YES;
  qiLogError("initglob.yes") << "coin";
  YES;
  qiLogError("initglob.no") << "coin";
  YES;
  qi::log::removeLogHandler("set");
}

TEST(log, filteringPerHandler)
{
  #define YES1 EXPECT_TRUE(tag1); tag1 = false
  #define NO1  EXPECT_TRUE(!tag1); tag1 = false // yes false
  #define YES2 EXPECT_TRUE(tag2); tag2 = false
  #define NO2  EXPECT_TRUE(!tag2); tag2 = false // yes false
  #define YESYES YES1; YES2
  #define YESNO YES1; NO2
  #define NOYES NO1; YES2
  #define NONO NO1; NO2
  bool tag1 = false;
  bool tag2 = false;
  unsigned int id1 = qi::log::addLogHandler("set1", boost::bind(&set, _3, boost::ref(tag1)));
  unsigned int id2 = qi::log::addLogHandler("set2", boost::bind(&set, _3, boost::ref(tag2)));
  tag1 = tag2 = false;

  NONO;
  qiLogError("qi.test") << "coin";
  YESYES;
  NONO;
  qi::log::setVerbosity(qi::LogLevel_Silent, id1);
  qiLogError("qi.test") << "coin";
  NOYES;
  qi::log::setVerbosity(qi::LogLevel_Silent, id2);
  qiLogError("qi.test") << "coin";
  NONO;
  qi::log::setVerbosity(qi::LogLevel_Debug, id1);
  qiLogError("qi.test") << "coin";
  YESNO;
  qi::log::setCategory("qi.test", qi::LogLevel_Silent, id1);
  qiLogError("qi.test") << "coin";
  NONO;
  qi::log::setVerbosity(qi::LogLevel_Debug, id2);
  qiLogError("qi.test") << "coin";
  NOYES;
  qi::log::setVerbosity(qi::LogLevel_Debug, id1);
  qiLogError("qi.test") << "coin";
  NOYES; // setCategory overrides setVerbosity for id1
  qi::log::setCategory("qi.test", qi::LogLevel_Debug, id1);
  qiLogError("qi.test") << "coin";
  YESYES;
  qi::log::removeLogHandler("set1");
  qi::log::removeLogHandler("set2");
}

TEST(log, globbing)
{
  #define YES EXPECT_TRUE(tag); tag = false
  #define NO  EXPECT_TRUE(!tag); tag = false // yes false
  bool tag = false;
  qi::log::SubscriberId id = qi::log::addLogHandler("set", boost::bind(&set, _3, boost::ref(tag)));
  qiLogError("qi.test") << "coin";
  YES;
  NO; // ensure reset works
  qi::log::setCategory("qi.*", qi::LogLevel_Silent, id);
  qiLogError("qi.test") << "coin";
  NO;
  // No priority or stacking between globbing or exact, they just apply
  qi::log::setCategory("qi.test", qi::LogLevel_Verbose, id);
  qiLogError("qi.test") << "coin";
  YES;
  // Check globbing applies to new category
  qiLogError("qi.foo") << "coin";
  NO;
  qiLogError("qo.foo") << "coin";
  YES;
  qi::log::removeLogHandler("set");
}

void nothing()
{
}

TEST(log, perf)
{
  // Compare perf of (disabled, enabled) x (QI_LOG, qiLog)
  qi::log::addLogHandler("nothing", boost::bind(&nothing));
  qi::log::removeLogHandler("consoleloghandler");
  qi::log::setVerbosity(qi::LogLevel_Info);
  qi::int64_t t;
  qiLogCategory("qi.test");
  long count = 1000;
  if (getenv("COUNT"))
    count = strtol(getenv("COUNT"), 0, 0);

  t = qi::os::ustime();
  for (long int i = 0; i < count; i++)
    qiLogWarning("qi.test") << "foo " << i;
  t = qi::os::ustime() - t;
  std::cerr << "qiLogWarning(cat) << stream " << t << std::endl;

  t = qi::os::ustime();
  for (long int i = 0; i < count; i++)
    qiLogWarningF("foo %s", i);
  t = qi::os::ustime() - t;
  std::cerr << "qiLogWarningF(format) " << t << std::endl;

  t = qi::os::ustime();
  for (long int i = 0; i < count; i++)
    qiLogWarning() << "foo " <<  i;
  t = qi::os::ustime() - t;
  std::cerr << "qiLogWarning() << stream " << t << std::endl;

  t = qi::os::ustime();
  for (long int i = 0; i < count; i++)
    qiLogWarning("qi.test", "foo %s", i) <<"bar " << i;
  t = qi::os::ustime() - t;
  std::cerr << "qiLogWarning(cat, format) << stream " << t << std::endl;


    t = qi::os::ustime();
  for (long int i = 0; i < count; i++)
    qiLogDebug("qi.test") << "foo " << i;
  t = qi::os::ustime() - t;
  std::cerr << "qiLogDebug(cat) << stream " << t << std::endl;

  t = qi::os::ustime();
  for (long int i = 0; i < count; i++)
    qiLogDebugF("foo %s", i);
  t = qi::os::ustime() - t;
  std::cerr << "qiLogDebugF(format) " << t << std::endl;

  t = qi::os::ustime();
  for (long int i = 0; i < count; i++)
    qiLogDebug() << "foo " <<  i;
  t = qi::os::ustime() - t;
  std::cerr << "qiLogDebug() << stream " << t << std::endl;

  t = qi::os::ustime();
  for (long int i = 0; i < count; i++)
    qiLogDebug("qi.test", "foo %s", i) <<"bar " << i;
  t = qi::os::ustime() - t;
  std::cerr << "qiLogDebug(cat, format) << stream " << t << std::endl;

}

// Useful for manual testing
TEST(log, jukebox_DISABLED)
{
  const char* cats[] = {"a", "a.a", "a.b", "b.a", "b.b", 0};
  for (unsigned i=0; cats[i]; ++i)
  {
    const char* c = cats[i];
    qiLogError(c) << c;
    qiLogVerbose(c) << c;
    qiLogDebug(c) << c;
  }
}

TEST(log, emptyLogWithCat)
{
  qi::log::init(qi::LogLevel_Debug, 0, true);

  qiLogDebug("log.test1");
  qiLogVerbose("log.test1");
  qiLogInfo("log.test1");
  qiLogWarning("log.test1");
  qiLogError("log.test1");
  qiLogFatal("log.test1");
}

TEST(log, emptyLog)
{
  qiLogCategory("log.test2");
  qiLogDebug();
  qiLogVerbose();
  qiLogInfo();
  qiLogWarning();
  qiLogError();
  qiLogFatal();
}

void nullHandler(const qi::LogLevel,
                             const qi::os::timeval,
                             const char*,
                             const char*,
                             const char*,
                             const char*,
                             int) {}
void makeCats(int uid, unsigned count, qi::Atomic<int>& finished)
{
  for (unsigned i=0; i<count; ++i)
  {
    std::stringstream s;
    s << "cat_" << uid << "_" << i;
    qi::log::addCategory(s.str());
  }
  ++finished;
}

void addHandler(qi::Atomic<int>& pos, qi::Atomic<int>& count)
{
  int p = ++pos;
  qi::log::SubscriberId id = qi::log::addLogHandler("foo" + boost::lexical_cast<std::string>(p),
    nullHandler);
  ++count;
}

TEST(log, threadSafeness)
{
  qi::Atomic<int> count;
  for (unsigned i=0; i<10; ++i)
    boost::thread(makeCats, i, 1000, boost::ref(count));
  while (*count < 10)
    qi::os::msleep(50);
  qi::Atomic<int> pos;
  count = 0;
  for (unsigned i=0; i<10; ++i)
    boost::thread(addHandler, boost::ref(pos), boost::ref(count));
  while (*count < 10)
    qi::os::msleep(50);
  EXPECT_TRUE(true);
}
