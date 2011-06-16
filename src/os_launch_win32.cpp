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

#include <windows.h>
#include <process.h>

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

    int spawnvp(char *const argv[])
    {
      return _spawnvp(_P_NOWAIT, argv[0], (char* const*)argv);
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

      return _spawnvp(_P_NOWAIT, cmd[0], (char* const*)cmd);
    }

    int system(const char *command)
    {
      boost::filesystem::path fname(command, qi::utf8facet());
      return _wsystem(fname.wstring(qi::utf8facet()).c_str());
    }

    int waitpid(int pid, int* status)
    {
      int result = 0;
      errno = 0;

      _cwait(status, pid, 0);

      if (errno == ECHILD)
      {
        result = 0;
        *status = 127;
        return result;
      }

      return errno;
    }
  };
};
