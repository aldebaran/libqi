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
}

TEST(ExceptionValue, Basic)
{
  using namespace qi;
  ASSERT_EQ(stdCode, exceptionCode(std::runtime_error{""}));
  ASSERT_EQ(boostCode, exceptionCode(BoostError{""}));
  ASSERT_EQ(unknownCode, exceptionCode());
}

TEST(ExceptionValue, Regular)
{
  using namespace qi;
  using F = ExceptionValue<int>;
  // This test is partial but representative.
  ASSERT_TRUE(detail::isRegular(
    {F{0, 0, 0}, F{0, 0, 1}, F{0, 1, 0}, F{1, 0, 0}, F{0, 1, 1}, F{1, 1, 1}}));
}

TEST(ExceptionLogError, Basic)
{
  using namespace qi;
  using namespace boost::algorithm;
  static const std::string prefix{"prefix"}, handlerName{"test"};
  const std::array<const char*, 3> expected{"standard exception", "boost exception", "unknown exception"};
  int i = 0;
  log::addHandler(handlerName,
    [&](LogLevel, Clock::time_point, SystemClock::time_point, const char*, std::string msg, const char*, const char*, int) {
      const auto exp = prefix + ": " + expected.at(i);
      if (!starts_with(msg, exp))
        throw std::runtime_error("'" + msg + "' does not start with '" + exp + "'");
      ++i;
    }
  );
  auto s = scoped([&] {
    log::removeHandler(handlerName);
  });
  ExceptionLogError<std::string> log{"category", prefix};
  log(std::runtime_error{""});
  log(BoostError{""});
  log();
}

TEST(ExceptionLogError, Regular)
{
  using namespace qi;
  using F = ExceptionLogError<std::string>;
  // This test is partial but representative.
  ASSERT_TRUE(detail::isRegular(
    {F{"a", "a"}, F{"a", "b"}, F{"b", "b"}, F{"ab", "b"}}));
}

TEST(ExceptionHandleException, Regular)
{
  using namespace qi;
  HandleExceptionAndRethrow<PolymorphicConstantFunction<void>> h;
  ASSERT_TRUE(detail::isRegular({h}));
}

template<typename T>
struct ExceptionHandleException : testing::Test {};

using exceptions = testing::Types<std::runtime_error, BoostError, std::string>;

TYPED_TEST_CASE(ExceptionHandleException, exceptions);

TYPED_TEST(ExceptionHandleException, Rethrow)
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

TEST(ExceptionHandleException, LogStdException)
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

TEST(ExceptionInvokeCatch, NoException)
{
  using namespace qi;
  const auto twice = [](int i) {
    return 2 * i;
  };
  const auto n = invokeCatch(exceptionCode, twice, 3);
  ASSERT_EQ(6, n);
}

template<typename T>
struct ExceptionInvokeCatchThrow : testing::Test {};

TYPED_TEST_CASE(ExceptionInvokeCatchThrow, exceptions);

TYPED_TEST(ExceptionInvokeCatchThrow, ExceptionValue)
{
  using Exception = TypeParam;
  using namespace qi;
  const auto twice = [](int) -> int {
    throw Exception{""};
  };
  const auto n = invokeCatch(exceptionCode, twice, 3);
  ASSERT_EQ(code<Exception>(), n);
}

TEST(ExceptionInvokeCatch, HandleExceptionAndRethrow_StdException)
{
  using namespace qi;
  const auto f = [](int) {
    throw std::runtime_error{""};
  };
  HandleExceptionAndRethrow<PolymorphicConstantFunction<void>> rethrow;
  EXPECT_THROW(invokeCatch(rethrow, f, 3), std::runtime_error);
}

TEST(ExceptionInvokeCatch, HandleExceptionAndRethrow_BoostException)
{
  using namespace qi;
  const auto f = [](int) {
    throw BoostError{};
  };
  HandleExceptionAndRethrow<PolymorphicConstantFunction<void>> rethrow;
  EXPECT_THROW(invokeCatch(rethrow, f, 3), BoostError);
}

TEST(ExceptionInvokeCatch, HandleExceptionAndRethrow_UnknownException)
{
  using namespace qi;
  const auto f = [](int) {
    throw 'z';
  };
  HandleExceptionAndRethrow<PolymorphicConstantFunction<void>> rethrow;
  EXPECT_THROW(invokeCatch(rethrow, f, 3), char);
}

TEST(ExceptionInvokeCatch, HandleExceptionAndRethrowLog_StdException)
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
