
/*
 * Copyright (c) 2012 Aldebaran Robotics. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the COPYING file.
 */

#include <boost/config.hpp>
#include <boost/function.hpp>
#include <qi/log/journaldloghandler.hpp>
#include <qi/log.hpp>
#include <qi/os.hpp>
// prevent sd_journal_send() from adding CODE_LINE, CODE_FILE, CODE_FUNC
// automatically, because any log would seem to come from
// journaldloghandler.cpp
#define SD_JOURNAL_SUPPRESS_LOCATION 1
#include <systemd/sd-journal.h>
#include <iostream>
#include <cstdio>

namespace qi
{
namespace log
{
  // systemd uses the log level defined by syslog.
  // Here is how we mix both.
  // LOG_EMERG(0)   LogLevel_Fatal(1)
  // LOG_ALERT(1)
  // LOG_CRIT(2)
  // LOG_ERR(3)     LogLevel_Error(2)
  // LOG_WARNING(4) LogLevel_Warning(3)
  // LOG_NOTICE(5)  LogLevel_Info(4)
  // LOG_INFO(6)    LogLevel_Verbose(5)
  // LOG_DEBUG(7)   LogLevel_Debug(6)
  int toSyslogPriority(const qi::LogLevel verb)
  {
    int prio = static_cast<int>(verb);
    if (prio == 1)
      return 0;
    return prio + 1;
  }

  bool isAssignmentValid(const std::string &field)
  {
    const auto pos = field.find('=');
    return (pos != 0) && (pos != std::string::npos);
  }

  class JournaldLogHandler
  {
  private:
    // because of #define FILE_SIZE 128 in log.cpp,
    // the optimal value for FILE_LEN is 127
    BOOST_STATIC_CONSTEXPR auto FILE_LEN = 127u;
    BOOST_STATIC_CONSTEXPR auto FILE_PREFIX_LEN = sizeof("CODE_FILE=") - 1;
    char _fileBuffer[FILE_PREFIX_LEN + FILE_LEN + 1u];
    char _lineBuffer[32u];
    // extraneous field to pass to journald, if not empty.
    std::string _extraField;
    const char *_extraFormat;
  public:

    // We'll use field with sd_journal_send as in:
    //
    // auto i = sd_journal_send(..., "%s", field.c_str(), nullptr);
    //
    // In this call, if field does not contain '=' or if it starts with '=',
    // then sd_journal_send fails (i != 0).
    // Otherwise, then the entry has the form of an
    // assignment (name=value), and the call succeeds (ie i == 0).
    // Yet, sd_journal_send might find the name invalid,
    // in which case the invalid assignment is skipped.
    // The other ones (MESSAGE, timestamps,...) are sent.
    //
    // Here are examples of inputs (tested on systemd 232):
    //
    //   ===================  =================  ==================
    //   field                i == 0             assignment skipped
    //   ===================  =================  ==================
    //   ""                   false
    //   "="                  false
    //   "=bar"               false
    //   "_=bar"              true               true
    //   " =bar"              true               true
    //   "_FOO=bar"           true               true
    //   "Foo=bar"            true               true
    //   "1_FOO=bar"          true               true
    //   "FOO=bar"            true               false
    //   "FOO="               true               false
    //   "FOO_1=charge=10%!"  true               false
    //   "FOO=%s"             true               false
    //   ===================  =================  ==================
    //
    // Note:
    //
    // the form
    //
    //   auto i = sd_journal_send(..., "%s", field.c_str(), nullptr);
    //
    // is prefered over
    //
    //   auto i = sd_journal_send(..., field.c_str(), nullptr);
    //
    // because with the latter, a field with value "HELL=%s" would make
    // the program crash.
    JournaldLogHandler(std::string field)
      : _fileBuffer{"CODE_FILE="}, // extra bytes are zero-initialized
        _lineBuffer{"CODE_LINE="}, // extra bytes are zero-initialized
        _extraField(isAssignmentValid(field) ? std::move(field) :  ""),
        _extraFormat(_extraField.empty() ? nullptr : "%s") {}

    /**
     * \brief Send logs messages to systemd
     * \param verb verbosity of the log message.
     * \param date qi::Clock date at which the log message was issued.
     * \param systemDate qi::SystemClock date at which the log message was issued.
     * \param category will be used in future for filtering
     * \param msg actual message to log.
     * \param file filename from which this log message was issued.
     * \param fct function name from which this log message was issued.
     * \param line line number in the issuer file.
     */
    void operator()(const qi::LogLevel verb,
                    const qi::Clock::time_point /* date */,
                    const qi::SystemClock::time_point /* systemDate */,
                    const char* category,
                    const char* msg,
                    const char* file,
                    const char* fct,
                    const int line)
    {

      QI_ASSERT(file != nullptr);
      const auto priority = toSyslogPriority(verb);
      const bool sendLocation =
#if defined(NO_QI_LOG_DETAILED_CONTEXT)
         false;
#else
        (*file != '\0');
#endif
      if (sendLocation)
      {
        // do not write to the last byte in file_buffer, to ensure there
        // is null terminating character if strlen(file) == FILE_LEN
        std::strncpy(_fileBuffer + FILE_PREFIX_LEN, file, FILE_LEN);
        std::snprintf(_lineBuffer, sizeof(_lineBuffer), "CODE_LINE=%i", line);
        int i = sd_journal_send_with_location(
                    _fileBuffer,
                    _lineBuffer,
                    fct,
                    "MESSAGE=%s", msg,
                    "QI_CATEGORY=%s", category,
                    "PRIORITY=%i", priority,
                    "QI=1",
                    _extraFormat,
                    _extraField.c_str(),
                    nullptr);
        if (i == 0)
          return;
      }
      else
      {
        int i = sd_journal_send("MESSAGE=%s", msg,
                                "QI_CATEGORY=%s", category,
                                "PRIORITY=%i", priority,
                                "QI=1",
                                _extraFormat,
                                _extraField.c_str(),
                                nullptr);
        if (i == 0)
          return;
      }
      // sd_journal_send failed, try a simpler call
      int j = sd_journal_print(priority, "%s; %s", category, msg);
      if (j == 0)
        return;
      // If it fails again print an error message
      std::cerr << "Failed to send message to journald." << std::endl;
    }
  };


  Handler makeJournaldLogHandler()
  {
    return JournaldLogHandler(qi::os::getenv("QI_LOG_EXTRA_JOURNALD_FIELD"));
  }
}
}
