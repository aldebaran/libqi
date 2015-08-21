/*
 * Copyright (c) 2012, 2013 Aldebaran Robotics. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the COPYING file.
 */

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
#include <qi/path.hpp>

namespace qi
{
namespace os
{
  namespace
  {
    std::vector<const wchar_t*> makeCProxy(const std::vector<std::wstring>& wstrlist)
    {
      std::vector<const wchar_t*> cstrlist;
      cstrlist.reserve(wstrlist.size() + 1);
      for (const auto& wstr : wstrlist)
      {
        cstrlist.push_back(wstr.c_str());
      }
      cstrlist.push_back(nullptr);
      return cstrlist;
    }

    template <class WStringSequence>
    int winSpawn(const WStringSequence& wArgs)
    {
      const auto cwArgs = makeCProxy(wArgs);
      return static_cast<int>(_wspawnvp(_P_NOWAIT, cwArgs[0], cwArgs.data()));
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
    HANDLE handle = (HANDLE)pid;
    int res = -1;
    DWORD status;
    GetExitCodeProcess(handle, &status);
    if (status == STILL_ACTIVE)
    {
      if (sig == SIGTERM || sig == SIGKILL)
      {
        DWORD error = TerminateProcess(handle, 0);
        qi::os::msleep(100);
        GetExitCodeProcess(handle, &status);
        if (status != STILL_ACTIVE)
          res = 0;
      }
      else
      {
        res = 0;
      }
    }
    return res;
  }
}
}
