
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
    #ifdef _WIN32
      try {
        boost::filesystem::path dest(filename);
        std::string  amode(mode);
        std::wstring wmode;
        std::copy(amode.begin(), amode.end(), std::back_inserter(wmode));
        return ::_wfopen(dest.wstring(qi::utf8facet()).c_str(), wmode.c_str());
      }
      catch (boost::filesystem::filesystem_error &) {
        return 0;
      }

    #else
      return ::fopen(filename, mode);
    #endif
    }

    int stat(const char *pFilename, struct ::stat* pStat) {
    #ifdef _WIN32
      try {
        struct _stat buffer;
        int result = ::_wstat(boost::filesystem::path(pFilename, qi::utf8facet()).wstring(qi::utf8facet()).c_str(), &buffer);
        pStat->st_gid   = buffer.st_gid;
        pStat->st_atime = buffer.st_atime;
        pStat->st_ctime = buffer.st_ctime;
        pStat->st_dev   = buffer.st_dev;
        pStat->st_ino   = buffer.st_ino;
        pStat->st_mode  = buffer.st_mode;
        pStat->st_mtime = buffer.st_mtime;
        pStat->st_nlink = buffer.st_nlink;
        pStat->st_rdev  = buffer.st_rdev;
        pStat->st_size  = buffer.st_size;
        pStat->st_uid   = buffer.st_uid;
        return result;
      }
      catch (boost::filesystem::filesystem_error &) {
        return -1;
      }
    #else
      return ::stat(pFilename, pStat);
    #endif
    }

    std::string getenv(const char *var) {
      #ifdef _MSC_VER
      char        *envDir = NULL;
      std::string  result;
      size_t       bufSize;
      _dupenv_s(&envDir, &bufSize, var);
      if (envDir == NULL)
        return "";
      std::string ret(envDir);
      free(envDir);
      return ret;
      #else
      char *res = ::getenv(var);
      if (res == NULL)
        return "";
      return std::string(res);
      #endif
    }

    int setenv(const char *var, const char *value) {
      #ifdef _WIN32
      std::string env = std::string(var) + std::string("=") + std::string(value);
      return _putenv(env.c_str());
      #else
      return ::setenv(var, value, 1);
      #endif
    }

    void sleep(unsigned int seconds) {
      #ifdef _WIN32
      Sleep(seconds * 1000);
      #else
      // In case sleep was interrupted by a signal,
      // keep sleeping until we have slept the correct amount
      // of time.
      while (seconds != 0) {
        seconds = ::sleep(seconds);
      }
      #endif
    }

    void msleep(unsigned int milliseconds) {
      #ifdef _WIN32
      Sleep(milliseconds);
      #else
      // Not the same for usleep: it returns a non-zero
      // error code if it was interupted...
      usleep(milliseconds * 1000);
      #endif
    }


    std::string home()
    {
      std::string envHome = qi::os::getenv("HOME");
      if (envHome != "")
      {
        return boost::filesystem::path(envHome, qi::utf8facet()).make_preferred().string(qi::utf8facet());
      }

      // $HOME environment variable not defined:
#   if defined(WIN32)
      std::string envUserProfile = qi::os::getenv("USERPROFILE");
      if (envUserProfile != "")
      {
        return envUserProfile;
      }

      std::string envHomeDrive = qi::os::getenv("HOMEDRIVE");
      std::string envHomePath  = qi::os::getenv("HOMEPATH");

      if (envHomePath == "" || envHomeDrive == "")
      {
        // Give up:
        return "";
      }
      boost::filesystem::path res(envHomeDrive, qi::utf8facet());
      res /= envHomePath;
      return res.make_preferred().string(qi::utf8facet());
#   else

      char *lgn;
      struct passwd *pw;
      if ((lgn = getlogin()) == NULL || (pw = getpwnam(lgn)) == NULL)
      {
        return boost::filesystem::path(pw->pw_dir, qi::utf8facet()).make_preferred().string(qi::utf8facet());
      }
      // Give up:
      return "";
#   endif
    }

    std::string tmp(const char *prefix) {
      std::string             cache = qi::os::getenv("XDG_CACHE_HOME");
      boost::filesystem::path path;
      if (cache.empty()) {
#   if defined(WIN32)
        path = qi::os::getenv("TMP");
#   elif defined(__APPLE__)
        path = boost::filesystem::path(::qi::os::home(), qi::utf8facet()) / "Cache";
#   else
        path = boost::filesystem::path(::qi::os::home(), qi::utf8facet()) / ".cache";
#   endif
      } else
        path = cache;
      path /= prefix;
      if (!boost::filesystem::exists(path)) {
        boost::filesystem::create_directories(path);
      }
      return path.string(qi::utf8facet());
    }



    #ifndef _WIN32
    int gettimeofday(qi::os::timeval *tp, void *tzp) {
      struct ::timeval tv;
      int ret = ::gettimeofday(&tv, (struct timezone *)tzp);
      tp->tv_sec = tv.tv_sec;
      tp->tv_usec = tv.tv_usec;
      return ret;
    }
    #endif


  };
};
