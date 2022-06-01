/*
 * Copyright (c) 2012 Aldebaran Robotics. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the COPYING file.
 */

// VS2015 fix atomic alignment and require an acknowledgement from developer
#include <boost/predef.h>
#if BOOST_COMP_MSVC
#  if (BOOST_COMP_MSVC >= BOOST_VERSION_NUMBER(14, 0, 0))
#    define _ENABLE_ATOMIC_ALIGNMENT_FIX
#  endif
#endif

#include <qi/assert.hpp>
#include <qi/log.hpp>
#include "log_p.hpp"
#include <qi/os.hpp>
#include <list>
#include <map>
#include <cstring>
#include <iomanip>
#include <iostream>

#include <qi/application.hpp>
#include <qi/atomic.hpp>
#include <qi/log/consoleloghandler.hpp>

#include <boost/thread/thread.hpp>
#include <boost/thread/condition_variable.hpp>
#include <boost/thread/recursive_mutex.hpp>
#include <boost/thread/locks.hpp>
#include <boost/program_options.hpp>
#include <boost/unordered_map.hpp>
#include <boost/algorithm/string.hpp>

#include <boost/lockfree/queue.hpp>
#include <boost/function.hpp>
#include <boost/predef.h>
#include <boost/utility/string_ref.hpp>
#include <boost/iterator/function_output_iterator.hpp>

#ifdef WITH_SYSTEMD
#include <qi/log/journaldloghandler.hpp>
#endif

#ifdef ANDROID
# include <qi/log/androidloghandler.hpp>
#endif

#if BOOST_OS_WINDOWS
# include <shlwapi.h>
# pragma comment(lib, "shlwapi.lib")
// Disable deprecation warnings about `std::auto_ptr`.
# define BOOST_LOCALE_HIDE_AUTO_PTR
# include <boost/locale.hpp>
# undef BOOST_LOCALE_HIDE_AUTO_PTR
# include <boost/filesystem.hpp>
#else
# include <fnmatch.h>
#endif


#define RTLOG_BUFFERS (128)

#define CAT_SIZE 64
#define FILE_SIZE 128
#define FUNC_SIZE 64
#define LOG_SIZE 2048

qiLogCategory("qi.log");

namespace qi {
  namespace detail {

    std::string logline(LogContext                         context,
                        const qi::Clock::time_point        date,
                        const qi::SystemClock::time_point  systemDate,
                        const char                        *category,
                        const char                        *msg,
                        const char                        *file,
                        const char                        *fct,
                        const int                          line,
                        const qi::LogLevel                 verb)
    {
      std::stringstream logline;

      if (context & qi::LogContextAttr_Verbosity)
        logline << qi::log::logLevelToString(verb) << " ";
      if (context & qi::LogContextAttr_ShortVerbosity)
        logline << qi::log::logLevelToString(verb, false) << " ";
      if (context & qi::LogContextAttr_Date)
        logline << qi::detail::dateToString(date) << " ";
      if (context & qi::LogContextAttr_SystemDate)
        logline << qi::detail::dateToString(systemDate) << " ";
      if (context & qi::LogContextAttr_Tid)
        logline << qi::detail::tidToString() << " ";
      if (context & qi::LogContextAttr_Category)
        logline << category << ": ";
      if (context & qi::LogContextAttr_File) {
        logline << file;
        if (line != 0)
          logline << "(" << line << ")";
        logline << " ";
      }
      if (context & qi::LogContextAttr_Function)
        logline << fct << "() ";
      if (context & qi::LogContextAttr_Return)
        logline << std::endl;
      if (msg)
      {
        boost::algorithm::trim_right_copy_if(std::ostream_iterator<char>(logline),
                                             boost::string_ref(msg), &isNewLine);
      }
      logline << std::endl;

      return logline.str();
    }

    std::string logline(LogContext             context,
                        const qi::os::timeval  systemDate,
                        const char            *category,
                        const char            *msg,
                        const char            *file,
                        const char            *fct,
                        const int              line,
                        const qi::LogLevel     verb)
    {
      return logline(context & ~qi::LogContextAttr_Date, // hide date
                     qi::ClockTimePoint(), // date, not printed
                     qi::SystemClockTimePoint(qi::Seconds(systemDate.tv_sec) +
                                              qi::MicroSeconds(systemDate.tv_usec)),
                     category, msg, file, fct, line, verb);
    }

    std::string csvheader()
    {
      std::ostringstream cvsheader;
      cvsheader << "VERBOSITYID,";
      cvsheader << "VERBOSITY,";
      cvsheader << "SVERBOSITY,";
      cvsheader << "DATE,";
      cvsheader << "SYSTEM_DATE,";
      cvsheader << "THREAD_ID,";
      cvsheader << "CATEGORY,";
      cvsheader << "FILE,";
      cvsheader << "LINE,";
      cvsheader << "FUNCTION,";
      cvsheader << "MSG" << std::endl;

      return cvsheader.str();
    }

