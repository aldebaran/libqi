/*
 * Copyright (c) 2012 Aldebaran Robotics. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the COPYING file.
 */

#include <qi/log.hpp>
#include "log_p.hpp"
#include <qi/os.hpp>
#include <list>
#include <map>
#include <cstring>
#include <iomanip>

#include <qi/application.hpp>
#include <qi/atomic.hpp>
#include <qi/log/consoleloghandler.hpp>

#include <boost/thread/thread.hpp>
#include <boost/thread/condition_variable.hpp>
#include <boost/program_options.hpp>
#include <boost/unordered_map.hpp>

#ifdef QI_USE_BOOST_LOCK_FREE
# include <boost/lockfree/fifo.hpp>
#else
# include <queue>
#endif
#include <boost/function.hpp>

#ifdef ANDROID
# include <android/log.h>
#endif

#ifndef _WIN32
#include <fnmatch.h>
#else
# include <shlwapi.h>
# pragma comment(lib, "shlwapi.lib")
#endif


#define RTLOG_BUFFERS (128)

#define CAT_SIZE 64
#define FILE_SIZE 128
#define FUNC_SIZE 64
#define LOG_SIZE 2048

namespace qi {
  namespace detail {
    int categoriesFromContext()
    {
      int ret = LOG_VERBOSITY;
      switch (qi::log::context())
      {
        case 1:
          ret |= LOG_CATEGORY;
          break;
        case 2:
          ret |= LOG_DATE;
          break;
        case 3:
          ret |= LOG_FILE;
          break;
        case 4:
          ret |= LOG_DATE | LOG_CATEGORY;
          break;
        case 5:
          ret |= LOG_DATE | LOG_FILE;
          break;
        case 6:
          ret |= LOG_CATEGORY | LOG_FILE;
          break;
        case 7:
          ret |= LOG_DATE | LOG_TID | LOG_CATEGORY | LOG_FILE | LOG_FUNCTION;
          break;
      }
      return ret;
    }

    std::string logline(const              os::timeval date,
                        const char        *category,
                        const char        *msg,
                        const char        *file,
                        const char        *fct,
                        const int          line,
                        const qi::log::LogLevel     verb
                       )
    {
      int categories = qi::detail::categoriesFromContext();
      std::stringstream logline;

      if (verb != qi::log::silent && categories & qi::detail::LOG_VERBOSITY)
        logline << qi::log::logLevelToString(verb) << " ";
      if (categories & qi::detail::LOG_DATE)
        logline << qi::detail::dateToString(date) << " ";
      if (categories & qi::detail::LOG_TID)
        logline << qi::detail::tidToString() << " ";
      if (categories & qi::detail::LOG_CATEGORY)
        logline << qi::detail::categoryToFixedCategory(category) << ": ";
      if (categories & qi::detail::LOG_FILE) {
        logline << file;
        if (line != 0)
          logline << "(" << line << ")";
        logline << " ";
      }
      if (categories & qi::detail::LOG_FUNCTION)
        logline << fct << "() ";
      logline.write(msg, qi::detail::rtrim(msg));
      logline << std::endl;

      return logline.str();
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

    const std::string categoryToFixedCategory(const char *category, int size)
    {
      if (size == 0)
        return std::string(category);

      std::string fixedCategory = category;
      fixedCategory.resize(size, ' ');
      return fixedCategory;
    }

    /* Emulate previous behavior that ensured a single newline was
    * present at the end on message.
    */
    int rtrim(const char *msg)
    {
      size_t p = strlen(msg) - 1;

      p -= (msg[p] == '\r')? 1:
             (msg[p] == '\n')?
               (p && msg[p-1] == '\r')? 2:1
               :0;

      return p+1;
    }
  }

  namespace log {

    typedef struct sPrivateLog
    {
      LogLevel        _logLevel;
      char            _category[CAT_SIZE];
      char            _file[FILE_SIZE];
      char            _function[FUNC_SIZE];
      int             _line;
      char            _log[LOG_SIZE];
      qi::os::timeval _date;
    } privateLog;

    class Log
    {
    public:
      inline Log();
      inline ~Log();

