/*
**
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2011 Cedric GESTES
*/


#include <boost/filesystem.hpp>
#include <locale>
#include <cstdio>
#include <cstdlib>
#include <algorithm>
#include <fcntl.h>
#include <sys/stat.h>

#ifdef _WIN32
# include <io.h>      //_wopen
# include <windows.h> //Sleep
#else
# include <pwd.h>
# include <sys/time.h>
#endif

#include <qi/os.hpp>
#include <qi/locale.hpp>
#include "src/filesystem.hpp"

namespace qi {
  namespace os {

    FILE* fopen(const char *filename, const char *mode) {
      return ::fopen(filename, mode);
    }

    int stat(const char *pFilename, struct ::stat* pStat) {
      return ::stat(pFilename, pStat);
    }

    std::string getenv(const char *var) {
      char *res = ::getenv(var);
      if (res == NULL)
        return "";
      return std::string(res);
    }

    int setenv(const char *var, const char *value) {
      return ::setenv(var, value, 1);
    }

    void sleep(unsigned int seconds) {
      // In case sleep was interrupted by a signal,
      // keep sleeping until we have slept the correct amount
      // of time.
      while (seconds != 0) {
        seconds = ::sleep(seconds);
      }
    }

    void msleep(unsigned int milliseconds) {
      // Not the same for usleep: it returns a non-zero
      // error code if it was interupted...
      usleep(milliseconds * 1000);
    }


    std::string home()
    {
      std::string envHome = qi::os::getenv("HOME");
      if (envHome != "")
      {
        return boost::filesystem::path(envHome, qi::unicodeFacet()).make_preferred().string(qi::unicodeFacet());
      }

      // $HOME environment variable not defined:
      char *lgn;
      struct passwd *pw;
      if ((lgn = getlogin()) == NULL || (pw = getpwnam(lgn)) == NULL)
      {
        return boost::filesystem::path(pw->pw_dir, qi::unicodeFacet()).make_preferred().string(qi::unicodeFacet());
      }
      // Give up:
      return "";
    }

    std::string tmp(const char *prefix) {
      std::string             cache = qi::os::getenv("XDG_CACHE_HOME");
      boost::filesystem::path path;
      if (cache.empty()) {
       #ifdef __APPLE__
        path = boost::filesystem::path(::qi::os::home(),
                                       qi::unicodeFacet()) / "Cache";
       #else
        path = boost::filesystem::path(::qi::os::home(),
                                       qi::unicodeFacet()) / ".cache";
       #endif
      } else
        path = cache;

      path /= prefix;
      if (!boost::filesystem::exists(path)) {
        boost::filesystem::create_directories(path);
      }
      return path.string(qi::unicodeFacet());
    }

    int gettimeofday(qi::os::timeval *tp) {
      struct ::timeval tv;
      int ret = ::gettimeofday(&tv, 0);
      tp->tv_sec = tv.tv_sec;
      tp->tv_usec = tv.tv_usec;
      return ret;
    }
  };
};
