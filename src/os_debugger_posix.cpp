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

#include <qi/os.hpp>

#include <cstdio>
#include <cstdlib>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ptrace.h>
#include <sys/wait.h>

#ifdef __APPLE__
# define MY_PTRACE_ATTACH PT_ATTACH
# define MY_PTRACE_CONT   PT_CONTINUE
# define MY_PTRACE_DETACH PT_DETACH
#else
# define MY_PTRACE_ATTACH PTRACE_ATTACH
# define MY_PTRACE_CONT   PTRACE_CONT
# define MY_PTRACE_DETACH PTRACE_DETACH
#endif

namespace qi {
  namespace os {

    int checkdbg()
    {
      int pid = fork();
      int status;
      int res;

      if (pid == -1) {
        perror("fork");
        return -1;
      }

      if (pid == 0) {
        int ppid = getppid();

        /* Child */
        if (ptrace(MY_PTRACE_ATTACH, ppid, NULL, NULL) == 0)
        {
          /* Wait for the parent to stop and continue it */
          ::waitpid(ppid, NULL, 0);
          ptrace(MY_PTRACE_CONT, NULL, NULL, NULL);

          /* Detach */
          ptrace(MY_PTRACE_DETACH, getppid(), NULL, NULL);

          /* We were the tracers, so gdb is not present */
          res = 0;
        }
        else
        {
          /* Trace failed so gdb is present */
          res = 1;
        }
        _exit(res);
      }
      else
      {
        ::waitpid(pid, &status, 0);
        res = WEXITSTATUS(status);
      }
      return res;
    }

  }
};
