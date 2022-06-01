#include <cstring>
#include <array>
#include <stdexcept>
#include <string>
#include <tuple>
#include <boost/algorithm/string.hpp>
#include <gtest/gtest.h>
#include <ka/conceptpredicate.hpp>
#include <ka/errorhandling.hpp>
#include <ka/scoped.hpp>
#include <ka/unit.hpp>
#include <qi/clock.hpp>
#include "test_qilog.hpp"
#include "../../src/log_p.hpp"

struct boost_error_t : boost::exception
{
  std::string msg;
  boost_error_t(std::string s = {}) : msg(std::move(s))
  {
  }
  std::ostream& operator<<(std::ostream& o) const
  {
    return o << "BoostError{\"" << msg << "\"}";
  }
};

namespace
{
  using qi::LogLevel;

  template<LogLevel L>
  using Constant = std::integral_constant<LogLevel, L>;

  // For a given log level, `ExceptionLog` type and its corresponding alias.
#define QI_DERIVE_EXCEPTIONLOG_TYPES(LOG_LEVEL)              \
  qi::ExceptionLog<qi::LogLevel_ ## LOG_LEVEL, std::string>, \
  qi::ExceptionLog ## LOG_LEVEL<std::string>

// Used to avoid finishing the type list with an extraneous comma.
#define QI_TEST_COMMA

  using ExceptionLogTypes = testing::Types<
#ifndef NO_QI_SILENT
    QI_TEST_COMMA QI_DERIVE_EXCEPTIONLOG_TYPES(Silent)
# undef QI_TEST_COMMA
# define QI_TEST_COMMA ,
#endif

#ifndef NO_QI_FATAL
    QI_TEST_COMMA QI_DERIVE_EXCEPTIONLOG_TYPES(Fatal)
# undef QI_TEST_COMMA
# define QI_TEST_COMMA ,
#endif

#ifndef NO_QI_ERROR
    QI_TEST_COMMA QI_DERIVE_EXCEPTIONLOG_TYPES(Error)
# undef QI_TEST_COMMA
# define QI_TEST_COMMA ,
#endif

#ifndef NO_QI_WARNING
    QI_TEST_COMMA QI_DERIVE_EXCEPTIONLOG_TYPES(Warning)
# undef QI_TEST_COMMA
# define QI_TEST_COMMA ,
#endif

#ifndef NO_QI_INFO
    QI_TEST_COMMA QI_DERIVE_EXCEPTIONLOG_TYPES(Info)
# undef QI_TEST_COMMA
# define QI_TEST_COMMA ,
#endif

#ifndef NO_QI_VERBOSE
    QI_TEST_COMMA QI_DERIVE_EXCEPTIONLOG_TYPES(Verbose)
# undef QI_TEST_COMMA
# define QI_TEST_COMMA ,
#endif

#if !defined(NO_QI_DEBUG) && !defined(NDEBUG)
    QI_TEST_COMMA QI_DERIVE_EXCEPTIONLOG_TYPES(Debug)
# undef QI_TEST_COMMA
# define QI_TEST_COMMA ,
#endif
  >;

#undef QI_TEST_COMMA
#undef QI_DERIVE_EXCEPTIONLOG_TYPES

  template<typename T>
  struct TestExceptionLogRegular : ::testing::Test
  {
  };
}

TYPED_TEST_SUITE(TestExceptionLogRegular, ExceptionLogTypes);

TYPED_TEST(TestExceptionLogRegular, Basic)
{
  using F = TypeParam;
  // This test is partial but representative.
  ASSERT_TRUE(ka::is_regular(
    {F{"a", "a"}, F{"a", "b"}, F{"b", "b"}, F{"ab", "b"}}));
}

namespace
{
  // For a given log level, sets all combinations of `ExceptionLog` types
  // (canonical and alias) and exception types (standard, boost, unknown).
  // Note: Unknown exception type is represented by `ka::unit_t`.
#define QI_DERIVE_LOG_TEST_TYPES(LOG_LEVEL)                                                       \
  std::tuple<Constant<qi::LogLevel_ ## LOG_LEVEL>, qi::ExceptionLog<qi::LogLevel_ ## LOG_LEVEL, std::string>, std::runtime_error>, \
  std::tuple<Constant<qi::LogLevel_ ## LOG_LEVEL>, qi::ExceptionLog<qi::LogLevel_ ## LOG_LEVEL, std::string>, boost_error_t>, \
  std::tuple<Constant<qi::LogLevel_ ## LOG_LEVEL>, qi::ExceptionLog<qi::LogLevel_ ## LOG_LEVEL, std::string>, ka::unit_t>, \
  std::tuple<Constant<qi::LogLevel_ ## LOG_LEVEL>, qi::ExceptionLog ## LOG_LEVEL<std::string>, std::runtime_error>, \
  std::tuple<Constant<qi::LogLevel_ ## LOG_LEVEL>, qi::ExceptionLog ## LOG_LEVEL<std::string>, boost_error_t>, \
  std::tuple<Constant<qi::LogLevel_ ## LOG_LEVEL>, qi::ExceptionLog ## LOG_LEVEL<std::string>, ka::unit_t>

// Used to avoid finishing the type list with an extraneous comma.
#define QI_TEST_COMMA

