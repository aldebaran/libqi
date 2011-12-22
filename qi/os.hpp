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

/*
 * Various cross-platform OS related functions
 *
 * \note Every path taken in parameter MUST be encoded in UTF8.
 * \note Every path returned are encoded in UTF8.
 */

#pragma once
#ifndef _LIBQI_QI_OS_HPP_
#define _LIBQI_QI_OS_HPP_

# include <string>
# include <qi/config.hpp>

struct stat;

namespace qi {

  namespace os {

    QI_API FILE* fopen(const char *filename, const char *mode);
    QI_API int stat(const char *filename, struct stat *pstat);
    QI_API int checkdbg();
    QI_API std::string home();
    QI_API std::string tmpdir(const char *prefix = "");

    // env
    QI_API std::string getenv(const char *var);
    QI_API int setenv(const char *var, const char *value);

    // time
    QI_API void sleep(unsigned int seconds);
    QI_API void msleep(unsigned int milliseconds);
    struct QI_API timeval {
      long tv_sec;
      long tv_usec;
    };
    QI_API int gettimeofday(qi::os::timeval *tp);

    // shared library
    QI_API void *dlopen(const char *filename, int flag = -1);
    QI_API int dlclose(void *handle);
    QI_API void *dlsym(void *handle, const char *symbol);
    QI_API const char *dlerror(void);

    // process management
    QI_API int spawnvp(char *const argv[]);
    QI_API int spawnlp(const char* argv, ...);
    QI_API int system(const char *command);
    QI_API int getpid();
    QI_API int waitpid(int pid, int* status);

  };
};


#endif  // _LIBQI_QI_OS_HPP_
