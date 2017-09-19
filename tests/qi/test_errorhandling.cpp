#include <array>
#include <stdexcept>
#include <string>
#include <boost/algorithm/string.hpp>
#include <gtest/gtest.h>
#include <ka/conceptpredicate.hpp>
#include <ka/errorhandling.hpp>
#include <ka/scoped.hpp>
#include <qi/clock.hpp>
#include <qi/log.hpp>

struct boost_error_t : boost::exception
{
  std::string msg;
  boost_error_t(const std::string& s = std::string{}) : msg(s)
  {
  }
  std::ostream& operator<<(std::ostream& o) const
  {
    return o << "BoostError{\"" << msg << "\"}";
  }
};

namespace
{
  const auto std_code = 55;
  const auto boost_code = 56;
  const auto unknown_code = 57;
  const ka::exception_value<int> exception_code{std_code, boost_code, unknown_code};

  template<typename T>
  int code()
  {
    return unknown_code;
  }

  template<>
  int code<std::runtime_error>()
  {
    return std_code;
  }

  template<>
  int code<boost_error_t>()
  {
    return boost_code;
  }
}

TEST(ExceptionLogError, Basic)
{
  using namespace qi;
  using namespace ka;
  using namespace boost::algorithm;
  static const std::string prefix{"prefix"}, handlerName{"test"};
  const std::array<const char*, 3> expected{"standard exception", "boost exception", "unknown exception"};
  int i = 0;
  log::addHandler(handlerName,
    [&](LogLevel, Clock::time_point, SystemClock::time_point, const char*, std::string msg, const char*, const char*, int) {
      const auto exp = prefix + ": " + expected.at(i);
      if (!starts_with(msg, exp) && msg != "Ping!")
        throw std::runtime_error("'" + msg + "' does not start with '" + exp + "'");
      ++i;
    }
  );
  auto s = ka::scoped([&] {
    log::removeHandler(handlerName);
  });
  ExceptionLogError<std::string> log{"category", prefix};
  log(std::runtime_error{""});
  log(boost_error_t{""});
  log();
}

TEST(ExceptionLogError, Regular)
{
  using namespace qi;
  using F = ExceptionLogError<std::string>;
  // This test is partial but representative.
  ASSERT_TRUE(ka::is_regular(
    {F{"a", "a"}, F{"a", "b"}, F{"b", "b"}, F{"ab", "b"}}));
}
