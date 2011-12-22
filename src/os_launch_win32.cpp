/*
 * Copyright (c) 2011, Aldebaran Robotics
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the Aldebaran Robotics nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL Aldebaran Robotics BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
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
#include <qi/qi.hpp>
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

      _cwait(status, pid, 0);

      if (errno == ECHILD)
      {
        *status = 127;
        return 0;
      }

      return errno;
    }
  };
};
