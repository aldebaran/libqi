#include "test_qilog.hpp"
#include <boost/function.hpp>
#include <boost/utility/string_ref.hpp>

LogHandler::LogHandler(const std::string& name, qi::log::Handler handler)
  : name(name)
  , id(qi::log::addHandler(name, std::move(handler)))
{
}

LogHandler::~LogHandler()
{
  qi::log::removeHandler(name);
}


MockLogHandler::MockLogHandler(const std::string& name)
  : handler(name, std::ref(*this))
{
}


void MockLogHandler::operator()(qi::LogLevel,
                                qi::Clock::time_point,
                                qi::SystemClock::time_point,
                                const char* category,
                                const char* message,
                                const char*,
                                const char*,
                                int)
{
  // remove log from the logger itself
  if (boost::string_ref(category) == "qi.log")
    return;
  this->log(message);
}
