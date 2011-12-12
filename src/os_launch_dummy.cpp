/*
** Author(s):
**  - Herve Cuche <hcuche@aldebaran-robotics.com>
**  - David Coz <dcoz@aldebaran-robotics.com>
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2011 Aldebaran Robotics
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
  };
};
