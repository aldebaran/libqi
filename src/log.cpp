/*
 *  Author(s):
 *  - Chris  Kilner <ckilner@aldebaran-robotics.com>
 *  - Cedric Gestes <gestes@aldebaran-robotics.com>
 *
 *  Copyright (C) 2010, 2011 Aldebaran Robotics
 */

#include <qi/log.hpp>
#include <qi/os.hpp>
#include <list>
#include <map>

#include <qi/log/consoleloghandler.hpp>

#include <boost/thread/thread.hpp>
#include <boost/thread/condition_variable.hpp>
#include <boost/lockfree/fifo.hpp>
#include <boost/function.hpp>

#define RTLOG_BUFFERS (128)

#define CAT_SIZE 64
#define FILE_SIZE 128
#define FUNC_SIZE 64
#define LOG_SIZE 2048

namespace qi {
  namespace log {
    static LogLevel _glVerbosity;
    static bool _glContext;

    typedef struct sPrivateLog
    {
      LogLevel _logLevel;
      char     _category[CAT_SIZE];
      char     _file[FILE_SIZE];
      char     _function[FUNC_SIZE];
      int      _line;
      char     _log[LOG_SIZE];
    } privateLog;

    static privateLog             rtLogBuffer[RTLOG_BUFFERS];
    static volatile unsigned long rtLogPush = 0;

    static ConsoleLogHandler gConsoleLogHandler;

    static class rtLog
    {
    public:
      inline rtLog();
      inline ~rtLog();

      void run();
      void printLog();

    public:
      bool                       rtLogInit;
      boost::thread              rtLogThread;
      boost::mutex               rtLogWriteLock;
      boost::condition_variable  rtLogReadyCond;

      boost::lockfree::fifo<privateLog*>     logs;
      std::map<std::string, logFuncHandler > logHandlers;
    } rtLogInstance;


    void rtLog::printLog()
    {
      privateLog* pl;
      while (logs.dequeue(&pl))
      {
        boost::mutex::scoped_lock lock(rtLogWriteLock);
        if (!logHandlers.empty())
        {
          std::map<std::string, logFuncHandler >::iterator it;
          for (it = logHandlers.begin();
               it != logHandlers.end(); ++it)
          {
            (*it).second(pl->_logLevel,
                         pl->_file,
                         pl->_function,
                         pl->_category,
                         pl->_line,
                         pl->_log);
          }
        }
      }
    }

    void rtLog::run()
    {
      while (rtLogInit)
      {
        {
          boost::mutex::scoped_lock lock(rtLogWriteLock);
          rtLogReadyCond.wait(lock);
        }

        printLog();
      }
    };

    inline rtLog::rtLog()
    {
      rtLogInit = true;
      rtLogThread = boost::thread(&rtLog::run, &rtLogInstance);
    };

    inline rtLog::~rtLog()
    {
      if (!rtLogInit)
        return;

      rtLogInit = false;

      rtLogReadyCond.notify_one();
      if (rtLogThread.joinable())
      {
        rtLogThread.join();
      }

      printLog();
    }

    void log(const LogLevel    verb,
             const char       *file,
             const char       *fct,
             const char       *category,
             const int         line,
             const char       *msg)

    {
      if (!rtLogInstance.rtLogInit)
        return;

      int tmpRtLogPush = ++rtLogPush % RTLOG_BUFFERS;
      privateLog* pl = &(rtLogBuffer[tmpRtLogPush]);

      pl->_logLevel = verb;
      pl->_line = line;

#ifdef _MSC_VER
      if (category != NULL)
        strcpy_s(pl->_category, CAT_SIZE, category);
      else
        strcpy_s(pl->_category, CAT_SIZE, "(null)");
      if (file != NULL)
        strcpy_s(pl->_file, FILE_SIZE, file);
      else
        strcpy_s(pl->_file, CAT_SIZE, "(null)");
      if (fct != NULL)
        strcpy_s(pl->_function, FUNC_SIZE, fct);
      else
        strcpy_s(pl->_function, CAT_SIZE, "(null)");
      if (msg != NULL)
        strcpy_s(pl->_log, LOG_SIZE, msg);
      else
        strcpy_s(pl->_log, CAT_SIZE, "(null)");
#else
      if (category != NULL)
        strncpy(pl->_category, category, CAT_SIZE);
      else
        strncpy(pl->_category, "(null)", CAT_SIZE);
      if (file != NULL)
        strncpy(pl->_file, file, FILE_SIZE);
      else
        strncpy(pl->_file, "(null)", FILE_SIZE);
      if (fct != NULL)
        strncpy(pl->_function, fct, FUNC_SIZE);
      else
        strncpy(pl->_function, "(null)", FUNC_SIZE);
      if (msg != NULL)
        strncpy(pl->_log, msg, LOG_SIZE);
      else
        strncpy(pl->_log, "(null)", LOG_SIZE);
#endif

      pl->_category[CAT_SIZE - 1] = '\0';
      pl->_file[FILE_SIZE - 1] = '\0';
      pl->_function[FUNC_SIZE - 1] = '\0';
      pl->_log[LOG_SIZE - 1] = '\0';

      rtLogInstance.logs.enqueue(pl);
      rtLogInstance.rtLogReadyCond.notify_one();
    }

    void consoleLogHandler(const LogLevel    verb,
                           const char       *file,
                           const char       *fct,
                           const char       *category,
                           const int         line,
                           const char       *msg)
    {
      gConsoleLogHandler.log(verb, file, fct, category, line, msg);
    }



    static class LogHandlerInit
    {
    public:
      LogHandlerInit()
      {
        addLogHandler(consoleLogHandler, "consoleloghandler");
      }
    } gLogHandlerInit;


    void addLogHandler(logFuncHandler fct, const std::string& name)
    {
      {
        boost::mutex::scoped_lock l(rtLogInstance.rtLogWriteLock);
        rtLogInstance.logHandlers[name] = fct;
      }
    }

    void removeLogHandler(const std::string& name)
    {
      {
        boost::mutex::scoped_lock l(rtLogInstance.rtLogWriteLock);
        rtLogInstance.logHandlers.erase(name);
      }
    }

    const LogLevel stringToLogLevel(const char* verb)
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
    };

    LogLevel getVerbosity()
    {
      return _glVerbosity;
    };

    void setContext(bool ctx)
    {
      _glContext = ctx;
    };

    bool getContext()
    {
      return _glContext;
    };
  } // namespace log
} // namespace qi

