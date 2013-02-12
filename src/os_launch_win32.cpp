/*
 * Copyright (c) 2012 Aldebaran Robotics. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the COPYING file.
 */

#include <iostream>

#include <errno.h>
#include <fcntl.h>

#include <windows.h>
#include <process.h>
#include <signal.h>

#include <sstream>
#include <string>

#include <boost/filesystem.hpp>
#include <locale>

#include <qi/os.hpp>
#include <qi/qi.hpp>
#include "filesystem.hpp"


namespace qi
{
  namespace os
  {

    int spawnvp(char *const argv[])
    {
      return static_cast<int>(_spawnvp(_P_NOWAIT, argv[0], (char* const*)argv));
    }

    int spawnlp(const char* argv, ...)
    {
      const char* cmd[64];

      va_list ap;
      const char* arg;

      int i = 0;
      va_start(ap, argv);
      for (arg = argv; arg != NULL; arg = va_arg(ap, const char*), ++i)
        cmd[i] = arg;

      va_end(ap);
      cmd[i] = NULL;

      return static_cast<int>(_spawnvp(_P_NOWAIT, cmd[0], (char* const*)cmd));
    }

    int system(const char *command)
    {
      boost::filesystem::path fname(command, qi::unicodeFacet());
      return _wsystem(fname.wstring(qi::unicodeFacet()).c_str());
    }

    int getpid()
    {
      return _getpid();
    }

    int gettid()
    {
      return GetCurrentThreadId();
    }

    int waitpid(int pid, int* status)
    {
      errno = 0;

      _cwait(status, pid, 0);

      if (errno == ECHILD)
      {
        *status = 127;
        return 0;
      }

      return errno;
    }

    int kill(int pid, int sig)
    {
      HANDLE handle = (HANDLE) pid;
      int res = -1;
      DWORD status;
      GetExitCodeProcess(handle, &status);
      if(status == STILL_ACTIVE)
      {
        if(sig == SIGTERM)
        {
          DWORD error = TerminateProcess(handle, 0);
          qi::os::msleep(100);
          GetExitCodeProcess(handle, &status);
          if(status != STILL_ACTIVE)
            res = 0;
        }
        else
        {
          res = 0;
        }
      }
      return res;
    }
  };
};