    std::string csvline(const qi::Clock::time_point date,
                        const qi::SystemClock::time_point systemDate,
                        const char        *category,
                        const char        *msg,
                        const char        *file,
                        const char        *fct,
                        const int          line,
                        const qi::LogLevel verb)
    {
      std::ostringstream csvline;
      csvline << verb << ",";
      csvline << qi::log::logLevelToString(verb) << ",";
      csvline << qi::log::logLevelToString(verb, false) << ",";
      csvline << qi::detail::dateToString(date) << ",";
      csvline << qi::detail::dateToString(systemDate) << ",";
      csvline << qi::detail::tidToString() << ",";

      csvline << "\"";
      csvline << category;
      csvline << "\"";
      csvline << ",";

      csvline << "\"";
      csvline << file;
      csvline << "\"";
      csvline << ",";

      if (line != 0)
        csvline << line;
      csvline << ",";

      csvline << "\"";
      csvline << fct << "()";
      csvline << "\"";
      csvline << ",";

      csvline << "\"";

      if (msg)
      {
        std::string msgStr(msg);
        boost::algorithm::replace_all(msgStr, "\"", "\"\"");
        boost::algorithm::trim_right_copy_if(std::ostream_iterator<char>(csvline), msgStr,
                                             &isNewLine);
      }
      csvline << "\"" << std::endl;

      return csvline.str();
    }

    const std::string dateToString(const qi::os::timeval date)
    {
      std::stringstream ss;
      ss << date.tv_sec << "."
        << std::setw(6) << std::setfill('0') << date.tv_usec;

      return ss.str();
    }

    const std::string tidToString()
    {
      int tid = qi::os::gettid();
      std::stringstream ss;
      ss << tid;

      return ss.str();
    }

    bool isNewLine(char c)
    {
      return c == '\n' || c == '\r';
    }
  }

  namespace log {

    using privateLog = struct sPrivateLog
    {
      qi::LogLevel               _logLevel;
      char                       _category[CAT_SIZE];
      char                       _file[FILE_SIZE];
      char                       _function[FUNC_SIZE];
      int                        _line;
      char                       _log[LOG_SIZE];
      qi::Clock::time_point       _date;
      qi::SystemClock::time_point _systemDate;
    };

    class Log
    {
    public:
      inline Log();
      inline ~Log();

      struct Handler
      {
        qi::log::Handler func;
        unsigned int index; // index of this handler in category levels
      };

      void run();
      void printLog();
      // Invoke handlers who enabled given level/category
      void dispatch_unsynchronized(const qi::LogLevel,
                                   const qi::Clock::time_point date,
                                   const qi::SystemClock::time_point systemDate,
                                   const char*,
                                   const char*,
                                   const char*,
                                   const char*,
                                   int);
      void dispatch_unsynchronized(const qi::LogLevel level,
                                   const qi::Clock::time_point date,
                                   const qi::SystemClock::time_point systemDate,
                                   detail::Category& category,
                                   const char* log,
                                   const char* file,
                                   const char* function,
                                   int line);
      Handler* logHandler(SubscriberId id);

      void setSynchronousLog(bool sync);
    public:
      bool                       LogInit;
      boost::thread              LogThread;
      boost::mutex               LogWriteLock;
      boost::mutex               LogHandlerLock;
      boost::condition_variable  LogReadyCond;
      bool                       SyncLog;
      bool                       AsyncLogInit;

      boost::lockfree::queue<privateLog*>     logs;

      using LogHandlerMap = std::map<std::string, Handler>;
      LogHandlerMap logHandlers;

      qi::Atomic<int> nextIndex;
    };

    // If we receive a setLevel with a globbing category, we must keep it
    // in mind, in case a new category that matches the glob is created.
    struct GlobRule
    {
      GlobRule(std::string t, unsigned int i, qi::LogLevel l)
      : target(t)
      , id(i)
      , level(l)
      {}

      bool matches(const std::string& n) const
      {
        return os::fnmatch(target, n);
      }

      std::string  target;       // glob target
      unsigned int id;           // listener id or -1 for all
      qi::LogLevel level;
    };

    static std::vector<GlobRule> _glGlobRules;

    // categories must be accessible at static init: cannot go in Log class
    using CategoryMap = std::map<std::string, detail::Category*>;
    inline CategoryMap& _categories()
    {
      static CategoryMap* _glCategories;
      QI_ONCE(_glCategories = new CategoryMap);
      return *_glCategories;
    }

