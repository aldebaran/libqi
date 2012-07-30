/*
 * Copyright (c) 2012 Aldebaran Robotics. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the COPYING file.
 */

#include <iostream>

#include <errno.h>
#include <fcntl.h>

#include <sys/wait.h>
#include <stdarg.h>

#include <sstream>
#include <string>

#include <boost/filesystem.hpp>
#include <locale>

#include <qi/os.hpp>
#include <qi/qi.hpp>
#include "src/filesystem.hpp"

//posix implementation
namespace qi
{
  namespace os
  {

    int spawnvp(char *const argv[])
    {
      return -1;
    }

    int spawnlp(const char* argv, ...)
    {
      return -1;
    }

    int system(const char *command)
    {
      return -1;
    }

    int getpid()
    {
      return -1;
    }

    int waitpid(int pid, int* status)
    {
      return -1;
    }

    int kill(int, int) {
        return -1;
    }

  };
};
