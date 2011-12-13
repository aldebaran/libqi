/*
** os_launch.cpp
** Login : <hcuche@hcuche-de>
** Started on  Mon Apr 11 10:07:22 2011 Herve Cuche
** $Id$
**
** Author(s):
**  - Herve Cuche <hcuche@aldebaran-robotics.com>
**  - David Coz <dcoz@aldebaran-robotics.com>
*/

#include <iostream>

#include <errno.h>
#include <fcntl.h>
#ifdef __linux__
# include <unistd.h> // for environ
#endif
#ifdef __APPLE__
// See man 3 environ (PROGRAMMING)
# include <crt_externs.h> // for _NSGetEnviron
#endif

#include <sys/wait.h>
#ifndef __ANDROID__
#  include <spawn.h>
#endif
#include <stdarg.h>

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
      boost::filesystem::path path("/proc", qi::unicodeFacet());
      path = path / pid / "fd";

      try
      {
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
            std::istringstream iss(itr->path().filename().string(qi::unicodeFacet()));
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
      }
      catch (...)
      {
        return 0;
      }

      return 0;
    }
#endif


    int spawnvp(char *const argv[])
    {
#ifdef __linux__
      // Set all parent FD to close them when exec
      setCloexecFlag(getpid());
#endif

      pid_t pID;
      int err;
      posix_spawnattr_t* pSpawnattr = NULL;

#ifdef __linux__
      posix_spawnattr_t spawnattr;
      spawnattr.__flags = POSIX_SPAWN_USEVFORK;
      pSpawnattr = &spawnattr;
#endif

      // Err != 0 means vfork failed.
      // If exec() fails, then err = 0 and we have to get the status of the child
      // process to know what happened.
      // Note: child process environment will be the same as parent process.
      // TODO: maybe we should have a way of setting child process env?
#ifdef __linux__
      char** child_env = environ;
#else
      char*** environ_ptr = _NSGetEnviron();
      char** child_env = *environ_ptr;
#endif
      err = posix_spawnp(&pID,
                         argv[0],
                         NULL,
                         pSpawnattr,
                         (char* const*)argv,
                         child_env);

      if (err == EINVAL || err == ENOENT)
      {
        return -1;
      }
      if (err != 0)
      {
        return -1;
      }

      return pID;
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

      pid_t pID;
      int err;

#ifdef __linux__
      // Set all parent FD to close them when exec
      setCloexecFlag(getpid());
#endif


      posix_spawnattr_t* pSpawnattr = NULL;

#ifdef __linux__
      posix_spawnattr_t spawnattr;
      spawnattr.__flags = POSIX_SPAWN_USEVFORK;
      pSpawnattr = &spawnattr;
#endif

      // Err != 0 means vfork failed.
      // If exec() fails, then err = 0 and we have to get the status of the child
      // process to know what happened.
      // Note: child process environment will be the same as parent process.
      // TODO: maybe we should have a way of setting child process env?
#ifdef __linux__
      char** child_env = environ;
#else
      char*** environ_ptr = _NSGetEnviron();
      char** child_env = *environ_ptr;
#endif
      err = posix_spawnp(&pID,
                         cmd[0],
                         NULL,
                         pSpawnattr,
                         (char* const*)cmd,
                         child_env);

      if (err == EINVAL || err == ENOENT)
      {
        return -1;
      }
      if (err != 0)
      {
        return -1;
      }

      return pID;
    }

    int system(const char *command)
    {
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
    }

    int getpid()
    {
      return ::getpid();
    }

    int waitpid(int pid, int* status)
    {
      int result = 0;
      int st = 0;
      errno = 0;

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
    }
  };
};