    // protects globs and categories, both the map and the per-category vector
    inline boost::recursive_mutex& _mutex()
    {
      static boost::recursive_mutex* _glMutex;
      QI_ONCE(_glMutex = new boost::recursive_mutex());
      return *_glMutex;
    }

    static int                    _glContext = 0;
    static bool                   _glInit    = false;
    static LogColor               _glColorWhen = LogColor_Auto;

    static Log                   *LogInstance = nullptr;
    static privateLog             LogBuffer[RTLOG_BUFFERS];
    static volatile unsigned long LogPush = 0;

    namespace detail {

      // return a log handler on windows, an invalid boost::function otherwise
      Handler makeWindowsDebuggerOutputLogHandler()
      {
#if BOOST_OS_WINDOWS && !defined(NDEBUG)
        auto winDebuggerOutputLogHandler = [](const qi::LogLevel verb, const qi::Clock::time_point date,
            const qi::SystemClock::time_point systemDate,
            const char* category, const char* msg,
            const char* file, const char* fct, const int line)
        {
          if (!IsDebuggerPresent()) // we need to check on each call in case the debuger is attached on the fly
            return;

          // We force long log level names in this specific case.
          auto context = (log::context() | LogContextAttr_Verbosity) & ~LogContextAttr_ShortVerbosity;
          auto logline = qi::detail::logline(context, date, systemDate, category, msg, file, fct, line, verb);

          const auto processId = std::to_wstring(GetCurrentProcessId());

          // Add the process name to help with multi-process debugging:
          static const auto processName = [&] {

            using Path = boost::filesystem::path;

            static const auto getProcessPath = [&] {
              const Path processPath = __argv ? Path{ __argv[0] } : Path{ __wargv[0] };

              // Before providing the process name, we log in the debug ouptut the complete command line
              // so that is is possible to identify processes in multi-process debugging when using several same process names
              // but different arguments:
              {
                std::wstringstream launchMessage;
                launchMessage << L"#### QI Process Logs Tracking Begin (PID=" << processId
                              << L") - Command Line: \n  " << processPath;
                for (int idx = 1; idx < __argc; ++idx)
                {
                  launchMessage << L' ';
                  if (__argv)
                    launchMessage << (__argv[idx] ? __argv[idx] : "<null-argument?>");
                  else
                    launchMessage << (__wargv[idx] ? __wargv[idx] : L"<null-argument?>");
                }
                launchMessage << L'\n';
                OutputDebugStringW(launchMessage.str().c_str());
              }

              return processPath;
            };


            return getProcessPath().filename().wstring() + L'(' + processId + L") > ";

          }();

          // We receive UTF-8 so we want to be able to display unicode characters in this output too:
          auto wlogline = processName + boost::locale::conv::utf_to_utf<wchar_t>(logline.c_str(), logline.c_str() + logline.size());
          OutputDebugStringW(wlogline.c_str());
        };
        return winDebuggerOutputLogHandler;
#else
        return Handler{};
#endif
      }

    }



    static ConsoleLogHandler *_glConsoleLogHandler = nullptr;

    namespace env {
      namespace QI_DEFAULT_LOGHANDLER {
        char const * const name = "QI_DEFAULT_LOGHANDLER";
        namespace value {
          char const * const none     = "none";
          char const * const stdOut   = "stdout";
          char const * const logger   = "logger";
          char const * const debugger = "debugger";
        }
      }
    }

    namespace detail {

      /// Creates and registers the default log handler according to QI_DEFAULT_LOGHANDLER
      /// environment variable, libqi WITH_SYSTEMD build option and the target OS platform.

