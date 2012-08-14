/*
 * Copyright (c) 2012 Aldebaran Robotics. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the COPYING file.
 */

#include <sys/syscall.h>
#include <sys/wait.h>
#include <errno.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <cassert>
#include <qi/os.hpp>

namespace qi
{
  namespace os
  {

    int spawnvp(char *const argv[])
    {
      assert(0 && "qi::os::spawnvp is not implemented for android");
      return -1;
    }

    int spawnlp(const char* argv, ...)
    {
      assert(0 && "qi::os::spawnlp is not implemented for android");
      return -1;
    }

    int system(const char *command)
    {
      return system(command);
    }

    int getpid()
    {
      return ::getpid();
    }

    int gettid() {
      return syscall(__NR_gettid);
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
      return result;
    }

    int kill(int pid, int sig) {
        return ::kill(pid, sig);
    }

  }
}