      struct Handler
      {
        logFuncHandler func;
        unsigned int index; // index of this handler in category levels
        LogLevel verbosity; // max verbosity this handler accepts
      };

      void run();
      void printLog();
      // Invoke handlers who enabled given level/category
      void dispatch(const qi::log::LogLevel,
                    const qi::os::timeval,
                    const char*,
                    const char*,
                    const char*,
                    const char*,
                    int);
      void dispatch(const qi::log::LogLevel level,
                    const qi::os::timeval date,
                    detail::Category& category,
                    const char* log,
                    const char* file,
                    const char* function,
                    int line);
      Handler* logHandler(Subscriber id);
    public:
      bool                       LogInit;
      boost::thread              LogThread;
      boost::mutex               LogWriteLock;
      boost::mutex               LogHandlerLock;
      boost::condition_variable  LogReadyCond;

#ifdef QI_USE_BOOST_LOCK_FREE
      boost::lockfree::fifo<privateLog*>     logs;
#else
      std::queue<privateLog*>     logs;
#endif

      typedef std::map<std::string, Handler> LogHandlerMap;
      LogHandlerMap logHandlers;

      qi::atomic<int> nextIndex;
    };

    // If we receive a setLevel with a globbing category, we must keep it
    // in mind, in case a new category that matches the glob is created.
    struct GlobRule
    {
      GlobRule(std::string t, unsigned int i, LogLevel l)
      : target(t) , id(i), level(l) {}
      bool matches(const std::string& n) const
      {
        return os::fnmatch(target, n);
      }
      std::string target; // glob target
      unsigned int id; // listener id or -1 for all
      LogLevel level;
      static const unsigned int ALL = 0xFFFF; // std::numeric_limits<unsigned int>
    };
    std::vector<GlobRule> _glGlobRules;

    // categories must be accessible at static init: cannot go in Log class
    typedef std::map<std::string, detail::Category*> CategoryMap;
    static CategoryMap*   _glCategories = 0;
    inline CategoryMap& _categories()
    {
      if (!_glCategories)
        _glCategories = new CategoryMap;
      return *_glCategories;
    }

    static LogLevel               _glVerbosity = qi::log::info;
    static int                    _glContext = false;
    static bool                   _glSyncLog = false;
    static bool                   _glInit    = false;
    static ConsoleLogHandler      *_glConsoleLogHandler;
    static ColorWhen              _glColorWhen = COLOR_AUTO;

    static Log                    *LogInstance;
    static privateLog             LogBuffer[RTLOG_BUFFERS];
    static volatile unsigned long LogPush = 0;

    namespace detail {
      LogLevel* globalLogLevelPtr()
      {
        return &_glVerbosity;
      }
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

      boost::format
      getFormat(const std::string& s)
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

    // check and apply existing glob if they match given category
    static void checkGlobs(detail::Category* cat)
    {
      for (unsigned i=0; i<_glGlobRules.size(); ++i)
      {
        GlobRule& g = _glGlobRules[i];
        if (g.matches(cat->name))
        {
          if (g.id != GlobRule::ALL)
          {
            if (cat->levels.size() <= g.id)
              cat->levels.resize(g.id + 1, debug);
            cat->levels[g.id] = g.level;
          }
          else
            cat->mainLevel = g.level;
        }
      }
    }

    // apply a globbing rule to existing categories
    static void applyGlob(const GlobRule& g)
    {
      CategoryMap& c = _categories();
      for (CategoryMap::iterator it = c.begin(); it != c.end(); ++it)
      {
        assert(it->first == it->second->name);
        if (g.matches(it->first))
        {
          detail::Category* cat = it->second;
          if (g.id != GlobRule::ALL)
          {
            if (cat->levels.size() <= g.id)
              cat->levels.resize(g.id + 1, debug);
            cat->levels[g.id] = g.level;
          }
          else
            cat->mainLevel = g.level;
        }
      }
    }

    // Check if globRule replaces an existiing one, then replace or append
    static void mergeGlob(const GlobRule& p)
    {
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
        qi::log::init();
      }

      ~DefaultLogInit()
      {
        qi::log::destroy();
      };
    } synchLog;

