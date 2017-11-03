#include <array>
#include <string>
#include <gtest/gtest.h>
#include <boost/function.hpp>
#include <boost/algorithm/string.hpp>
#include <qi/log.hpp>
#include <qi/functional.hpp>
#include <qi/errorhandling.hpp>
#include <qi/detail/conceptpredicate.hpp>
#include <qi/scoped.hpp>

struct BoostError : boost::exception
{
  std::string msg;
  BoostError(const std::string& s = std::string{}) : msg(s)
  {
  }
  std::ostream& operator<<(std::ostream& o) const
  {
    return o << "BoostError{\"" << msg << "\"}";
  }
};

namespace
{
  const auto stdCode = 55;
  const auto boostCode = 56;
  const auto unknownCode = 57;
  const qi::ExceptionValue<int> exceptionCode{stdCode, boostCode, unknownCode};

  template<typename T>
  int code()
  {
    return unknownCode;
  }

  template<>
  int code<std::runtime_error>()
  {
    return stdCode;
  }

  template<>
  int code<BoostError>()
  {
    return boostCode;
  }

  /// This fixture will ensure that logs in tests of its test case are synchronous which makes
  /// them simpler to implement.
  struct TestErrorHandling : testing::Test
  {
    static void SetUpTestCase()
    {
      qi::log::setSynchronousLog(true);
    }
  };
}

TEST_F(TestErrorHandling, ExceptionValueBasic)
{
  using namespace qi;
  ASSERT_EQ(stdCode, exceptionCode(std::runtime_error{""}));
  ASSERT_EQ(boostCode, exceptionCode(BoostError{""}));
  ASSERT_EQ(unknownCode, exceptionCode());
}

TEST_F(TestErrorHandling, ExceptionValueRegular)
{
  using namespace qi;
  using F = ExceptionValue<int>;
  // This test is partial but representative.
  ASSERT_TRUE(detail::isRegular(
    {F{0, 0, 0}, F{0, 0, 1}, F{0, 1, 0}, F{1, 0, 0}, F{0, 1, 1}, F{1, 1, 1}}));
}

TEST_F(TestErrorHandling, ExceptionLogErrorBasic)
{
  using namespace qi;
  static const std::string prefix{"prefix"}, handlerName{"test"};
  const std::array<const char*, 3> expected{{"standard exception", "boost exception", "unknown exception"}};
  int i = 0;

  log::addHandler(handlerName,
    [&](LogLevel, Clock::time_point, SystemClock::time_point, const char*, std::string msg, const char*, const char*, int) {
      const auto exp = prefix + ": " + expected.at(static_cast<std::size_t>(i));
      if (boost::algorithm::starts_with(msg, exp))
      {
        ++i;
      }
    }
  );
  auto s = scoped([&] {
    log::removeHandler(handlerName);
  });
  ExceptionLogError<std::string> log{"category", prefix};
  log(std::runtime_error{""});
  log(BoostError{""});
  log();
  ASSERT_EQ(3, i);
}

TEST_F(TestErrorHandling, ExceptionLogErrorRegular)
{
  using namespace qi;
  using F = ExceptionLogError<std::string>;
  // This test is partial but representative.
  ASSERT_TRUE(detail::isRegular(
    {F{"a", "a"}, F{"a", "b"}, F{"b", "b"}, F{"ab", "b"}}));
}

TEST_F(TestErrorHandling, ExceptionHandleExceptionRegular)
{
  using namespace qi;
  HandleExceptionAndRethrow<PolymorphicConstantFunction<void>> h;
  ASSERT_TRUE(detail::isRegular({h}));
}

template<typename T>
struct TestErrorHandlingExceptionParam : TestErrorHandling {};

using exceptions = testing::Types<std::runtime_error, BoostError, std::string>;

TYPED_TEST_CASE(TestErrorHandlingExceptionParam, exceptions);

TYPED_TEST(TestErrorHandlingExceptionParam, ExceptionHandleExceptionRethrow)
{
  using Exception = TypeParam;
  using namespace qi;
  HandleExceptionAndRethrow<PolymorphicConstantFunction<void>> rethrow;
  auto f = [=] {
    try
    {
      throw Exception{""};
    }
    catch (const Exception& e)
    {
      rethrow(e);
    }
  };
  EXPECT_THROW(f(), Exception);
}

struct Append
{
  std::string& log;
// PolymorphicFunction<T (std::exception), T (boost::exception), T ()>:
  void operator()(std::exception const& e) const
  {
    log += e.what();
  }
  void operator()(boost::exception const& e) const
  {
    log += boost::diagnostic_information(e);
  }
  void operator()() const
  {
    log += "unknown";
  }
};

TEST_F(TestErrorHandling, HandleExceptionRethrowLogsStdException)
{
  using namespace qi;
  std::string log;
  HandleExceptionAndRethrow<Append> rethrow{Append{log}};
  const std::string msg = "abcdefghijkl";
  auto f = [=] {
    try
    {
      throw std::runtime_error{msg};
    }
    catch (const std::exception& e)
    {
      rethrow(e);
    }
  };
  ASSERT_THROW(f(), std::runtime_error);
  ASSERT_EQ(msg, log);
}

TEST_F(TestErrorHandling, NoException)
{
  using namespace qi;
  const auto twice = [](int i) {
    return 2 * i;
  };
  const auto n = invokeCatch(exceptionCode, twice, 3);
  ASSERT_EQ(6, n);
}

TYPED_TEST(TestErrorHandlingExceptionParam, InvokeCatchCodeValue)
{
  using Exception = TypeParam;
  using namespace qi;
  const auto twice = [](int) -> int {
    throw Exception{""};
  };
  const auto n = invokeCatch(exceptionCode, twice, 3);
  ASSERT_EQ(code<Exception>(), n);
}

TYPED_TEST(TestErrorHandlingExceptionParam, InvokeCatchHandleExceptionAndRethrow)
{
  using Exception = TypeParam;
  using namespace qi;
  const auto f = [](int) {
    throw Exception{""};
  };
  HandleExceptionAndRethrow<PolymorphicConstantFunction<void>> rethrow;
  EXPECT_THROW(invokeCatch(rethrow, f, 3), Exception);
}

TEST_F(TestErrorHandling, InvokeCatchHandleExceptionAndRethrowLogStdException)
{
  using namespace qi;
  std::string log;
  const std::string msg = "abcdefghijkl";
  const auto f = [&](int) {
    throw std::runtime_error{msg};
  };
  HandleExceptionAndRethrow<Append> rethrow{Append{log}};
  ASSERT_THROW(invokeCatch(rethrow, f, 3), std::runtime_error);
  ASSERT_EQ(msg, log);
}
