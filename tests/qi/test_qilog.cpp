#include "test_qilog.hpp"
#include <boost/function.hpp>
#include <boost/utility/string_ref.hpp>

LogHandler::LogHandler(const std::string& name, qi::log::Handler handler, qi::LogLevel level)
  : name(name)
  , id(qi::log::addHandler(name, std::move(handler), level))
{
}

LogHandler::~LogHandler()
{
  qi::log::removeHandler(name);
}


MockLogHandler::MockLogHandler(const std::string& name)
  : handler(name, std::ref(*this))
{
  // TODO: Replace unit_t by void once we upgrade GoogleMock with the fix on mock methods returning
  // void crashing in optimized compilation on recent compilers.
  // See: https://github.com/google/googletest/issues/705
  using testing::_;
  ON_CALL(*this, log(_, _, _)).WillByDefault(testing::Return(ka::unit));
}


void MockLogHandler::operator()(qi::LogLevel level,
                                qi::Clock::time_point,
                                qi::SystemClock::time_point,
                                const char* category,
                                const char* message,
                                const char*,
                                const char*,
                                int)
{
  boost::string_ref catStrRef{ category };
  // remove log from the logger itself and the eventloop
  if (catStrRef == "qi.log" || catStrRef == "qi.eventloop")
    return;
  this->log(level, category, message);
}
