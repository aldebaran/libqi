#include "test_qilog.hpp"
#include "../../src/log_p.hpp"
#include "qi/testutils/mockutils.hpp"
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <qi/log.hpp>
#include <ka/conceptpredicate.hpp>
#include <ka/functional.hpp>

using namespace qi;
using ::testing::_;
using ::testing::Exactly;
using ::testing::StrEq;

namespace
{
  using qi::LogLevel;

  template<LogLevel L>
  using Constant = std::integral_constant<LogLevel, L>;

  // Test with `InvokeCatchLogRethrow` and its alias for a given log level.
#define QI_DERIVE_LOG_TEST_TYPES(LOG_LEVEL)                                                       \
  std::pair<Constant<LogLevel_ ## LOG_LEVEL>, qi::InvokeCatchLogRethrow<LogLevel_ ## LOG_LEVEL>>, \
  std::pair<Constant<LogLevel_ ## LOG_LEVEL>, qi::InvokeCatchLogRethrow ## LOG_LEVEL>

// Used to avoid finishing the type list with an extraneous comma.
#define QI_TEST_COMMA

  using TestTypes = testing::Types<
#if !defined(NO_QI_SILENT)
    QI_TEST_COMMA QI_DERIVE_LOG_TEST_TYPES(Silent)
# undef QI_TEST_COMMA
# define QI_TEST_COMMA ,
#endif

#ifndef NO_QI_FATAL
    QI_TEST_COMMA QI_DERIVE_LOG_TEST_TYPES(Fatal)
# undef QI_TEST_COMMA
# define QI_TEST_COMMA ,
#endif

#if !defined(NO_QI_ERROR)
    QI_TEST_COMMA QI_DERIVE_LOG_TEST_TYPES(Error)
# undef QI_TEST
# define QI_TEST_COMMA ,
#endif

#ifndef NO_QI_WARNING
    QI_TEST_COMMA QI_DERIVE_LOG_TEST_TYPES(Warning)
# undef QI_TEST
# define QI_TEST_COMMA ,
#endif

#ifndef NO_QI_INFO
    QI_TEST_COMMA QI_DERIVE_LOG_TEST_TYPES(Info)
# undef QI_TEST
# define QI_TEST_COMMA ,
#endif

#ifndef NO_QI_VERBOSE
    QI_TEST_COMMA QI_DERIVE_LOG_TEST_TYPES(Verbose)
# undef QI_TEST
# define QI_TEST_COMMA ,
#endif

#if !defined(NO_QI_DEBUG) && !defined(NDEBUG)
    QI_TEST_COMMA QI_DERIVE_LOG_TEST_TYPES(Debug)
# undef QI_TEST
# define QI_TEST_COMMA ,
#endif
  >;

#undef QI_TEST_COMMA
#undef QI_DERIVE_LOG_TEST_TYPES

  template<typename T>
  struct SyncLogInvokeCatch : SyncLog {};
}

TYPED_TEST_SUITE(SyncLogInvokeCatch, TestTypes);

// Expected: Non-throwing procedure is executed, no log is done.
TYPED_TEST(SyncLogInvokeCatch, NoExceptionThrown)
{
  using InvokeCatchLogRethrow = typename TypeParam::second_type;

  // Prevents log handler to filter any log (hence, `debug` log level).
  MockLogHandler handler("stop cookies");
  qi::log::setLogLevel(qi::LogLevel_Debug, handler.id);
  EXPECT_CALL(handler, log(_, _, _)).Times(Exactly(0)); // Expect no call.

  auto successor = [](int x) noexcept {return x + 1;}; // Non-throwing.
  EXPECT_EQ(1, InvokeCatchLogRethrow{}("cat", "prefix", successor, 0));
}

// Expected: Throwing procedure is executed, log is done, in the right log
// level, with the right content.
TYPED_TEST(SyncLogInvokeCatch, RethrowExceptionThrown)
{
  using ConstantLogLevel = typename TypeParam::first_type;
  using InvokeCatchLogRethrow = typename TypeParam::second_type;

  // Prevents log handler to filter any log (hence, `debug` log level).
  static const auto logLevel = ConstantLogLevel::value;
  MockLogHandler handler("no more cookie");
  qi::log::setLogLevel(qi::LogLevel_Debug, handler.id);

  // Makes log message.
  const auto logCat = "cat";
  const auto logPrefix = "prefix";
  const auto exceptionMsg = "this is not a string";
  const auto logMsg = logPrefix + std::string(": standard exception: ") + exceptionMsg;

  if (logLevel != qi::LogLevel_Silent)
  {
    EXPECT_CALL(handler, log(
      logLevel,
      StrEq(logCat),
      StrEq(logMsg.c_str()))).Times(Exactly(1));
  }

  auto fail = [&](int) -> int {throw std::runtime_error(exceptionMsg);};
  EXPECT_THROW(InvokeCatchLogRethrow{}(logCat, logPrefix, fail, 0), std::runtime_error);
}
