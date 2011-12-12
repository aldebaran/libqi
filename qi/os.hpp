/**
 * Author(s):
 *  - Herve CUCHE <hcuche@aldebaran-robotics.com>
 *  - Cedric GESTES <gestes@aldebaran-robotics.com>
 *
 * Copyright (C) 2011 Aldebaran Robotics
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
