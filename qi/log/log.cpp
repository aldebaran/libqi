/*
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2010 Aldebaran Robotics
*/

#include <qi/log/log.hpp>

#include <sstream>
#include <iostream>
#include <cstdarg>
# include <cstdio>
# include <cstring>

namespace qi
{
  static LogFunctionPtr gMyLog = 0;

  static void basicLogFunction(const Int8       *file,
                               const Int8       *fct,
                               const sInt32     line,
                               const sInt8      verb,
                               const Int8       *fmt, ...)
  {
     va_list     vl;
     int         len;
     const char *b = file;

     va_start(vl, fmt);
     len = strlen(file);
     if (len > 20)
       b = file + (len - 20);
     printf("%.20s:%s:%d:(%s): ", b, fct, line, cat);
     vprintf(fmt, vl);
     va_end(vl);
  }

  class LogFunctionInit {
    LogFunctionSetter() {
      setLogFunction(basicLogFunction);
    }
  } gLogFunctionInit;


  void setLogFunction(LogFunctionPtr p)
  {
    gMyLog = p;
  }

  void doLog(const Int8       *file,
             const Int8       *fct,
             const sInt32     line,
             const Int8       *cat,
             const sInt8      verb,
             const Int8       *fmt, ...)
  {
    if (gMyLog) {
      va_list   vl;
      va_start(vl, fmt);
      gMyLog(file, fct, line, cat, verb, fmt, vl);
      va_end(vl);
    }
    else {
      va_list   vl;
      va_start(vl, fmt);
      printf("[MISSING Logger]: ");
      vprintf(fmt, vl);
      va_end(vl);
    }
    #endif
  }
}


namespace AL
{
  static Log *gLogger = NULL;

  ALLog * getLogger(void) {
    if (gLogger == NULL) {
      gLogger = new Log();
    }
    return gLogger;
  }

  void ALLog::setLogLevel(const LogLevel& level) {
    fLogLevel = level;
  }

  void ALLog::setFilter(const std::string& pFilter) {
    fFilter = pFilter;
  }

  void Log::connect(void fonc(const std::string&)) {
    fPtrFonction = fonc;
  }

  void ALLog::log(const LogLevel& level,
                  const std::string& log,
                  const std::string &file,
                  const std::string &function,
                  int line)
  {
    if (level > fLogLevel) {
      return;
    }
    if ( fFilter != "" && (log.find(fFilter) == std::string::npos )) {
      return;
    }

    std::stringstream ss;
    switch(level) {
      case t_info:
        ss << "INF: ";
        break;
      case t_debug:
        ss << "DBG: ";
        break;
      case t_warning:
        ss << "WRN: ";
        break;
      case t_error:
        ss << "ERR: ";
        break;
      case t_fatal:
        ss << "FAT: ";
        break;
      default:
        ss << "INF: ";
        break;
     }

    ss << log << std::endl;

    if (fLogLevel == t_debug)
      ss << "     File: " << file << std::endl << "     Line: " << line << std::endl << "     Function: " << function << std::endl;
    process(ss.str());
  }

  void ALLog::process(const std::string &pStrLog)
  {
    if (fPtrFonction == NULL)
      std::cout << pStrLog;
    else
      fPtrFonction(pStrLog);
  }

  ALLog::ALLog()
  {
    fPtrFonction = NULL;
    fFocus = "";
    fFilter = "";
    fLogLevel = t_info;
    if ((ALSystem::getEnv("VERBOSE")!="") && (ALSystem::getEnv( "VERBOSE" ) != "0"))
    {
      fLogLevel = t_debug;
    }
  }


  ALStreamLog::ALStreamLog(
      const LogLevel& pLevel,
      const std::string& pFile,
      const std::string &pFunction,
      int pLine,
      bool pLogEntry)
    {
      fLogLevel  = pLevel;
      fFile      = pFile;
      fFunction  = pFunction;
      fLine      = pLine;
      fScopedLog = pLogEntry;
      if (pLogEntry)
      {
          getLogger()->log(AL::t_debug, std::string("Enter function ") + std::string(pFunction), fFile, fFunction, fLine);
      }
    }

  ALStreamLog::~ALStreamLog()
    {
      if (!fScopedLog)
        getLogger()->log(fLogLevel, this->str(), fFile, fFunction, fLine);
      else
        getLogger()->log(t_debug, std::string("     Exit function ") + std::string(fFunction), "", fFunction, fLine);
    }
}
