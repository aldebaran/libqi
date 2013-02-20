/*
 * Copyright (c) 2012 Aldebaran Robotics. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the COPYING file.
 */

// Disable one to test the disabled macros
#define NO_QI_INFO
#include <gtest/gtest.h>
#include <boost/function.hpp>
#include <boost/bind.hpp>

#include <qi/log.hpp>
#include "../src/log_p.hpp"
#include <cstring>

TEST(log, logsync)
{
  qi::log::init(qi::log::info, 0, true);
  atexit(qi::log::destroy);

   for (int i = 0; i < 1000; i++)
     qiLogFatal("core.log.test1", "%d\n", i);
}


TEST(log, logline)
{
  ::qi::log::setContext(0);
  qi::os::timeval tv;
  tv.tv_sec = 0;
  tv.tv_usec = 0;
  std::stringstream snewline;
  snewline << std::endl;
  std::string newline = snewline.str();
  using qi::detail::logline;
  EXPECT_EQ(logline(tv, "", "foo", "", "", 0)     , "foo" + newline);
  EXPECT_EQ(logline(tv, "", "foo\r", "", "", 0)   , "foo" + newline);
  EXPECT_EQ(logline(tv, "", "foo\n", "", "", 0)   , "foo" + newline);
  EXPECT_EQ(logline(tv, "", "foo\r\n", "", "", 0) , "foo" + newline);
  EXPECT_EQ(logline(tv, "", "foo\r\n\n\r\r\n\n\r", "", "", 0) , "foo\r\n\n\r\r\n\n" + newline);
  EXPECT_EQ(logline(tv, "", "foo\r\n\n\r\r\n\n\r\n", "", "", 0) , "foo\r\n\n\r\r\n\n" + newline);
  EXPECT_EQ(logline(tv, "", "foo\r\n\n\r\r\n\n\n", "", "", 0) , "foo\r\n\n\r\r\n\n" + newline);
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

void set (bool& b)
{
  b = true;
}

TEST(log, filtering)
{
  #define YES EXPECT_TRUE(tag); tag = false
  #define NO  EXPECT_TRUE(!tag); tag = false // yes false
  bool tag = false;
  qi::log::addLogHandler("set", boost::bind(&set, boost::ref(tag)));
  qiLogError("qi.test") << "coin";
  YES;
  NO; // ensure reset works
  // global level
  qi::log::setVerbosity(qi::log::silent);
  qiLogError("qi.test") << "coin";
  NO;
  {
    qiLogCategory("qi.test");
    qiLogErrorF("coin");
    NO;
  }
  qi::log::setVerbosity(qi::log::error);
  qiLogError("qi.test") << "coin";
  YES;
  qi::log::disableCategory("qi.test");
  qiLogError("qi.test") << "coin";
  NO;
  qi::log::enableCategory("qi.test");
  qiLogError("qi.test") << "coin";
  YES;
  {
    qiLogCategory("qi.test");
    qiLogErrorF("coin");
    YES;
  }
  // ensure re-enabling category made us debug capable.
  qi::log::setVerbosity(qi::log::debug);
  qiLogDebug("qi.test") << "coin";
  YES;
  // and finish in all-on state
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
  unsigned int id1 = qi::log::addLogHandler("set1", boost::bind(&set, boost::ref(tag1)));
  unsigned int id2 = qi::log::addLogHandler("set2", boost::bind(&set, boost::ref(tag2)));
  NONO;
  qiLogError("qi.test") << "coin";
  YESYES;
  NONO;
  qi::log::setVerbosity(id1, qi::log::silent);
  qiLogError("qi.test") << "coin";
  NOYES;
  qi::log::setVerbosity(id2, qi::log::silent);
  qiLogError("qi.test") << "coin";
  NONO;
  qi::log::setVerbosity(id1, qi::log::debug);
  qiLogError("qi.test") << "coin";
  YESNO;
  qi::log::setCategory(id1, "qi.test", qi::log::silent);
  qiLogError("qi.test") << "coin";
  NONO;
  qi::log::setVerbosity(id2, qi::log::debug);
  qiLogError("qi.test") << "coin";
  NOYES;
  qi::log::setVerbosity(id1, qi::log::silent);
  qi::log::setCategory(id1, "qi.test", qi::log::debug);
  qiLogError("qi.test") << "coin";
  NOYES;
  qi::log::setVerbosity(id1, qi::log::debug);
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
  qi::log::addLogHandler("set", boost::bind(&set, boost::ref(tag)));
  qiLogError("qi.test") << "coin";
  YES;
  NO; // ensure reset works
  qi::log::setCategory("qi.*", qi::log::silent);
  qiLogError("qi.test") << "coin";
  NO;
  // No priority or stacking between globbing or exact, they just apply
  qi::log::setCategory("qi.test", qi::log::verbose);
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
  qi::log::setVerbosity(qi::log::info);
  qi::int64_t t;
  qiLogCategory("qi.test");
  int count = 1000;
  if (getenv("COUNT"))
    count = strtol(getenv("COUNT"), 0, 0);

  t = qi::os::ustime();
  for (int i = 0; i < count; i++)
    qiLogWarning("qi.test") << "foo " << i;
  t = qi::os::ustime() - t;
  std::cerr << "qiLogWarning(cat) << stream " << t << std::endl;

  t = qi::os::ustime();
  for (int i = 0; i < count; i++)
    qiLogWarningF("foo %s", i);
  t = qi::os::ustime() - t;
  std::cerr << "qiLogWarningF(format) " << t << std::endl;

  t = qi::os::ustime();
  for (int i = 0; i < count; i++)
    qiLogWarning() << "foo " <<  i;
  t = qi::os::ustime() - t;
  std::cerr << "qiLogWarning() << stream " << t << std::endl;

  t = qi::os::ustime();
  for (int i = 0; i < count; i++)
    qiLogWarning("qi.test", "foo %s", i) <<"bar " << i;
  t = qi::os::ustime() - t;
  std::cerr << "qiLogWarning(cat, format) << stream " << t << std::endl;


    t = qi::os::ustime();
  for (int i = 0; i < count; i++)
    qiLogDebug("qi.test") << "foo " << i;
  t = qi::os::ustime() - t;
  std::cerr << "qiLogDebug(cat) << stream " << t << std::endl;

  t = qi::os::ustime();
  for (int i = 0; i < count; i++)
    qiLogDebugF("foo %s", i);
  t = qi::os::ustime() - t;
  std::cerr << "qiLogDebugF(format) " << t << std::endl;

  t = qi::os::ustime();
  for (int i = 0; i < count; i++)
    qiLogDebug() << "foo " <<  i;
  t = qi::os::ustime() - t;
  std::cerr << "qiLogDebug() << stream " << t << std::endl;

  t = qi::os::ustime();
  for (int i = 0; i < count; i++)
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
