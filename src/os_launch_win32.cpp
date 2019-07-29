/*
 * Copyright (c) 2012, 2013 Aldebaran Robotics. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the COPYING file.
 */

#include <errno.h>
#include <fcntl.h>


#include <process.h>
#include <signal.h>

#include <sstream>
#include <string>

#include <boost/filesystem.hpp>
#include <locale>

#include <qi/assert.hpp>
#include <qi/log.hpp>
#include <qi/os.hpp>
#include <qi/path.hpp>

#include "os_win32.hpp"

qiLogCategory("qi.os");

namespace qi
{
namespace os
{
  namespace
  {
    std::vector<wchar_t> toCWStrings(const std::vector<std::wstring>& wstrlist)
    {
      std::vector<wchar_t> cstrlist;
      cstrlist.reserve(wstrlist.size() + 1);
      for(const auto& wstr : wstrlist)
      {
        for(const auto& wchr : wstr)
        {
          cstrlist.push_back(wchr);
        }
        cstrlist.push_back(L' ');
      }
      cstrlist.push_back(0);
      return cstrlist;
    }

    template <class WStringSequence>
    int winSpawn(const WStringSequence& wArgs)
    {
      auto cwArgs = toCWStrings(wArgs);
      STARTUPINFOW startupInfo { };
      PROCESS_INFORMATION processInfo { };
      const BOOL spawned = CreateProcessW(
            NULL, &cwArgs[0], NULL, NULL, false, NULL, NULL, NULL,
            &startupInfo, &processInfo);
      if(!spawned)
        return -1;
      return static_cast<int>(processInfo.dwProcessId);
    }

  }

  int spawnvp(char* const argv[])
  {
    std::vector<std::wstring> wargs;
    for (const char* const* argIt = &argv[0]; *argIt != NULL; ++argIt)
    {
      wargs.push_back(qi::Path(*argIt).bfsPath().wstring());
    }

    return winSpawn(wargs);
  }

  int spawnlp(const char* argv, ...)
  {
    std::vector<std::wstring> wargs;

    {
      va_list ap;
      const char* arg = nullptr;
      int i = 0;
      va_start(ap, argv);
      for (arg = argv; arg != NULL; arg = va_arg(ap, const char*), ++i)
      {
        wargs.push_back(qi::Path(arg).bfsPath().wstring());
      }
      va_end(ap);
    }

    return winSpawn(wargs);
  }

  int system(const char* command)
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
    auto logLastError = [&pid](const std::string& doingWhat)
    {
      const auto errorMessage = lastErrorMessage();
      qiLogDebug() << "Error waiting for pid " << pid << " "
                   << doingWhat << ": " << errorMessage;
    };

    const HANDLE handle = OpenProcess(
          PROCESS_QUERY_INFORMATION | PROCESS_VM_READ | SYNCHRONIZE, FALSE,
          static_cast<DWORD>(pid));
    if (!handle)
    {
      *status = 127;
      logLastError("checking the process");
      return 0;
    }

    const DWORD result = WaitForSingleObject(handle, INFINITE);
    QI_ASSERT(result != WAIT_TIMEOUT);
    if (result == WAIT_FAILED)
    {
      *status = 127;
      logLastError("waiting for the process");
      return 0;
    }

    DWORD exitCode = 0xFFFFFF;
    if (!GetExitCodeProcess(handle, &exitCode))
    {
      *status = 127;
      logLastError("retrieving the exit code");
      return 0;
    }

    *status = exitCode;
    return 0;
  }

  int kill(int pid, int sig)
  {
    if(sig == 0) // if signal is 0, just check that it is running
      return isProcessRunning(pid) ? 0 : -1;

    qiLogDebug() << "Killing " << pid << ": checking the process";
    const HANDLE handle = OpenProcess(
          PROCESS_QUERY_INFORMATION | PROCESS_VM_READ | PROCESS_TERMINATE, FALSE,
          static_cast<DWORD>(pid));
    if (!handle)
      return -1;

    qiLogDebug() << "Killing " << pid << ": terminating the process";
    if (!TerminateProcess(handle, sig))
      return -1;

    qiLogDebug() << "Killing " << pid << ": waiting the end of the process";
    WaitForSingleObject(handle, INFINITE);
    qiLogDebug() << "Killed " << pid;
    return 0;
  }
} // os
} // qi