      /// See documentation of the public function calling this private one: qi::log::init().
      void createAndInstallDefaultHandler(qi::LogLevel verb)
      {
        using namespace qi::log::env;
        auto handler = qi::os::getenv(QI_DEFAULT_LOGHANDLER::name);
        if (handler.empty())
        {
          handler =
#if defined(ANDROID) || defined(WITH_SYSTEMD)
              QI_DEFAULT_LOGHANDLER::value::logger;
#else
              QI_DEFAULT_LOGHANDLER::value::stdOut;
#endif
        }
        const auto invalidId = static_cast<SubscriberId>(-1);
        auto id = invalidId;
        QI_ASSERT(! handler.empty());
        if (handler == QI_DEFAULT_LOGHANDLER::value::stdOut){
          namespace ph = std::placeholders;
          _glConsoleLogHandler = new ConsoleLogHandler;
          id = addHandler("consoleloghandler",
                          boost::bind(&ConsoleLogHandler::log,
                                      _glConsoleLogHandler,
                                      ph::_1, ph::_2, ph::_3, ph::_4, ph::_5, ph::_6, ph::_7, ph::_8),
                          verb);
          QI_ASSERT(id == 0 || id == invalidId);
        }
        else if (handler == QI_DEFAULT_LOGHANDLER::value::logger)
        {
#ifdef ANDROID
          id = addHandler("androidloghandler", makeAndroidLogHandler(), verb);
#elif defined(WITH_SYSTEMD)
          id = addHandler("journaldloghandler", makeJournaldLogHandler(), verb);
#endif
          QI_ASSERT(id == 0 || id == invalidId);
        }
        else if (handler == QI_DEFAULT_LOGHANDLER::value::debugger)
        {
          auto h = makeWindowsDebuggerOutputLogHandler();
          if (h)
          {
            id = addHandler("winDebuggerOutputLogHandler", std::move(h), verb);
          }
          QI_ASSERT(id == 0 || id == invalidId);
        }
        else if (handler == QI_DEFAULT_LOGHANDLER::value::none)
        {
          QI_ASSERT(id == invalidId);
        }
        else
        {
          QI_ASSERT(id == invalidId);
          std::cerr << "qi.log: bad value for " << QI_DEFAULT_LOGHANDLER::name
                    << " environment variable: \"" << handler << "\"."
                    << " Possible values are: \"\","
                    << " \"" << QI_DEFAULT_LOGHANDLER::value::none     << "\","
                    << " \"" << QI_DEFAULT_LOGHANDLER::value::stdOut   << "\","
                    << " \"" << QI_DEFAULT_LOGHANDLER::value::logger   << "\","
                    << " \"" << QI_DEFAULT_LOGHANDLER::value::debugger << "\".\n";
        }
        if (id == invalidId)
        {
          std::cerr << "qi.log: failed to register \"" << handler
                    << "\" log handler. Log messages will be lost until a"
                       " log handler is added.\n";
        }
      }

      void destroyDefaultHandler()
      {
#ifndef ANDROID
        if(_glConsoleLogHandler)
        {
          delete _glConsoleLogHandler;
          _glConsoleLogHandler = nullptr;
        }
#endif
      }
    } // namespace detail

    namespace detail {

      void log(const qi::LogLevel    verb,
               CategoryType          category,
               const char           *categoryStr,
               const char           *msg,
               const char           *file,
               const char           *fct,
               const int             line);

      // This pattern allows to continue logging at static destruction time
      // even if the static FormatMap is destroyed
      class FormatMap: public boost::unordered_map<std::string, boost::format>
      {
      public:
        FormatMap(bool& ward)
        : ward_(ward)
        {
          ward_ = true;
        }

        ~FormatMap()
        {
          ward_ = false;
        }

      private:
        bool& ward_;
      };

      boost::format getFormat(const std::string& s)
      {
        static bool map_ok(false);
        static FormatMap map(map_ok);
        if (map_ok)
        {
          static boost::mutex mutex;
          boost::mutex::scoped_lock lock(mutex);
          FormatMap::iterator i = map.find(s);
          if (i == map.end())
          {
            boost::format& result = map[s]; // creates with default ctor
            result.parse(s);
            result.exceptions(boost::io::no_error_bits);
            return result;
          }
          else
            return i->second;
        }
        else
          {
            boost::format result = boost::format(s);
            result.exceptions(boost::io::no_error_bits);
            return result;
          }
      }
    }

    namespace detail {
      void Category::setLevel(SubscriberId sub, qi::LogLevel level)
      {
        boost::recursive_mutex::scoped_lock lock(_mutex());
        if (levels.size() <= sub)
        {
          bool willUseDefault = (levels.size() < sub);
          levels.resize(sub + 1, LogLevel_Info);
          if (willUseDefault)
          { // should not happen
            // cannot qilog here or deadlock
            std::cerr << "Default level for category " << name
              << " will be used for subscriber " << sub
              << ", use setVerbosity() after adding the subscriber"
              << std::endl;
          }
        }
        levels[sub] = level;
        maxLevel = *std::max_element(levels.begin(), levels.end());
      }
    }

    // check and apply existing glob if they match given category
    static void checkGlobs(detail::Category* cat)
    {
      boost::recursive_mutex::scoped_lock lock(_mutex());
      for (unsigned i=0; i<_glGlobRules.size(); ++i) {
        GlobRule& g = _glGlobRules[i];
        if (g.matches(cat->name))
          cat->setLevel(g.id, g.level);
      }
    }

    // apply a globbing rule to existing categories
    static void applyGlob(const GlobRule& g)
    {
      boost::recursive_mutex::scoped_lock lock(_mutex());
      CategoryMap& c = _categories();
      for (CategoryMap::iterator it = c.begin(); it != c.end(); ++it)
      {
        QI_ASSERT(it->first == it->second->name);
        if (g.matches(it->first)) {
          detail::Category* cat = it->second;
          checkGlobs(cat);
        }
      }
    }

