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
#include "src/filesystem.hpp"


namespace qi
{
  namespace os
  {
    int spawnvp(char *const argv[])
    {
      std::string tmp;
      int i =0;
      char* test = argv[i];
      while(test != NULL)
      {
        tmp.append(argv[i]);
        i++;
        test = argv[i];
        if(test != NULL)
          tmp.push_back(' ');
      }

      LPTSTR cmdLine = (LPTSTR) malloc(tmp.size() + 1);
      strcpy(cmdLine, tmp.c_str());

      STARTUPINFO info = {sizeof(info)};
      PROCESS_INFORMATION processInfo;
      CreateProcess(
        NULL,
        cmdLine,
        NULL,
        NULL,
        false,
        0,
        NULL,
        NULL,
        &info,
        &processInfo);

      free(cmdLine);

      return processInfo.dwProcessId;
    }

    int spawnlp(const char* argv, ...)
    {
      std::string tmp;
      va_list ap;
      const char* arg;

      va_start(ap, argv);
      for(arg = argv ; arg != NULL ; arg = va_arg(ap, const char*))
      {
        tmp.append(arg);
        tmp.push_back(' ');
      }
      va_end(ap);

      LPTSTR cmdLine = (LPTSTR) malloc(tmp.size() + 1);
      strcpy(cmdLine, tmp.c_str());

      STARTUPINFO info = {sizeof(info)};
      PROCESS_INFORMATION processInfo;
      CreateProcess(
        NULL,
        cmdLine,
        NULL,
        NULL,
        false,
        0,
        NULL,
        NULL,
        &info,
        &processInfo);

      free(cmdLine);

      return processInfo.dwProcessId;
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

    int waitpid(int pid, int* status)
    {
      errno = 0;

      HANDLE processHandle = OpenProcess(
        PROCESS_QUERY_INFORMATION | SYNCHRONIZE,
        false,
        pid);
      WaitForSingleObjectEx(processHandle, INFINITE, true);
      DWORD wStatus = 0;
      qi::os::msleep(100);
      GetExitCodeProcess(processHandle, &wStatus);

      *status = wStatus;

      CloseHandle(processHandle);
      if (errno == ECHILD || processHandle == NULL)
      {
        *status = 127;
        return 0;
      }

      return errno;
    }

    int kill(int pid, int sig)
    {
      int res = -1;
      HANDLE processHandle = OpenProcess(
        PROCESS_TERMINATE | PROCESS_QUERY_INFORMATION,
        false,
        pid);
      if(processHandle != NULL)
      {
        DWORD status;
        GetExitCodeProcess(processHandle, &status);
        if(status == STILL_ACTIVE)
        {
          if(sig == SIGTERM)
          {
            DWORD error = TerminateProcess(processHandle, 0);
            qi::os::msleep(100);
            GetExitCodeProcess(processHandle, &status);
            if(status != STILL_ACTIVE)
              res = 0;
          }
          else
          {
            res = 0;
          }
        }
        else
        {
          res = -1;
        }
        CloseHandle(processHandle);
        return res;
      }
      else
      {
        return -1;
      }
    }
  };
};
