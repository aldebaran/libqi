/*
** os_launch.cpp
** Login : <hcuche@hcuche-de>
** Started on  Mon Apr 11 10:07:22 2011 Herve Cuche
** $Id$
**
** Author(s):
**  - Herve Cuche <hcuche@aldebaran-robotics.com>
**  - David Coz <dcoz@aldebaran-robotics.com>
**
** Copyright (C) 2011 Herve Cuche
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 3 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program; if not, write to the Free Software
** Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
*/

#include <iostream>

#include <errno.h>
#include <fcntl.h>

#if _WIN32
# include <windows.h>
# include <process.h>
#else
# include <sys/wait.h>
# ifndef __ANDROID__
#   include <spawn.h>
# endif
# include <stdarg.h>
#endif

#include <sstream>
#include <string>

#include <boost/filesystem.hpp>
#include <locale>

#include <qi/os.hpp>
#include <qi/locale.hpp>
#include "src/filesystem.hpp"


namespace qi
{
  namespace os
  {

   #ifdef __linux__
    /**
     * Set cloexec flag to close all FD on exec process
     * @param pID pid of the process.
     * @return 0
     */
    static int setCloexecFlag(pid_t pID)
    {
      std::string pid;
      std::stringstream ss;
      ss << pID;
      pid = ss.str();

      // get path of the current pid
      boost::filesystem::path path("/proc", qi::utf8facet());
      path = path / pid / "fd";

      // foreach fd set FD_CLOEXEC flag
      for (boost::filesystem::directory_iterator itr(path);
           itr != boost::filesystem::directory_iterator();
           ++itr)
      {
        // do not close stdin, stdout, stderr
        if (itr->path().filename() != "0"
            && itr->path().filename() != "1"
            && itr->path().filename() != "2")
        {
          // get file descriptor
          int fd;
          std::istringstream iss(itr->path().filename().string(qi::utf8facet()));
          iss >> fd;

          // get flags
          int oldflags = fcntl(fd, F_GETFD, 0);

          // If reading the flags failed, return error indication now.
          if (oldflags < 0)
            return oldflags;

          // Set just the flag we want to set.
          oldflags |= FD_CLOEXEC;

          // Store modified flag word in the descriptor.
          return fcntl(fd, F_SETFD, oldflags);
        }
      }
      return 0;
    }
   #endif


    int spawnvp(char *const argv[])
    {
     #ifdef _WIN32
      return _spawnvp(_P_NOWAIT, argv[0], (char* const*)argv);
     #else
     #ifdef __linux__
      // Set all parent FD to close them when exec
      setCloexecFlag(getpid());
     #endif

      pid_t pID;
      int err;
      posix_spawnattr_t* pSpawnattr = NULL;

     #ifndef __APPLE__
      posix_spawnattr_t spawnattr;
      spawnattr.__flags = POSIX_SPAWN_USEVFORK;
      pSpawnattr = &spawnattr;
     #endif

      // Err != 0 means vfork failed.
      // If exec() fails, then err = 0 and we have to get the status of the child
      // process to know what happened.
      err = posix_spawnp(&pID,
                         argv[0],
                         NULL,
                         pSpawnattr,
                         (char* const*)argv,
                         NULL);

      if (err == EINVAL || err == ENOENT)
      {
        return -1;
      }
      if (err != 0)
      {
        return -1;
      }

      return pID;
     #endif
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

     #ifdef _WIN32
      return _spawnvp(_P_NOWAIT, cmd[0], (char* const*)cmd);
     #else
      pid_t pID;
      int err;

     #ifdef __linux__
      // Set all parent FD to close them when exec
      setCloexecFlag(getpid());
     #endif


      posix_spawnattr_t* pSpawnattr = NULL;

     #ifndef __APPLE__
      posix_spawnattr_t spawnattr;
      spawnattr.__flags = POSIX_SPAWN_USEVFORK;
      pSpawnattr = &spawnattr;
     #endif

      // Err != 0 means vfork failed.
      // If exec() fails, then err = 0 and we have to get the status of the child
      // process to know what happened.
      err = posix_spawnp(&pID,
                         cmd[0],
                         NULL,
                         pSpawnattr,
                         (char* const*)cmd,
                         NULL);

      if (err == EINVAL || err == ENOENT)
      {
        return -1;
      }
      if (err != 0)
      {
        return -1;
      }

      return pID;
     #endif
    }

    int system(const char *command)
    {
     #ifdef _WIN32
      boost::filesystem::path fname(command, qi::utf8facet());

      return _wsystem(fname.wstring(qi::utf8facet()).c_str());
     #else
     #ifdef __linux__
      // Set all parent FD to close them when exec
      setCloexecFlag(getpid());
     #endif

      int status = 0;
      pid_t pID = vfork();

      if (pID < 0)
      {
        return -1;
      }
      if (pID == 0)
      {
        // In the child process.

        // WARNING: critical code!
        // The entire process is stuck here until we call exec() or _exit.
        // Never access variables from the parent process, call another funciton, etc.
        // Just call exec right away.
        execlp("/bin/sh", "/bin/sh", "-c", command, NULL);

        // In case exec fails, we kill the child process.
        _exit(errno);
      }
      else
      {
        // Parent process.
        qi::os::waitpid(pID, &status);
        return status;
      }

      return -1;
     #endif
    }

    int waitpid(int pid, int* status)
    {
      int result = 0;
      errno = 0;

     #ifdef _WIN32
      _cwait(status, pid, 0);

      if (errno == ECHILD)
      {
        result = 0;
        *status = 127;
        return result;
      }

      return errno;
     #else
      int st = 0;

      ::waitpid(pid, &st, 0);

      if (WIFSIGNALED(st))
      {
        result -= WTERMSIG(st);
        status = NULL;
      }
      else
      {
        result = errno;
        *status = WEXITSTATUS(st);
      }

     #ifdef __APPLE__
      if (errno == ECHILD)
      {
        result = 0;
        *status = 127;
        return result;
      }
     #endif

      return result;
     #endif
    }
  };
};
