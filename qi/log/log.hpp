/*
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2010 Aldebaran Robotics
*/

#ifndef QI_LOG_LOG_H
#define QI_LOG_LOG_H

#include <string>
#include <sstream>
#include <iostream>

// :TODO: use __attribute__((visibility("hidden"))) on compatible platforms
#ifdef QI_EXPORTS
# ifdef _WIN32
#   define QILOGAPI __declspec(dllexport)
# else
#   define QILOGAPI
# endif
#else
# ifdef _WIN32
#   define QILOGAPI __declspec(dllimport)
# else
#   define QILOGAPI
# endif
#endif

// log macro
#define qiInfo(x)    doLog(AL::info,    x, __FILE__, __FUNCTION__, __LINE__)
#define qiDebug(x)   doLog(AL::debug,   x, __FILE__, __FUNCTION__, __LINE__)
#define qiWarning(x) doLog(AL::warning, x, __FILE__, __FUNCTION__, __LINE__)
#define qiError(x)   doLog(AL::error,   x, __FILE__, __FUNCTION__, __LINE__)
#define qiFatal(x)   doLog(AL::fatal,   x, __FILE__, __FUNCTION__, __LINE__)

#define qisInfo      qi::LogStream(AL::info,    __FILE__,__FUNCTION__, __LINE__).self()
#define qisDebug     qi::LogStream(AL::debug,   __FILE__,__FUNCTION__, __LINE__).self()
#define qisWarning   qi::LogStream(AL::warning, __FILE__,__FUNCTION__, __LINE__).self()
#define qisError     qi::LogStream(AL::error,   __FILE__,__FUNCTION__, __LINE__).self()
#define qisFatal     qi::LogStream(AL::fatal,   __FILE__,__FUNCTION__, __LINE__).self()

namespace qi
{

  /**
   * Enum of accepted error types
   */
  enum LogLevel
  {
    silent = 0,
    fatal,
    error,
    warning,
    info,
    debug,
  };


  /*
   * log ptr
   */
  typedef void (*LogFunctionPtr)(const char         *file,
                                 const char         *fct,
                                 const unsigned int  line,
                                 const sInt8         verb,
                                 const Int8         *fmt,
                                 va_list             va); //I think va_list could be replaced by char*

  /**
   * the log function
   */
  void doLog(const Int8       *file,
             const Int8       *fct,
             const sInt32     line,
             const sInt8      verb,
             const Int8       *fmt, ...);


  /**
   * set the function called when we need to log something
   * the log_ptr is plateform dependent.
   */
  void setLogFunction(LogFunctionPtr p);


  LIBSYSLOGAPI ALLog* getLogger(void);


  class QILOGAPI LogStream: public std::stringstream
  {
  public:

    /**
     * LogStream. Will log at object destruction
     * @param pLevel { t_debug = 5, t_info = 4, t_warning = 3, t_error = 2, t_fatal = 1, t_silent = 0 }
     * @param pFile __FILE__
     * @param pFunction __FUNCTION__
     * @param pLine __LINE__
     * @param pLogEntry log stream constructor and destructor (for scoped log)
    */
    LogStream(const LogLevel    &pLevel,
              const std::string &pFile,
              const std::string &pFunction,
              const int          pLine,
              bool               pLogEntry=false);

    ~ALStreamLog();

    // necessary to have sinfo et al. work with an anonymous object
    ALStreamLog& self() { return *this; }

  private:
    LogLevel    fLogLevel;
    std::string fFile;
    std::string fFunction;
    int         fLine;
    bool        fScopedLog;
  };

}


#endif