  using LogCombinationTypes = testing::Types<
#ifndef NO_QI_SILENT
    QI_TEST_COMMA QI_DERIVE_LOG_TEST_TYPES(Silent)
# undef QI_TEST_COMMA
# define QI_TEST_COMMA ,
#endif

#ifndef NO_QI_FATAL
    QI_TEST_COMMA QI_DERIVE_LOG_TEST_TYPES(Fatal)
# undef QI_TEST_COMMA
# define QI_TEST_COMMA ,
#endif

#ifndef NO_QI_ERROR
    QI_TEST_COMMA QI_DERIVE_LOG_TEST_TYPES(Error)
# undef QI_TEST_COMMA
# define QI_TEST_COMMA ,
#endif

#ifndef NO_QI_WARNING
    QI_TEST_COMMA QI_DERIVE_LOG_TEST_TYPES(Warning)
# undef QI_TEST_COMMA
# define QI_TEST_COMMA ,
#endif

#ifndef NO_QI_INFO
    QI_TEST_COMMA QI_DERIVE_LOG_TEST_TYPES(Info)
# undef QI_TEST_COMMA
# define QI_TEST_COMMA ,
#endif

#ifndef NO_QI_VERBOSE
    QI_TEST_COMMA QI_DERIVE_LOG_TEST_TYPES(Verbose)
# undef QI_TEST_COMMA
# define QI_TEST_COMMA ,
#endif

#if !defined(NO_QI_DEBUG) && !defined(NDEBUG)
    QI_TEST_COMMA QI_DERIVE_LOG_TEST_TYPES(Debug)
# undef QI_TEST_COMMA
# define QI_TEST_COMMA ,
#endif
  >;

#undef QI_TEST_COMMA
#undef QI_DERIVE_LOG_TEST_TYPES

  template<typename T>
  struct TestExceptionLog : ::testing::Test
  {
  };

  // Calls an exception-handling procedure with the right exception.
  template<typename Proc>
  void call(Proc proc, ka::type_t<std::runtime_error>, std::string msg)
  {
    proc(std::runtime_error{std::move(msg)});
  }

  template<typename Proc>
  void call(Proc proc, ka::type_t<boost_error_t>, std::string msg)
  {
    proc(boost_error_t{std::move(msg)});
  }

  template<typename Proc>
  void call(Proc proc, ka::type_t<ka::unit_t>, const std::string&)
  {
    // This is the 'unknown exception' case, so there is no exception to provide
    // to the procedure.
    proc();
  }

  using StrPair = std::pair<std::string, std::string>;

  // Exception kind and message that are expected in logs.
  StrPair str(ka::type_t<std::runtime_error>, std::string msg)
  {
    return {"standard exception", std::move(msg)};
  }

  StrPair str(ka::type_t<boost_error_t>, const std::string&)
  {
    // No expected message, because `boost` won't provide it in this test
    // context ("no known throw location"-like reason).
    return {"boost exception", ""};
  }

  StrPair str(ka::type_t<ka::unit_t>, const std::string&)
  {
    return {"unknown exception", ""};
  }

  // Begin and end variants that handle standard containers and C-strings.
  template<typename C>
  auto myBegin(const C& c) -> decltype(std::begin(c))
  {
    return std::begin(c);
  }

  template<typename C>
  auto myEnd(const C& c) -> decltype(std::end(c))
  {
    return std::end(c);
  }

  auto myBegin(const char* c) -> const char*
  {
    return c;
  }

  auto myEnd(const char* c) -> const char*
  {
    return c + std::strlen(c);
  }

  // True iff `c0` contains `c1`.
  template<typename C0, typename C1>
  bool contains(const C0& c0, const C1& c1)
  {
    return std::search(myBegin(c0), myEnd(c0), myBegin(c1), myEnd(c1)) != myEnd(c0);
  }

  // GTest predicate that matches a log.
  // Checks that the log contains the expected components.
  template<typename Exception>
  struct LogApproxMatcher
  {
    std::string _logPrefix, _exceptionMsg;

    template<typename S>
    bool MatchAndExplain(const S& s,
                         ::testing::MatchResultListener* /* listener */) const
    {
      std::string except, msg;
      std::tie(except, msg) = str(ka::type_t<Exception>{}, std::move(_exceptionMsg));
      return contains(s, _logPrefix)
          && contains(s, except)
          && contains(s, msg);
    }
    void DescribeTo(::std::ostream* os) const
    {
      *os << "has right format.";
    }
    void DescribeNegationTo(::std::ostream* os) const
    {
      *os << "has not right format.";
    }
  };
} // namespace

TYPED_TEST_SUITE(TestExceptionLog, LogCombinationTypes);

// Expected: Log is done, in the right log level, with (approximately) the right
// message.
TYPED_TEST(TestExceptionLog, Basic)
{
  // Extract type parameters.
  using ConstantLogLevel = typename std::tuple_element<0, TypeParam>::type;
  using ExceptionLog = typename std::tuple_element<1, TypeParam>::type;
  using Exception = typename std::tuple_element<2, TypeParam>::type;

  // Setup log handler to avoid any unwanted filtering (hence, set `debug` log level).
  MockLogHandler handler("no more cookie");
  qi::log::setLogLevel(qi::LogLevel_Debug, handler.id);

  // Set expected log message components.
  const auto logCat = "cat";
  const auto logPrefix = "prefix";
  const auto exceptionMsg = "this is not a string";

  // Expect a log only if not 'silent'.
  using ::testing::Exactly;
  using ::testing::StrEq;
  auto LogApproxEq = [](std::string prefix, std::string msg) {
    return ::testing::MakePolymorphicMatcher(
      LogApproxMatcher<Exception>{std::move(prefix), std::move(msg)}
    );
  };
  static const auto logLevel = ConstantLogLevel::value;
  if (logLevel != qi::LogLevel_Silent)
  {
    EXPECT_CALL(handler, log(
      logLevel,
      StrEq(logCat),
      LogApproxEq(logPrefix, exceptionMsg))).Times(Exactly(1));
  }

  // Call the procedure that logs the exception.
  call(ExceptionLog{logCat, logPrefix}, ka::type_t<Exception>{}, exceptionMsg);
}
