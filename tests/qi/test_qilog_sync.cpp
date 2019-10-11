/*
 * Copyright (c) 2012, 2013 Aldebaran Robotics. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the COPYING file.
 */

// Disable one to test the disabled macros
#define NO_QI_INFO

#include "test_qilog.hpp"
#include "../../src/log_p.hpp"
#include "qi/testutils/mockutils.hpp"
#include <boost/function.hpp>
#include <boost/utility/string_ref.hpp>
#include <cstring>
#include <future>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <qi/atomic.hpp>
#include <qi/log.hpp>
#include <thread>
#include <ka/conceptpredicate.hpp>
#include <ka/functional.hpp>

using namespace qi;
using ::testing::_;
using ::testing::Exactly;
using ::testing::StrEq;

namespace
{

TEST_F(SyncLog, logsync)
{
  MockLogHandler handler("muffins");

  {
    const auto _u = scopeMockExpectations(handler);
    EXPECT_CALL(handler, log(_, _, _)).Times(Exactly(1000));
    for (int i = 0; i < 1000; i++)
      qiLogFatal("core.log.test1", "%d\n", i);
  }

  // Just a test to check for compilation warning.
  if (true)
    qiLogInfo() << "canard"; // disabled, will not log
}

TEST_F(SyncLog, ifCorrectness)
{
  qiLogCategory("test");
  MockLogHandler handler("cupcakes");

  {
    const auto _u = scopeMockExpectations(handler);
    EXPECT_CALL(handler, log(_, _, _));
    if (true)
      qiLogError("qi.test") << "coin";
  }

  {
    const auto _u = scopeMockExpectations(handler);
    EXPECT_CALL(handler, log(_, _, _));
    if (true)
      qiLogErrorF("qi.test");
  }

  {
    const auto _u = scopeMockExpectations(handler);
    // will not log
    if (false)
      qiLogError("qi.test") << "coin";
  }

  {
    const auto _u = scopeMockExpectations(handler);
    // will not log
    if (false)
      qiLogErrorF("qi.test");
  }

  {
    const auto _u = scopeMockExpectations(handler);
    // disabled so will not log
    if (true)
      qiLogInfo("qi.test") << "coin";
  }

  {
    const auto _u = scopeMockExpectations(handler);
    // disabled so will not log
    if (true)
      qiLogInfoF("qi.test");
  }

  {
    const auto _u = scopeMockExpectations(handler);
    // will not log
    if (false)
      qiLogInfo("qi.test") << "coin";
  }

  {
    const auto _u = scopeMockExpectations(handler);
    // will not log
    if (false)
      qiLogInfoF("qi.test");
  }
}

TEST_F(SyncLog, formatting)
{
  qiLogCategory("qi.test");

  MockLogHandler handler("brownies");

  {
    const auto _u = scopeMockExpectations(handler);
    EXPECT_CALL(handler, log(_, _, StrEq("coin")));
    qiLogError("qi.test") << "coin";
  }

  {
    const auto _u = scopeMockExpectations(handler);
    EXPECT_CALL(handler, log(_, _, StrEq("coin 42")));
    qiLogError("qi.test") << "coin " << 42;
  }

  {
    const auto _u = scopeMockExpectations(handler);
    EXPECT_CALL(handler, log(_, _, StrEq("coin")));
    qiLogErrorF("coin");
  }

  {
    const auto _u = scopeMockExpectations(handler);
    EXPECT_CALL(handler, log(_, _, StrEq("coin 42")));
    qiLogErrorF("coin %s", 42);
  }

  {
    const auto _u = scopeMockExpectations(handler);
    EXPECT_CALL(handler, log(_, _, StrEq("coin 42")));
    qiLogErrorF("coin %s", "42");
  }

  {
    const auto _u = scopeMockExpectations(handler);
    EXPECT_CALL(handler, log(_, _, StrEq("coin 42")));
    qiLogError() << "coin " << 42;
  }

  {
    const auto _u = scopeMockExpectations(handler);
    EXPECT_CALL(handler, log(_, _, StrEq("coin 42")));
    qiLogError("qi.test", "coin 42");
  }

  {
    const auto _u = scopeMockExpectations(handler);
    EXPECT_CALL(handler, log(_, _, StrEq("coin 42")));
    qiLogError("qi.test", "coin %s", "42");
  }

  {
    const auto _u = scopeMockExpectations(handler);
    EXPECT_CALL(handler, log(_, _, StrEq("coin 42 42 42")));
    qiLogError("qi.test", "coin %s %s %s", "42", 42, "42");
  }

  {
    const auto _u = scopeMockExpectations(handler);
    EXPECT_CALL(handler, log(_, _, StrEq("coin 42")));
    qiLogError("qi.test", "coin %s", 42);
  }

  // Test with invalid formats
  {
    const auto _u = scopeMockExpectations(handler);
    EXPECT_CALL(handler, log(_, _, StrEq("coin 42")));
    qiLogErrorF("coin %s", 42, 51);
  }

  {
    const auto _u = scopeMockExpectations(handler);
    EXPECT_CALL(handler, log(_, _, StrEq("coin 42")));
    qiLogErrorF("coin %s%s", 42);
  }
}

TEST_F(SyncLog, filteringChange)
{
  MockLogHandler handler("set");

  log::setLogLevel(LogLevel_Info);

  {
    const auto _u = scopeMockExpectations(handler);
    EXPECT_CALL(handler, log(_, _, _)).Times(Exactly(2));
    qiLogVerbose("init.test2") << "VLoL2"; // will not log, verbosity is info
    qiLogError("init.test2") << "ELoL2";
    qiLogWarning("init.test2") << "WLoL2";
  }

  log::addFilter("init.*", LogLevel_Silent, handler.id);
  log::addFilter("ini*", LogLevel_Verbose, handler.id);
  log::addFilter("init.*", LogLevel_Warning, handler.id);

  {
    const auto _u = scopeMockExpectations(handler);
    EXPECT_CALL(handler, log(_, _, _));
    qiLogVerbose("init.test") << "VLoL"; // will log, filter has priority over global level
  }

  {
    const auto _u = scopeMockExpectations(handler);
    EXPECT_CALL(handler, log(_, _, _));
    qiLogError("init.test") << "ELoL";
  }

  {
    const auto _u = scopeMockExpectations(handler);
    EXPECT_CALL(handler, log(_, _, _));
    qiLogWarning("init.test") << "WLoL";
  }

  {
    const auto _u = scopeMockExpectations(handler);
    EXPECT_CALL(handler, log(_, _, _));
    qiLogVerbose("init.test2") << "VLoL2";
  }

  {
    const auto _u = scopeMockExpectations(handler);
    EXPECT_CALL(handler, log(_, _, _));
    qiLogError("init.test2") << "ELoL2";
  }

  {
    const auto _u = scopeMockExpectations(handler);
    EXPECT_CALL(handler, log(_, _, _));
    qiLogWarning("init.test2") << "WLoL2";
  }
}

TEST_F(SyncLog, filtering)
{
  MockLogHandler handler("set");

  {
    const auto _u = scopeMockExpectations(handler);
    EXPECT_CALL(handler, log(_, _, _));
    qiLogError("qi.test") << "coin";
  }

  {
    const auto _u = scopeMockExpectations(handler);
    EXPECT_CALL(handler, log(_, _, _));
    qiLogError("init.yes") << "coin";
  }

  {
    const auto _u = scopeMockExpectations(handler);
    EXPECT_CALL(handler, log(_, _, _));
    qiLogError("initglob.yes") << "coin";
  }

  log::addFilter("init.yes", LogLevel_Info, handler.id);
  log::addFilter("init.no", LogLevel_Info, handler.id);
  log::addFilter("initglob.*", LogLevel_Info, handler.id);

  // ********************************************************************
  // Test that global level is applied, but is overriden by all addFilter
  // ********************************************************************
  log::setLogLevel(LogLevel_Silent, handler.id);
  {
    const auto _u = scopeMockExpectations(handler);
    qiLogError("qi.test") << "coin";
  }

  {
    const auto _u = scopeMockExpectations(handler);
    EXPECT_CALL(handler, log(_, _, _));
    qiLogError("init.yes") << "coin";
  }

  {
    const auto _u = scopeMockExpectations(handler);
    EXPECT_CALL(handler, log(_, _, _));
    qiLogError("init.no") << "coin";
  }

  {
    const auto _u = scopeMockExpectations(handler);
    EXPECT_CALL(handler, log(_, _, _));
    qiLogError("initglob.yes") << "coin";
  }

  {
    const auto _u = scopeMockExpectations(handler);
    EXPECT_CALL(handler, log(_, _, _));
    qiLogError("initglob.no") << "coin";
  }

  {
    const auto _u = scopeMockExpectations(handler);
    qiLogCategory("qi.test");
    qiLogErrorF("coin"); // will not log
  }

  log::setLogLevel(LogLevel_Error, handler.id);

  {
    const auto _u = scopeMockExpectations(handler);
    EXPECT_CALL(handler, log(_, _, _));
    qiLogError("qi.test") << "coin";
  }

  {
    const auto _u = scopeMockExpectations(handler);
    EXPECT_CALL(handler, log(_, _, _));
    qiLogError("init.yes") << "coin";
  }

  {
    const auto _u = scopeMockExpectations(handler);
    EXPECT_CALL(handler, log(_, _, _));
    qiLogError("init.no") << "coin";
  }

  log::disableCategory("qi.test", handler.id);
  {
    const auto _u = scopeMockExpectations(handler);
    qiLogError("qi.test") << "coin"; // will not log
  }

  log::enableCategory("qi.test", handler.id);
  {
    const auto _u = scopeMockExpectations(handler);
    EXPECT_CALL(handler, log(_, _, _));
    qiLogError("qi.test") << "coin";
  }

  {
    const auto _u = scopeMockExpectations(handler);
    EXPECT_CALL(handler, log(_, _, _));
    qiLogCategory("qi.test");
    qiLogErrorF("coin");
  }

  // ********************************************************************
  // Test that global level is applied, but is overriden by all addFilter
  // ********************************************************************
  log::setLogLevel(LogLevel_Silent, handler.id);
  {
    const auto _u = scopeMockExpectations(handler);
    EXPECT_CALL(handler, log(_, _, _)); // enableCategory overrides setLogLevel
    qiLogError("qi.test") << "coin";
  }

  {
    const auto _u = scopeMockExpectations(handler);
    qiLogError("qi.newCat") << "coin"; // will not log
  }

  {
    const auto _u = scopeMockExpectations(handler);
    EXPECT_CALL(handler, log(_, _, _));
    qiLogError("init.yes") << "coin";
  }

  {
    const auto _u = scopeMockExpectations(handler);
    EXPECT_CALL(handler, log(_, _, _));
    qiLogError("init.no") << "coin";
  }

  {
    const auto _u = scopeMockExpectations(handler);
    EXPECT_CALL(handler, log(_, _, _));
    qiLogError("initglob.yes") << "coin";
  }

  {
    const auto _u = scopeMockExpectations(handler);
    EXPECT_CALL(handler, log(_, _, _));
    qiLogError("initglob.no") << "coin";
  }
}

TEST_F(SyncLog, filteringPerHandler)
{
  MockLogHandler handler1("set1");
  MockLogHandler handler2("set2");

  {
    const auto _u = scopeMockExpectations(handler1);
    const auto _u2 = scopeMockExpectations(handler2);
    EXPECT_CALL(handler1, log(_, _, _));
    EXPECT_CALL(handler2, log(_, _, _)); // both handlers log
    qiLogError("qi.test") << "coin";
  }

  log::setLogLevel(LogLevel_Silent, handler1.id);
  {
    const auto _u = scopeMockExpectations(handler1);
    const auto _u2 = scopeMockExpectations(handler2);
    EXPECT_CALL(handler2, log(_, _, _)); // only handler 2 will log
    qiLogError("qi.test") << "coin";
  }

  log::setLogLevel(LogLevel_Silent, handler2.id);
  {
    const auto _u = scopeMockExpectations(handler1);
    const auto _u2 = scopeMockExpectations(handler2);
    qiLogError("qi.test") << "coin"; // no handler will log
  }

  log::setLogLevel(LogLevel_Debug, handler1.id);
  {
    const auto _u = scopeMockExpectations(handler1);
    const auto _u2 = scopeMockExpectations(handler2);
    EXPECT_CALL(handler1, log(_, _, _)); // only handler 1 will log
    qiLogError("qi.test") << "coin";
  }

  log::addFilter("qi.test", LogLevel_Silent, handler1.id);
  {
    const auto _u = scopeMockExpectations(handler1);
    const auto _u2 = scopeMockExpectations(handler2);
    qiLogError("qi.test") << "coin"; // no handler will log
  }

  log::setLogLevel(LogLevel_Debug, handler2.id);
  {
    const auto _u = scopeMockExpectations(handler1);
    const auto _u2 = scopeMockExpectations(handler2);
    EXPECT_CALL(handler2, log(_, _, _)); // only handler 2 will log
    qiLogError("qi.test") << "coin";
  }

  log::setLogLevel(LogLevel_Debug, handler1.id);
  {
    const auto _u = scopeMockExpectations(handler1);
    const auto _u2 = scopeMockExpectations(handler2);
    EXPECT_CALL(handler2, log(_, _, _)); // only handler 2 will log, addFilter overrides setLogLevel for handler1
    qiLogError("qi.test") << "coin";
  }

  log::addFilter("qi.test", LogLevel_Debug, handler1.id);
  {
    const auto _u = scopeMockExpectations(handler1);
    const auto _u2 = scopeMockExpectations(handler2);
    EXPECT_CALL(handler1, log(_, _, _));
    EXPECT_CALL(handler2, log(_, _, _)); // both handlers log
    qiLogError("qi.test") << "coin";
  }
}

TEST_F(SyncLog, globbing)
{
  MockLogHandler handler("pudding");

  {
    const auto _u = scopeMockExpectations(handler);
    EXPECT_CALL(handler, log(_, _, _));
    qiLogError("qi.test") << "coin";
  }

  log::addFilter("qi.*", LogLevel_Silent, handler.id);
  {
    const auto _u = scopeMockExpectations(handler);
    qiLogError("qi.test") << "coin"; // will not log
  }

  // No priority or stacking between globbing or exact, they just apply
  log::addFilter("qi.test", LogLevel_Verbose, handler.id);
  {
    const auto _u = scopeMockExpectations(handler);
    EXPECT_CALL(handler, log(_, _, _));
    qiLogError("qi.test") << "coin";
  }

  // Check globbing applies to new category
  {
    const auto _u = scopeMockExpectations(handler);
    qiLogError("qi.foo") << "coin"; // will not log
  }

  {
    const auto _u = scopeMockExpectations(handler);
    EXPECT_CALL(handler, log(_, _, _));
    qiLogError("qo.foo") << "coin";
  }
}

TEST_F(SyncLog, emptyLogWithCat)
{
  MockLogHandler handler("cookies");

  EXPECT_CALL(handler, log(_, _, _)).Times(Exactly(3));
  qiLogDebug("log.test1");   // will not log, default log level is info
  qiLogVerbose("log.test1"); // will not log, default log level is info
  qiLogInfo("log.test1");    // will not log, info is disabled
  qiLogWarning("log.test1");
  qiLogError("log.test1");
  qiLogFatal("log.test1");
}

TEST_F(SyncLog, emptyLog)
{
  MockLogHandler handler("cookies again");

  EXPECT_CALL(handler, log(_, _, _)).Times(Exactly(3));
  qiLogCategory("log.test2");
  qiLogDebug();   // will not log, default log level is info
  qiLogVerbose(); // will not log, default log level is info
  qiLogInfo();    // will not log, info is disabled
  qiLogWarning();
  qiLogError();
  qiLogFatal();
}

TEST_F(SyncLog, threadSafeness)
{
  // add category
  {
    std::vector<std::future<void>> futures;
    std::atomic<int> count{0};
    for (int i = 0; i < 10; ++i)
    {
      futures.emplace_back(std::async(std::launch::async, [&count] {
        for (int i = 0; i < 1000; ++i)
        {
          std::ostringstream s;
          s << "cat_" << i << "_" << i;
          log::addCategory(s.str());
        }
        ++count;
      }));
    }

    while (count.load() < 10)
      std::this_thread::sleep_for(std::chrono::milliseconds{50});

    for (auto& fut : futures)
      fut.wait();
  }

  // add handler
  {
    std::vector<std::future<void>> futures;
    std::atomic<int> pos{0};
    std::atomic<int> count{0};
    for (int i = 0; i < 10; ++i)
    {
      futures.emplace_back(std::async(std::launch::async, [&pos, &count] {
        int p = ++pos;
        log::addHandler("foo" + qi::os::to_string(p), &dummyHandler);
        ++count;
      }));
    }

    while (count.load() < 10)
      std::this_thread::sleep_for(std::chrono::milliseconds{50});

    for (auto& fut : futures)
      fut.wait();
  }
}

}