    // Check if globRule replaces an existiing one, then replace or append
    static void mergeGlob(const GlobRule& p)
    {
      boost::recursive_mutex::scoped_lock lock(_mutex());
      for (unsigned i=0; i<_glGlobRules.size(); ++i)
      {
        GlobRule& c = _glGlobRules[i];
        if (p.target == c.target && p.id == c.id)
        {
          c = p;
          return;
        }
      }
      _glGlobRules.push_back(p);
    }

    static class DefaultLogInit
    {
    public:
      DefaultLogInit()
      {
        _glInit = false;
        const std::string logLevel = qi::os::getEnvParam<std::string>("QI_LOG_LEVEL", "info");
        const int context = qi::os::getEnvParam<int>("QI_LOG_CONTEXT", 30);
        _glContext = context;
        const std::string rules = qi::os::getEnvParam<std::string>("QI_LOG_FILTERS", std::string());
        if (!rules.empty())
          addFilters(rules);
        qi::log::init(stringToLogLevel(logLevel.c_str()), context);
      }

      ~DefaultLogInit()
      {
        qi::log::destroy();
      }
    } synchLog;

    void Log::printLog()
    {
      privateLog* pl = nullptr;

      boost::recursive_mutex::scoped_lock lock(_mutex(), boost::defer_lock);
      boost::mutex::scoped_lock lockHandlers(LogInstance->LogHandlerLock, boost::defer_lock);
      boost::lock(lock, lockHandlers);
      while (logs.pop(pl))
      {
        dispatch_unsynchronized(pl->_logLevel, pl->_date, pl->_systemDate, pl->_category, pl->_log,
                                pl->_file, pl->_function, pl->_line);
      }
    }

    void Log::dispatch_unsynchronized(const qi::LogLevel level,
                                      const qi::Clock::time_point date,
                                      const qi::SystemClock::time_point systemDate,
                                      const char* category,
                                      const char* log,
                                      const char* file,
                                      const char* function,
                                      int line)
    {
      dispatch_unsynchronized(level, date, systemDate, *addCategory(category), log, file, function,
                              line);
    }

    void Log::dispatch_unsynchronized(const qi::LogLevel level,
                                      const qi::Clock::time_point date,
                                      const qi::SystemClock::time_point systemDate,
                                      detail::Category& category,
                                      const char* log,
                                      const char* file,
                                      const char* function,
                                      int line)
    {
      if (!logHandlers.empty())
      {
        LogHandlerMap::iterator it;
        for (it = logHandlers.begin(); it != logHandlers.end(); ++it)
        {
          Handler& h = it->second;
          unsigned int index = h.index;
          if (category.levels.size() <= index || category.levels[index] >= level)
            h.func(level, date, systemDate, category.name.c_str(), log, file, function, line);
        }
      }
    }

    void Log::run()
    {
      while (LogInit)
      {
        {
          boost::mutex::scoped_lock lock(LogWriteLock);
          LogReadyCond.wait(lock, []{ return !LogInstance->logs.empty(); });
        }

        printLog();
      }
    }

    void Log::setSynchronousLog(bool sync)
    {
      SyncLog = sync;
      if (!SyncLog && !AsyncLogInit)
      {
        AsyncLogInit = true;
        LogThread = boost::thread(&Log::run, this);
      }
    }

    inline Log::Log() :
      SyncLog(true),
      AsyncLogInit(false)
      , logs(50)
    {
      LogInit = true;
    }

    inline Log::~Log()
    {
      if (!LogInit)
        return;
      LogInit = false;

      if (AsyncLogInit)
      {
        LogThread.interrupt();
        LogThread.join();

        printLog();
      }
    }

    static void my_strcpy(char *dst, const char *src, int len) {
      if (!src)
        src = "(null)";
#ifdef _MSV_VER
      strncpy_s(dst, len, src, _TRUNCATE);
#else
      strncpy(dst, src, len);
      dst[len - 1] = 0;
#endif
    }

    static void doInit(qi::LogLevel verb) {
      //if init has already been called, we are set here. (reallocating all globals
      // will lead to racecond)
      if (_glInit)
        return;

      LogInstance = new Log;
      detail::createAndInstallDefaultHandler(verb);
      _glInit = true;
    }

    void init(qi::LogLevel verb,
              int ctx,
              bool synchronous)
    {
      QI_ONCE(doInit(verb));

      setContext(ctx);

      setSynchronousLog(synchronous);
    }

    void destroy()
    {
      if (!_glInit)
        return;
      _glInit = false;
      LogInstance->printLog();
      detail::destroyDefaultHandler();
      delete LogInstance;
      LogInstance = nullptr;
    }

    void flush()
    {
      if (_glInit)
        LogInstance->printLog();
    }

