
/*
 * Copyright (c) 2012 Aldebaran Robotics. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the COPYING file.
 */

#include <qi/log/journaldloghandler.hpp>

#include <qi/log.hpp>
#include <systemd/sd-journal.h>
#include <iostream>

namespace qi
{
namespace log
{
  void JournaldLogHandler::log(const qi::LogLevel verb,
                               const char* category,
                               const char* msg,
                               const char* file,
                               const char* fct,
                               const int line)
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
    int _verb = static_cast<int>(verb);
    if (_verb == 1) _verb = 0;
    else _verb += 1;

    int i = sd_journal_send("MESSAGE=%s",     msg,
                            "QI_CATEGORY=%s", category,
                            "PRIORITY=%i",    _verb,
                            "CODE_FILE=%s",   file,
                            "CODE_LINE=%i",   line,
                            "CODE_FUNC=%s",   fct,
                            NULL
                           );
    if (i == 0)
      return;

    // If it fail try to do a simpler call to journald
    int j = sd_journal_print(_verb, "%s; %s", category, msg);
    if (j == 0)
      return;

    // If it fail again print an error message
    std::cerr << "Can't send message to journald." << std::endl;
  }
}
}