    void Log::printLog()
    {
      privateLog* pl;
      boost::mutex::scoped_lock lock(LogHandlerLock);
#ifdef QI_USE_BOOST_LOCK_FREE
      while (logs.dequeue(&pl))
      {

#else
      while ((pl = logs.front()) != 0)
      {
        logs.pop();
#endif
        dispatch(pl->_logLevel,
                 pl->_date,
                 pl->_category,
                 pl->_log,
                 pl->_file,
                 pl->_function,
                 pl->_line);
      }
    }

    void Log::dispatch(const qi::log::LogLevel level,
                       const qi::os::timeval date,
                       const char*  category,
                       const char* log,
                       const char* file,
                       const char* function,
                       int line)
    {
      dispatch(level, date, *addCategory(category), log, file, function, line);
    }

    void Log::dispatch(const qi::log::LogLevel level,
                       const qi::os::timeval date,
                       detail::Category& category,
                       const char* log,
                       const char* file,
                       const char* function,
                       int line)
    {
      if (!logHandlers.empty())
      {
        LogHandlerMap::iterator it;
        for (it = logHandlers.begin();
               it != logHandlers.end(); ++it)
        {
          Handler& h = it->second;
          unsigned int index = h.index;
          if (h.verbosity >= level && (category.levels.size() <= index || category.levels[index] >= level))
            h.func(level, date, category.name.c_str(), log, file, function, line);
        }
      }
    }

    void Log::run()
    {
      while (LogInit)
      {
        {
          boost::mutex::scoped_lock lock(LogWriteLock);
          LogReadyCond.wait(lock);
        }

        printLog();
      }
    }

    inline Log::Log()
    {
      LogInit = true;
      if (!_glSyncLog)
        LogThread = boost::thread(&Log::run, this);
    };

    inline Log::~Log()
    {
      if (!LogInit)
        return;

      LogInit = false;

      if (!_glSyncLog)
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

    void init(qi::log::LogLevel verb,
              int ctx,
              bool synchronous)
    {
      setVerbosity(verb);
      setContext(ctx);
      setSynchronousLog(synchronous);

      if (_glInit)
        destroy();

      _glConsoleLogHandler = new ConsoleLogHandler;
      LogInstance          = new Log;
      addLogHandler("consoleloghandler",
                    boost::bind(&ConsoleLogHandler::log,
                                _glConsoleLogHandler,
                                _1, _2, _3, _4, _5, _6, _7));
      _glInit = true;
    }

    void destroy()
    {
      if (!_glInit)
        return;
      _glInit = false;
      LogInstance->printLog();
      delete _glConsoleLogHandler;
      _glConsoleLogHandler = 0;
      delete LogInstance;
      LogInstance = 0;
    }

    void flush()
    {
      if (_glInit)
        LogInstance->printLog();
    }

    void log(const LogLevel        verb,
             category_type        category,
             const std::string&   msg,
             const char           *file,
             const char           *fct,
             const int             line)
    {
      #ifndef ANDROID
      if (_glSyncLog)
      {
        if (!detail::isVisible(category, verb))
          return;
        qi::os::timeval tv;
        qi::os::gettimeofday(&tv);
        LogInstance->dispatch(verb, tv, *category, msg.c_str(), file, fct, line);
      }
      else
      #endif
      // FIXME suboptimal
      log(verb, category->name.c_str(), msg.c_str(), file, fct, line);
    }

    void log(const LogLevel        verb,
             const char           *category,
             const char           *msg,
             const char           *file,
             const char           *fct,
             const int             line)
    {
      if (!isVisible(category, verb))
        return;
      #ifdef ANDROID
        std::map<LogLevel, android_LogPriority> _conv;

        _conv[silent]  = ANDROID_LOG_SILENT;
        _conv[fatal]   = ANDROID_LOG_FATAL;
        _conv[error]   = ANDROID_LOG_ERROR;
        _conv[warning] = ANDROID_LOG_WARN;
        _conv[info]    = ANDROID_LOG_INFO;
        _conv[verbose] = ANDROID_LOG_VERBOSE;
        _conv[debug]   = ANDROID_LOG_DEBUG;

        __android_log_print(_conv[verb], category, msg);
        return;
      #endif

      if (!LogInstance)
        return;
      if (!LogInstance->LogInit)
        return;

      qi::os::timeval tv;
      qi::os::gettimeofday(&tv);
      if (_glSyncLog)
      {
        LogInstance->dispatch(verb, tv, category, msg, file, fct, line);
      }
      else
      {
        int tmpRtLogPush = ++LogPush % RTLOG_BUFFERS;
        privateLog* pl = &(LogBuffer[tmpRtLogPush]);



        pl->_logLevel = verb;
        pl->_line = line;
        pl->_date.tv_sec = tv.tv_sec;
        pl->_date.tv_usec = tv.tv_usec;

        my_strcpy(pl->_category, category, CAT_SIZE);
        my_strcpy(pl->_file, file, FILE_SIZE);
        my_strcpy(pl->_function, fct, FUNC_SIZE);
        my_strcpy(pl->_log, msg, LOG_SIZE);
#ifdef QI_USE_BOOST_LOCK_FREE
        LogInstance->logs.enqueue(pl);
#else
        LogInstance->logs.push(pl);
#endif
        LogInstance->LogReadyCond.notify_one();
      }
    }

    Log::Handler* Log::logHandler(Subscriber id)
    {
       boost::mutex::scoped_lock l(LogInstance->LogHandlerLock);
       LogHandlerMap::iterator it;
       for (it = logHandlers.begin(); it != logHandlers.end(); ++it)
       {
         if (it->second.index == id)
           return &it->second;
       }
       return 0;
    }

    Subscriber addLogHandler(const std::string& name, logFuncHandler fct)
    {
      if (!LogInstance)
        return -1;
      boost::mutex::scoped_lock l(LogInstance->LogHandlerLock);
      unsigned int id = ++LogInstance->nextIndex;
      --id; // no postfix ++ on atomic
      Log::Handler h;
      h.index = id;
      h.func = fct;
      h.verbosity = debug;
      LogInstance->logHandlers[name] = h;
      return id;
    }

    void removeLogHandler(const std::string& name)
    {
      if (!LogInstance)
        return;
      boost::mutex::scoped_lock l(LogInstance->LogHandlerLock);
      LogInstance->logHandlers.erase(name);
    }

    LogLevel stringToLogLevel(const char* verb)
    {
      std::string v(verb);
      if (v == "silent")
        return qi::log::silent;
      if (v == "fatal")
        return qi::log::fatal;
      if (v == "error")
        return qi::log::error;
      if (v == "warning")
        return qi::log::warning;
      if (v == "info")
        return qi::log::info;
      if (v == "verbose")
        return qi::log::verbose;
      if (v == "debug")
        return qi::log::debug;
      return qi::log::info;
    }

    const char *logLevelToString(const LogLevel verb)
    {
      static const char *sverb[] = {
        "[SILENT]", // never shown
        "[FATAL]",
        "[ERROR]",
        "[WARN ]",
        "[INFO ]",
        "[VERB ]",
        "[DEBUG]"
      };
      return sverb[verb];
    }

    void setVerbosity(const LogLevel lv)
    {
      _glVerbosity = lv;

      qiLogVerbose("qi.log") << "Verbosity set to " << _glVerbosity;
    };

    LogLevel verbosity()
    {
      return _glVerbosity;
    };

    void setContext(int ctx)
    {

      _glContext = ctx;

      qiLogVerbose("qi.log") << "Context set to " << _glContext;
    };

    int context()
    {
      return _glContext;
    };

    void setColor(ColorWhen color)
    {
      _glColorWhen = color;
      _glConsoleLogHandler->updateColor();
    };

    ColorWhen color()
    {
      return _glColorWhen;
    }

    void setSynchronousLog(bool sync)
    {
      _glSyncLog = sync;
    };

    category_type addCategory(const std::string& name)
    {
      CategoryMap& c = _categories();
      CategoryMap::iterator i = c.find(name);
      if (i == c.end())
      {
        detail::Category* res = new detail::Category;
        res->name = name;
        res->mainLevel = debug;
        c[name] = res;
        checkGlobs(res);
        return res;
      }
      else
        return i->second;
    }

    bool isVisible(const std::string& category, LogLevel level)
    {
      return log::isVisible(addCategory(category), level);
    }

    bool isVisible(category_type category, LogLevel level)
    {
      return detail::isVisible(category, level);
    }

    void enableCategory(const std::string& cat)
    {
      setCategory(cat, _glVerbosity);
    }

    void disableCategory(const std::string& cat)
    {
      setCategory(cat, silent);
    }

    void setCategory(const std::string& cat, LogLevel level)
    {
      if (cat.find('*') != cat.npos)
      {
        GlobRule rule(cat, GlobRule::ALL, level);
        applyGlob(rule);
        mergeGlob(rule);
      }
      else
        addCategory(cat)->mainLevel = level;
    }

    void setCategory(Subscriber sub, const std::string& catName, LogLevel level)
    {
      if (catName.find('*') != catName.npos)
      {
        GlobRule rule(catName, sub, level);
        applyGlob(rule);
        mergeGlob(rule);
      }
      else
      {
        category_type cat = addCategory(catName);
        if (cat->levels.size() <= sub)
          cat->levels.resize(sub + 1, debug);
        cat->levels[sub] = level;
      }
    }

    std::vector<std::string> categories()
    {
      std::vector<std::string> res;
      CategoryMap& c = _categories();
      for (CategoryMap::iterator it = c.begin(); it != c.end(); ++it)
        res.push_back(it->first);
      return res;
    }

    void setVerbosity(Subscriber sub, LogLevel level)
    {
      LogInstance->logHandler(sub)->verbosity = level;
    }

    void setVerbosity(const std::string& rules)
    {
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
          LogLevel level = stringToLogLevel(sLevel.c_str());
          setCategory(cat, level);
        }
        else
        {
          if (token[0] == '-')
            setCategory(token.substr(1), silent);
          else
            setCategory(token, debug);
        }
        if (next == rules.npos)
          break;
        pos = next+1;
      }
    }