    void log(const qi::LogLevel    verb,
             CategoryType          category,
             const std::string&    msg,
             const char           *file,
             const char           *fct,
             const int             line)
    {
      if (!isVisible(category, verb))
        return;

      ::qi::log::detail::log(verb, category, category->name.c_str(), msg.c_str(), file, fct, line);
    }

    void log(const qi::LogLevel    verb,
             const char           *categoryStr,
             const char           *msg,
             const char           *file,
             const char           *fct,
             const int             line)
    {
      if (!isVisible(categoryStr, verb))
        return;

      ::qi::log::detail::log(verb, NULL, categoryStr, msg, file, fct, line);
    }

    void detail::log(const qi::LogLevel    verb,
                     CategoryType          category,
                     const char           *categoryStr,
                     const char           *msg,
                     const char           *file,
                     const char           *fct,
                     const int             line)
    {
      if (!LogInstance)
        return;
      if (!LogInstance->LogInit)
        return;

      qi::Clock::time_point date = qi::Clock::now();
      qi::SystemClock::time_point systemDate = qi::SystemClock::now();
      if (LogInstance->SyncLog)
      {
        boost::recursive_mutex::scoped_lock lock(_mutex(), boost::defer_lock);
        boost::mutex::scoped_lock lockHandlers(LogInstance->LogHandlerLock, boost::defer_lock);
        boost::lock(lock, lockHandlers);
        if (category)
          LogInstance->dispatch_unsynchronized(verb, date, systemDate, *category, msg, file, fct,
                                               line);
        else
          LogInstance->dispatch_unsynchronized(verb, date, systemDate, categoryStr, msg, file, fct,
                                               line);
      }
      else
      {
        int tmpRtLogPush = ++LogPush % RTLOG_BUFFERS;
        privateLog* pl = &(LogBuffer[tmpRtLogPush]);

        pl->_logLevel = verb;
        pl->_line = line;
        pl->_date = date;
        pl->_systemDate = systemDate;

        my_strcpy(pl->_category, categoryStr, CAT_SIZE);
        my_strcpy(pl->_file, file, FILE_SIZE);
        my_strcpy(pl->_function, fct, FUNC_SIZE);
        my_strcpy(pl->_log, msg, LOG_SIZE);
        LogInstance->logs.push(pl);
        LogInstance->LogReadyCond.notify_one();
      }
    }

    Log::Handler* Log::logHandler(SubscriberId id)
    {
       boost::mutex::scoped_lock l(LogInstance->LogHandlerLock);
       LogHandlerMap::iterator it;
       for (it = logHandlers.begin(); it != logHandlers.end(); ++it)
       {
         if (it->second.index == id)
           return &it->second;
       }
       return  nullptr;
    }

    void adaptLogFuncHandler(
        logFuncHandler handler,
        const qi::LogLevel verb,
        const qi::Clock::time_point,
        const qi::SystemClock::time_point systemDate,
        const char *category,
        const char *msg,
        const char *file,
        const char *fct,
        const int line)
    {
      handler(verb, qi::os::timeval(systemDate.time_since_epoch()), category,
              msg, file, fct, line);
    }

    SubscriberId addHandler(const std::string& name, Handler fct,
                            qi::LogLevel defaultLevel)
    {
      if (!LogInstance)
        return -1;
      boost::mutex::scoped_lock l(LogInstance->LogHandlerLock);
      unsigned int id = ++LogInstance->nextIndex;
      --id; // no postfix ++ on atomic
      Log::Handler h;
      h.index = id;
      h.func = fct;
      LogInstance->logHandlers[name] = h;
      setLogLevel(defaultLevel, id);
      return id;
    }

    SubscriberId addLogHandler(const std::string& name, logFuncHandler fct,
                               qi::LogLevel defaultLevel)
    {
      namespace ph = std::placeholders;
      return addHandler(name,
          boost::bind(adaptLogFuncHandler,
                      fct, ph::_1, ph::_2, ph::_3, ph::_4, ph::_5, ph::_6, ph::_7, ph::_8),
                      defaultLevel);
    }

    void removeHandler(const std::string& name)
    {
      if (!LogInstance)
        return;
      boost::mutex::scoped_lock l(LogInstance->LogHandlerLock);
      LogInstance->logHandlers.erase(name);
    }

    void removeLogHandler(const std::string& name)
    {
      removeHandler(name);
    }

    qi::LogLevel stringToLogLevel(const char* verb)
    {
      std::string v(verb);
      if (v == "silent" || v == "0")
        return qi::LogLevel_Silent;
      if (v == "fatal" || v == "1")
        return qi::LogLevel_Fatal;
      if (v == "error" || v == "2")
        return qi::LogLevel_Error;
      if (v == "warning" || v == "3")
        return qi::LogLevel_Warning;
      if (v == "info" || v == "4")
        return qi::LogLevel_Info;
      if (v == "verbose" || v == "5")
        return qi::LogLevel_Verbose;
      if (v == "debug" || v == "6")
        return qi::LogLevel_Debug;
      return qi::LogLevel_Info;
    }

