/*
**
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2011 Aldebaran Robotics
*/


#include <boost/filesystem.hpp>
#include <locale>
#include <cstdio>
#include <cstdlib>
#include <algorithm>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/timeb.h>

#include <io.h>       //_wopen
#include <windows.h>  //Sleep
#include <winsock2.h>

#include <qi/os.hpp>
#include <qi/qi.hpp>
#include "src/filesystem.hpp"


namespace qi {
  namespace os {

    // A global time provider is needed, as the performance counter
    // is relative to system start time, which we get only once
    struct TimeStorage {
      bool                   usePerformanceCounter;
      qi::os::timeval        systemStartTime;
      LARGE_INTEGER          systemStartTicks;
      LARGE_INTEGER          systemTicksPerSecond;
    };

    static TimeStorage *gTimeStorage;

    static void init_timer()
    {
      if (gTimeStorage)
        return;
      gTimeStorage = new TimeStorage;

      // get the system clock frequency
      // in theory this never changes, but advanced
      // power saving modes on laptops may change this
      // TODO investigate if this is a problem for us
      int success = QueryPerformanceFrequency( &gTimeStorage->systemTicksPerSecond );
      gTimeStorage->usePerformanceCounter = false;
      // See http://www.nynaeve.net/?p=108 : colorburst crystal: good. Everything else: bad.
      if (success/* && (systemTicksPerSecond.QuadPart == 1193182 || systemTicksPerSecond.QuadPart == 3579545)*/)
      {
        gTimeStorage->usePerformanceCounter = true;

        // Get the system ticks at startup (test using CPU affinity)
        // Thread affinity coped from an MS example
        DWORD_PTR oldmask = SetThreadAffinityMask(GetCurrentThread(), 0);
        QueryPerformanceCounter(&gTimeStorage->systemStartTicks);
        SetThreadAffinityMask(GetCurrentThread(), oldmask);
      }

      // get the start time
      struct _timeb timebuffer;
      _ftime64_s( &timebuffer );

      // store this in a timeval struct
      gTimeStorage->systemStartTime.tv_sec  = (long)timebuffer.time;
      gTimeStorage->systemStartTime.tv_usec = 1000*timebuffer.millitm;
    }

    /**
     * Special Hack for windows using performance counter
     * to give accurate timing, otherwise the accuracy is only +/- 16ms
     * @return Always zero
     */
    int gettimeofday(qi::os::timeval *t)
    {
      if (!gTimeStorage)
        init_timer();
      if (gTimeStorage->usePerformanceCounter) {
        LARGE_INTEGER lCurrentSystemTicks;

        // gets the elapsed ticks since system startup (test using CPU affinity)
        DWORD_PTR oldmask = SetThreadAffinityMask(GetCurrentThread(), 0);
        QueryPerformanceCounter(&lCurrentSystemTicks);
        SetThreadAffinityMask(GetCurrentThread(), oldmask);

        // remove the initial offset
        lCurrentSystemTicks.QuadPart -= gTimeStorage->systemStartTicks.QuadPart;

        // convert to a double number of seconds, using the ticksPerSecond
        double secondsElapsedDouble = ((double)lCurrentSystemTicks.QuadPart) /
          ((double)gTimeStorage->systemTicksPerSecond.QuadPart);

        // convert to the parts needed for the timeval
        long seconds = long(secondsElapsedDouble);
        long microseconds = long((secondsElapsedDouble - seconds) * 1000000);

        // add this offset to system startup time
        t->tv_sec  = gTimeStorage->systemStartTime.tv_sec  + seconds;
        t->tv_usec = gTimeStorage->systemStartTime.tv_usec + microseconds;
      } else {
        // no good performance counter, so revert to old behaviour
        struct _timeb timebuffer;
        _ftime64_s( &timebuffer );

        // store this in a timeval struct
        t->tv_sec=(long)timebuffer.time;
        t->tv_usec=1000*timebuffer.millitm;
      }
      return 0;
    }

    FILE* fopen(const char *filename, const char *mode) {
      try {
        boost::filesystem::path dest(filename);
        std::string  amode(mode);
        std::wstring wmode;
        std::copy(amode.begin(), amode.end(), std::back_inserter(wmode));
        return ::_wfopen(dest.wstring(qi::unicodeFacet()).c_str(), wmode.c_str());
      }
      catch (boost::filesystem::filesystem_error &) {
        return 0;
      }
    }

    int stat(const char *pFilename, struct ::stat* pStat) {
      try {
        struct _stat buffer;
        int result = ::_wstat(boost::filesystem::path(pFilename, qi::unicodeFacet()).wstring(qi::unicodeFacet()).c_str(), &buffer);
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
      std::string env = std::string(var) + std::string("=") + std::string(value);
      return _putenv(env.c_str());
    }

    void sleep(unsigned int seconds) {
      Sleep(seconds * 1000);
    }

    void msleep(unsigned int milliseconds) {
      Sleep(milliseconds);
    }


    std::string home()
    {
      std::string envHome = qi::os::getenv("HOME");
      if (envHome != "")
      {
        return boost::filesystem::path(envHome, qi::unicodeFacet()).make_preferred().string(qi::unicodeFacet());
      }

      // $HOME environment variable not defined:
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
      boost::filesystem::path res(envHomeDrive, qi::unicodeFacet());
      res /= envHomePath;
      return res.make_preferred().string(qi::unicodeFacet());
    }

    std::string tmpdir(const char *prefix)
    {
      boost::filesystem::path path;
      path = qi::os::getenv("TMP");

      std::string filename(prefix);
      filename += path.filename().string();
      path = path.parent_path() / filename;

      if (!boost::filesystem::exists(path))
        boost::filesystem::create_directories(path);

      return path.string(qi::unicodeFacet());
    }
  }
}
