#include <boost/bind.hpp>
#include <boost/function.hpp>

#include <qi/log.hpp>
#include <qi/log/consoleloghandler.hpp>

//using qi::log::ConsoleLogHandler;

int main(int argc, char **argv)
{
  //ConsoleLogHandler clh;
  //qi::log::LogFunctionPtr p = boost::bind<void>(&ConsoleLogHandler::log, &clh, _1, _2, _3, _4, _5, _6);
  //boost::function6<void, qi::log::LogLevel, const char *, const char *, int, const char*, va_list> fn;
  //fn = boost::bind<void>(&ConsoleLogHandler::log, &clh, _1, _2, _3, _4, _5, _6);
  //qi::log::setLogHandler(fn);

  qiFatal("%d\n", 42);
  qiError("%d\n", 42);
  qiWarning("%d\n", 42);
  qiInfo("%d\n", 42);
  qiDebug("%d\n", 42);

  qisFatal   << "f" << 42 << std::endl;
  qisError   << "e" << 42 << std::endl;
  qisWarning << "w" << 42 << std::endl;
  qisInfo    << "i" << 42 << std::endl;
  qisDebug   << "d" << 42 << std::endl;
}
