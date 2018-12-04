#include <qi/log/androidloghandler.hpp>

#include <android/log.h>

namespace qi
{
namespace log
{
  void androidLogHandler(LogLevel verb,
                         Clock::time_point,
                         SystemClock::time_point,
                         const char *category,
                         const char *msg,
                         const char */*file*/,
                         const char */*fct*/,
                         int /*line*/)
  {
    int prio = ANDROID_LOG_DEFAULT;
    switch (verb)
    {
      case LogLevel_Silent:   prio = ANDROID_LOG_SILENT;   break;
      case LogLevel_Fatal:    prio = ANDROID_LOG_FATAL;    break;
      case LogLevel_Error:    prio = ANDROID_LOG_ERROR;    break;
      case LogLevel_Warning:  prio = ANDROID_LOG_WARN;     break;
      case LogLevel_Info:     prio = ANDROID_LOG_INFO;     break;
      case LogLevel_Verbose:  prio = ANDROID_LOG_VERBOSE;  break;
      case LogLevel_Debug:    prio = ANDROID_LOG_DEBUG;    break;
      // no default: complete switch
    }
    __android_log_write(prio, category, msg);
  }

  Handler makeAndroidLogHandler() { return androidLogHandler; }

} // namespace log
} // namespace qi