    const char *logLevelToString(const qi::LogLevel level, bool verbose)
    {
      static const char *sverb[] = {
        "[SILENT]", // never shown
        "[F]",
        "[E]",
        "[W]",
        "[I]",
        "[V]",
        "[D]"
      };
      static const char *verb[] = {
        "[SILENT]", // never shown
        "[FATAL]",
        "[ERROR]",
        "[WARN ]",
        "[INFO ]",
        "[VERB ]",
        "[DEBUG]"
      };
      QI_ASSERT(level >= 0 && level <= qi::LogLevel_Debug);
      if (verbose)
        return verb[level];
      return sverb[level];
    }

    qi::LogLevel logLevel(SubscriberId sub)
    {
      CategoryType cat = addCategory("*");
      if (sub < cat->levels.size())
        return cat->levels[sub];
      return LogLevel_Info;
    }

    void setContext(int ctx)
    {
      _glContext = ctx;
      qiLogVerbose() << "Context set to " << _glContext;
    }

    int context()
    {
      return _glContext;
    }

    void setColor(LogColor color)
    {
      _glColorWhen = color;
#ifndef ANDROID
      if(_glConsoleLogHandler)
      {
        _glConsoleLogHandler->updateColor();
      }
#endif
    }

    LogColor color()
    {
      return _glColorWhen;
    }

    void setSynchronousLog(bool sync)
    {
      LogInstance->setSynchronousLog(sync);
    }

    CategoryType addCategory(const std::string& name)
    {
      boost::recursive_mutex::scoped_lock lock(_mutex());
      CategoryMap& c = _categories();
      CategoryMap::iterator i = c.find(name);
      if (i == c.end())
      {
        detail::Category* res = new detail::Category(name);
        c[name] = res;
        checkGlobs(res);
        return res;
      }
      else
        return i->second;
    }

    bool isVisible(const std::string& category, qi::LogLevel level)
    {
      return log::isVisible(addCategory(category), level);
    }

    void enableCategory(const std::string& cat, SubscriberId sub)
    {
      addFilter(cat, logLevel(sub), sub);
    }

    void disableCategory(const std::string& cat, SubscriberId sub)
    {
      addFilter(cat, LogLevel_Silent, sub);
    }

    void addFilter(const std::string& catName, qi::LogLevel level, SubscriberId sub)
    {
      qiLogVerbose() << "addFilter(cat=" << catName << ", level=" << (int)level << ", sub=" << (int)sub << ")";
      if (catName.find('*') != catName.npos)
      {
        GlobRule rule(catName, sub, level);
        mergeGlob(rule);
        applyGlob(rule);
      }
      else
      {
        CategoryType cat = addCategory(catName);
        cat->setLevel(sub, level);
        GlobRule rule(catName, sub, level);
        mergeGlob(rule);
      }
    }

    std::vector<std::string> categories()
    {
      boost::recursive_mutex::scoped_lock lock(_mutex());
      std::vector<std::string> res;
      CategoryMap& c = _categories();
      for (CategoryMap::iterator it = c.begin(); it != c.end(); ++it)
        res.push_back(it->first);
      return res;
    }

    void setLogLevel(qi::LogLevel level, SubscriberId sub)
    {
      boost::recursive_mutex::scoped_lock lock(_mutex());
      // Check if there is already a '*' rule, replace it if so
      bool found = false;
      for (unsigned i=0; i<_glGlobRules.size(); ++i)
      {
        if (_glGlobRules[i].target == "*" && _glGlobRules[i].id == sub)
        {
          _glGlobRules[i].level = level;
          found = true;
          break;
        }
      }
      if (!found)
      {
        // Prepend the rule
        GlobRule rule("*", sub, level);
        // Insert the rule with initial '*' rule set, ordered by subscriber id
        // to avoid spurious unset-verbosity warning
        std::vector<GlobRule>::iterator insertIt = _glGlobRules.begin();
        while (insertIt != _glGlobRules.end()
          && insertIt->target == "*" && insertIt->id < sub)
          ++insertIt;
        _glGlobRules.insert(insertIt, rule);
      }
      // Then reprocess all categories
      CategoryMap& c = _categories();
      for (CategoryMap::iterator it = c.begin(); it != c.end(); ++it)
        checkGlobs(it->second);
    }