    static void _setVerbosityInt(int v)
    {
      setVerbosity((LogLevel)v);
    }

    static void _setVerbose(bool on)
    {
      if (on)
        setVerbosity(verbose);
    }
    static void _setDebug(bool on)
    {
      if (on)
        setVerbosity(debug);
    }
    static void _quiet(bool on)
    {
      if (on)
        removeLogHandler("consoleloghandler");
    }
    static void _setColor(const std::string &color)
    {
      if (color == "always")
        setColor(COLOR_ALWAYS);
      else if (color == "never")
        setColor(COLOR_NEVER);
      else
        setColor(COLOR_AUTO);
    }

    _QI_COMMAND_LINE_OPTIONS(
      "Logging options",
      ("verbose,v", bool_switch()->notifier(&_setVerbose), "Set verbose verbosity.")
      ("debug,d", bool_switch()->notifier(&_setDebug), "Set debug verbosity.")
      ("quiet,q",  bool_switch()->notifier(&_quiet), "Do not show logs on console.")
      ("context,c", value<int>()->notifier(&setContext), "Show context logs: [0-7] (0: none, 1: categories, 2: date, 3: file+line, 4: date+categories, 5: date+line+file, 6: categories+line+file, 7: all (date+categories+line+file+function)).")
      ("synchronous-log", bool_switch()->notifier(boost::bind(&setSynchronousLog, true)),  "Activate synchronous logs.")
      ("log-level,L", value<int>()->notifier(&_setVerbosityInt), "Change the log minimum level: [0-6] (0: silent, 1: fatal, 2: error, 3: warning, 4: info, 5: verbose, 6: debug). Default: 4 (info)")
      ("color", value<std::string>()->notifier(&_setColor), "Tell if we should put color or not in log (auto, always, never).")
      )

    int process_env()
    {
      const char* verbose = std::getenv("VERBOSE");
      if (verbose)
        setVerbosity((LogLevel)atoi(verbose));
      const char *context = std::getenv("CONTEXT");
      if (context)
        _glContext = (atoi(context));
      const char* rules = std::getenv("QI_LOG_CATEGORIES");
      if (rules)
        setVerbosity(rules);
      return 0;
    }
    static int _init = process_env();
  } // namespace log
} // namespace qi

