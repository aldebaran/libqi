/*
 * Copyright (c) 2012 Aldebaran Robotics. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the COPYING file.
 */

#include <boost/filesystem.hpp>
#include <locale>
#include <cstdio>
#include <cstdlib>
#include <algorithm>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/timeb.h>

#include <iostream>
#include <io.h>       //_wopen
#include <windows.h>  //Sleep
#include <winsock2.h>
#include <direct.h>   // _mkdir
#include <iphlpapi.h> // GetComputerName

#include <qi/error.hpp>
#include <qi/log.hpp>
#include <qi/os.hpp>
#include <qi/qi.hpp>
#include "src/filesystem.hpp"

#include "utils.hpp"

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
        return ::_wfopen(boost::filesystem::path(filename, qi::unicodeFacet()).wstring(qi::unicodeFacet()).c_str(),
                         boost::filesystem::path(mode, qi::unicodeFacet()).wstring(qi::unicodeFacet()).c_str());
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
      size_t       bufSize;
      std::wstring wvar = boost::filesystem::path(var, qi::unicodeFacet()).wstring(qi::unicodeFacet());

     #ifdef _MSC_VER
      wchar_t     *envDir = NULL;
      _wdupenv_s(&envDir, &bufSize, wvar.c_str());
      if (envDir == NULL)
        return "";

      boost::filesystem::path dest(envDir, qi::unicodeFacet());
      std::string ret(dest.string(qi::unicodeFacet()).c_str());
      free(envDir);
      return ret;
    #else
      _wgetenv_s(&bufSize, NULL, 0,  wvar.c_str());

      wchar_t *envDir = (wchar_t *) malloc(bufSize * sizeof(wchar_t));
      _wgetenv_s(&bufSize, envDir, bufSize, wvar.c_str());

      if (envDir == NULL)
        return "";

      boost::filesystem::path dest(envDir, qi::unicodeFacet());
      std::string ret(dest.string(qi::unicodeFacet()).c_str());
      free(envDir);
      return ret;
     #endif
    }

    int setenv(const char *var, const char *value) {
      return _wputenv_s(boost::filesystem::path(var, qi::unicodeFacet()).wstring(qi::unicodeFacet()).c_str(),
                        boost::filesystem::path(value, qi::unicodeFacet()).wstring(qi::unicodeFacet()).c_str());
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

    std::string mktmpdir(const char *prefix)
    {
      std::string  sprefix;
      std::wstring tmpdir;
      std::string  path;
      int          i = 0;

      if (prefix)
        sprefix = prefix;
      boost::filesystem::path pp;
      do
      {
        tmpdir = boost::filesystem::path(sprefix, qi::unicodeFacet()).wstring(qi::unicodeFacet());
        tmpdir += wrandomstr(7);
        pp = boost::filesystem::path(qi::os::tmp(), qi::unicodeFacet());
        pp.append(tmpdir, qi::unicodeFacet());
        path = pp.make_preferred().string(qi::unicodeFacet());
        ++i;
      }
      while (_wmkdir(pp.wstring(qi::unicodeFacet()).c_str()) == -1 && i < TMP_MAX);

      return path;
    }

    std::string tmp()
    {
      wchar_t* tempPath = (wchar_t*)malloc(sizeof(wchar_t) * MAX_PATH);
      DWORD len = ::GetTempPathW(MAX_PATH, tempPath);
      boost::filesystem::path dest;

      if (len > 0)
      {
        tempPath[len] = 0;
        dest = boost::filesystem::path(tempPath, qi::unicodeFacet());
      }

      if (dest.empty())
        return boost::filesystem::path("C:/tmp", qi::unicodeFacet()).string(qi::unicodeFacet());

      free(tempPath);
      return  dest.make_preferred().string(qi::unicodeFacet());
    }

    std::string tmpdir(const char *prefix) {
      return mktmpdir(prefix);
    }

    std::string gethostname()
    {
      DWORD dwBufferSize = MAX_COMPUTERNAME_LENGTH + 1;
      WCHAR chrComputerName[MAX_COMPUTERNAME_LENGTH + 1];
      if (GetComputerNameW(chrComputerName, &dwBufferSize)) {
        return boost::filesystem::path((WCHAR *)chrComputerName, qi::unicodeFacet()).string(qi::unicodeFacet());
      }
      return std::string();
    }


    // Function to get real string representation from
    // GetLastError() and WSAGetLastError()
    static std::string GetLastErrorMessage(DWORD lastError)
    {
      TCHAR errmsg[512];

      if (!FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM,
                         0,
                         lastError,
                         0,
                         errmsg,
                         511,
                         NULL))
      {
        /* if we fail, call ourself to find out why and return that error */
        return (GetLastErrorMessage(GetLastError()));
      }

      return errmsg;
    }

    unsigned short findAvailablePort(unsigned short port)
    {
      static bool static_bWSAInit = false;
      if (!static_bWSAInit )
      {
        WORD wVersionRequested = MAKEWORD(2, 0);
        WSADATA wsaData;
        int nRet = WSAStartup(wVersionRequested, &wsaData);
        if (nRet == SOCKET_ERROR)
          qiLogError("core.common.network") << "WSAStartup returned error: %d\n"
                                            << WSAGetLastError() << std::endl;
        else
          static_bWSAInit = true;
      }

      struct sockaddr_in name;
      name.sin_family = AF_INET;
      name.sin_addr.s_addr = htonl(INADDR_ANY);
      name.sin_port = (u_short)htons(port);
      int sock = ::socket(AF_INET, SOCK_STREAM, 0);

      // CK July 8 2010
      // Set port options which make sure that the port is not
      // alredy in use.
      int optval = 1;
      int rc = setsockopt(sock, SOL_SOCKET, SO_EXCLUSIVEADDRUSE,
                          (char*)&optval, sizeof(optval));
      // CK August 2 2011
      // Force Brutal disconnect (not background) so that the socket is freed
      // as soon as disconnect returns
      // See: http://msdn.microsoft.com/en-us/library/ms737582(v=VS.85).aspx
      struct linger linger_setting;
      linger_setting.l_onoff  = 1;
      linger_setting.l_linger = 0;
      rc = setsockopt(sock, SOL_SOCKET, SO_LINGER,
                      (char*)&linger_setting, sizeof(linger_setting));

      // cast ushort into int to check all ports between
      // [0, 65535] (e.g. USHRT_MAX)
      int iPort = static_cast<int>(port);
      int unavailable = -1;
      do
      {
        name.sin_port = htons(iPort);
        unavailable = ::bind(sock, (struct sockaddr *)&name, sizeof(name));

        if (!unavailable)
        {
          unavailable = ::closesocket(sock);
          // See: http://msdn.microsoft.com/en-us/library/ms737582(v=VS.85).aspx last comments
          sock = INVALID_SOCKET; // do not make further use of sock.
          if (!unavailable)
            break;
        }
        ++iPort;
      }
      while (iPort <= USHRT_MAX);

      if (unavailable)
      {
        unavailable = WSAGetLastError();
        std::string error = GetLastErrorMessage(unavailable);

        qiLogError("core.common.network") << "freePort Socket Bind Error: "
                                          << error << std::endl;
        port = 0;
      }

      qiLogDebug("core.common.network") << "freePort: Returning port: "
                                        << port << std::endl;
      return port;
    }

  }
}