    namespace detail {
      std::vector<std::tuple<std::string, qi::LogLevel>> parseFilterRules(
          const std::string &rules) {
        std::vector<std::tuple<std::string, qi::LogLevel>> res;
        // See doc in header for format
        size_t pos = 0;
        while (true)
        {
          if (pos >= rules.length())
            break;
          size_t next = rules.find(':', pos);
          std::string token;
          if (next == rules.npos)
            token = rules.substr(pos);
          else
            token = rules.substr(pos, next-pos);
          if (token.empty())
          {
            pos = next + 1;
            continue;
          }
          if (token[0] == '+')
            token = token.substr(1);
          size_t sep = token.find('=');
          if (sep != token.npos)
          {
            std::string sLevel = token.substr(sep+1);
            std::string cat = token.substr(0, sep);
            qi::LogLevel level = stringToLogLevel(sLevel.c_str());
            res.emplace_back(cat, level);
          }
          else
          {
            if (token[0] == '-')
              res.emplace_back(token.substr(1), LogLevel_Silent);
            else
              res.emplace_back(token, LogLevel_Debug);
          }
          if (next == rules.npos)
            break;
          pos = next+1;
        }
        return res;
      }
    }

    void addFilters(const std::string& rules, SubscriberId sub)
    {
      std::string cat;
      qi::LogLevel level;
      for (auto &&p: detail::parseFilterRules(rules))
      {
        std::tie(cat, level) = std::move(p);
        addFilter(cat, level, sub);
      }
    }

    static void _setLogLevel(const std::string &level)
    {
      setLogLevel(stringToLogLevel(level.c_str()), 0u);
    }

    static void _setColor(const std::string &color)
    {
      if (color == "always")
        setColor(LogColor_Always);
      else if (color == "never")
        setColor(LogColor_Never);
      else
        setColor(LogColor_Auto);
    }

    static void _setFilters(const std::string &filters)
    {
      addFilters(filters);
    }

    static const std::string contextLogOption = ""
        "Show context logs, it's a bit field (add the values below):\n"
        " 1  : Verbosity\n"
        " 2  : ShortVerbosity\n"
        " 4  : SystemDate\n"
        " 8  : ThreadId\n"
        " 16 : Category\n"
        " 32 : File\n"
        " 64 : Function\n"
        " 128: EndOfLine\n"
        " 256: Date\n"
        "some useful values for context are:\n"
        " 26 : (verb+threadId+cat)\n"
        " 30 : (verb+threadId+date+cat)\n"
        " 126: (verb+threadId+date+cat+file+fun)\n"
        " 254: (verb+threadId+date+cat+file+fun+eol)\n"
        "Can be set with env var QI_LOG_CONTEXT";

    static const std::string levelLogOption = ""
        "Change the log minimum level: [0-6] (default:4)\n"
        " 0: silent\n"
        " 1: fatal\n"
        " 2: error\n"
        " 3: warning\n"
        " 4: info\n"
        " 5: verbose\n"
        " 6: debug\n"
        "Can be set with env var QI_LOG_LEVEL";

    static const std::string filterLogOption = ""
        "Set log filtering options.\n"

        " Colon separated list of rules.\n"
        " Each rule can be:\n"
        "  - +CAT      : enable category CAT\n"
        "  - -CAT      : disable category CAT\n"
        "  - CAT=level : set category CAT to level\n"
        " Each category can include a '*' for globbing.\n"
        "Can be set with env var QI_LOG_FILTERS\n"
        "Example: 'qi.*=debug:-qi.foo:+qi.foo.bar' (all qi.* logs in info, remove all qi.foo logs except qi.foo.bar)";

    _QI_COMMAND_LINE_OPTIONS(
      "Logging options",
      ("qi-log-context",     value<int>()->notifier(&setContext), contextLogOption.c_str())
      ("qi-log-synchronous", bool_switch()->notifier(boost::bind(&setSynchronousLog, true)),  "Activate synchronous logs.")
      ("qi-log-level",       value<std::string>()->notifier(&_setLogLevel), levelLogOption.c_str())
      ("qi-log-color",       value<std::string>()->notifier(&_setColor), "Tell if we should put color or not in log (auto, always, never).")
      ("qi-log-filters",     value<std::string>()->notifier(&_setFilters), filterLogOption.c_str())
    )

    // deprecated
    qi::LogLevel verbosity(SubscriberId sub)
    {
      return logLevel(sub);
    }

    // deprecated
    void setVerbosity(qi::LogLevel level, SubscriberId sub)
    {
      setLogLevel(level, sub);
    }

    // deprecated
    void setVerbosity(const std::string& rules, SubscriberId sub)
    {
      addFilters(rules, sub);
    }

    // deprecated
    void setCategory(const std::string& catName, qi::LogLevel level, SubscriberId sub)
    {
      addFilter(catName, level, sub);
    }

  } // namespace log
} // namespace qi

