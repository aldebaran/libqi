/*
 * Copyright (c) 2012, 2013 Aldebaran Robotics. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the COPYING file.
 */

#include "os_win32.hpp"

#include <boost/filesystem.hpp>
#include <locale>
#include <cstdio>
#include <cstdlib>
#include <cstdarg>
#include <algorithm>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/timeb.h>

#include <io.h>       //_wopen _isatty
#include <windows.h>  //Sleep, *CPU*
#include <winsock2.h>
#include <direct.h>   // _mkdir
#include <iphlpapi.h> // GetComputerName
#include <psapi.h>

# include <shlwapi.h>
# pragma comment(lib, "shlwapi.lib")

#include <boost/container/flat_map.hpp>
#include <boost/utility/string_ref.hpp>

#include <qi/log.hpp>
#include <qi/os.hpp>
#include <qi/path.hpp>
#include <ka/scoped.hpp>

#include "utils.hpp"

qiLogCategory("qi.os");

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

    FILE* fopen(const char *filename, const char *mode) {
      try {
        return ::_wfopen(boost::filesystem::path(filename, qi::unicodeFacet()).wstring().c_str(),
                         boost::filesystem::path(mode, qi::unicodeFacet()).wstring().c_str());
      }
      catch (boost::filesystem::filesystem_error &) {
        return  nullptr;
      }
    }

    int stat(const char *pFilename, struct ::stat* pStat) {
      try {
        struct _stat buffer;
        int result = ::_wstat(boost::filesystem::path(pFilename, qi::unicodeFacet()).wstring().c_str(), &buffer);
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
      std::wstring wvar = boost::filesystem::path(var, qi::unicodeFacet()).wstring();

      wchar_t *envDir = NULL;
     #ifdef _MSC_VER
      _wdupenv_s(&envDir, &bufSize, wvar.c_str());
    #else
      _wgetenv_s(&bufSize, NULL, 0,  wvar.c_str());

      envDir = (wchar_t *) malloc(bufSize * sizeof(wchar_t));
      _wgetenv_s(&bufSize, envDir, bufSize, wvar.c_str());
     #endif

      if (envDir == NULL)
        return "";

      qi::Path dest = boost::filesystem::path(envDir);
      std::string ret = dest.str();
      free(envDir);
      return ret;
    }

    std::string pathsep() {
      return ";";
    }

    int setenv(const char *var, const char *value) {
      return _wputenv_s(boost::filesystem::path(var, qi::unicodeFacet()).wstring().c_str(),
                        boost::filesystem::path(value, qi::unicodeFacet()).wstring().c_str());
    }

    int unsetenv(const char *var) {
      return _wputenv_s(boost::filesystem::path(var, qi::unicodeFacet()).wstring().c_str(),
                        L"");
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
        tmpdir = boost::filesystem::path(sprefix, qi::unicodeFacet()).wstring();
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

    std::string gethostname()
    {
      DWORD dwBufferSize = MAX_COMPUTERNAME_LENGTH + 1;
      WCHAR chrComputerName[MAX_COMPUTERNAME_LENGTH + 1];
      if (GetComputerNameW(chrComputerName, &dwBufferSize)) {
        return boost::filesystem::path((WCHAR *)chrComputerName, qi::unicodeFacet()).string(qi::unicodeFacet());
      }
      return std::string();
    }

    int isatty(int fd)
    {
      return ::_isatty(fd) == 0 ? 0 : 1;
    }

    char* strdup(const char *src)
    {
      return _strdup(src);
    }

    int snprintf(char *str, size_t size, const char *format, ...)
    {
      va_list list;
      va_start(list, format);
      int ret = vsnprintf_s(str, size, size, format, list);
      va_end(list);
      return ret;
    }


    std::string lastErrorMessage()
    {
      return translateSystemError(GetLastError());
    }

    std::string lastSocketErrorMessage()
    {
      return translateSystemError(WSAGetLastError());
    }

    std::string translateSystemError(DWORD errorCode)
    {
      LPVOID messageBufferPtr = nullptr;

      if (!FormatMessage(
          FORMAT_MESSAGE_ALLOCATE_BUFFER |
          FORMAT_MESSAGE_FROM_SYSTEM |
          FORMAT_MESSAGE_IGNORE_INSERTS,
          NULL,
          errorCode,
          MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
          (LPTSTR)& messageBufferPtr,
          0, NULL))
      {
        /* if we fail, call ourself to find out why and return that error */
        std::stringstream message;
        message << "Failed to allocate message buffer for error '" << errorCode
                << "' : " << lastErrorMessage();
        return message.str();
      }
      auto scopedBuffer = ka::scoped(messageBufferPtr, [&](LPVOID buffer) {
        if (LocalFree(buffer))
        {
          qiLogWarning() << "Failed to deallocate message buffer for error '" << errorCode
                         << "', error: " << lastErrorMessage();
        }
      });
      const std::string message = static_cast<const char*>(messageBufferPtr);
      return message;
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
          qiLogError() << "WSAStartup returned error: %d\n"
                       << WSAGetLastError() << std::endl;
        else
          static_bWSAInit = true;
      }

      struct sockaddr_in name;
      name.sin_family = AF_INET;
      name.sin_addr.s_addr = htonl(INADDR_ANY);
      name.sin_port = (u_short)htons(port);
      int sock = static_cast<int>(::socket(AF_INET, SOCK_STREAM, 0));

      // CK July 8 2010
      // Set port options which make sure that the port is not
      // alredy in use.
      int optval = 1;
      setsockopt(sock, SOL_SOCKET, SO_EXCLUSIVEADDRUSE,
                          (char*)&optval, sizeof(optval));
      // CK August 2 2011
      // Force Brutal disconnect (not background) so that the socket is freed
      // as soon as disconnect returns
      // See: http://msdn.microsoft.com/en-us/library/ms737582(v=VS.85).aspx
      struct linger linger_setting;
      linger_setting.l_onoff  = 1;
      linger_setting.l_linger = 0;
      setsockopt(sock, SOL_SOCKET, SO_LINGER,
                      (char*)&linger_setting, sizeof(linger_setting));

      // cast ushort into int to check all ports between
      // [49152, 65535] (e.g. USHRT_MAX)
      int iPort = port != 0 ? static_cast<int>(port) : 49152;
      int unavailable = -1;
      do
      {
        name.sin_port = htons(iPort);
        unavailable = ::bind(sock, (struct sockaddr *)&name, sizeof(name));

        if (!unavailable)
        {
          unavailable = ::closesocket(sock);
          // See: http://msdn.microsoft.com/en-us/library/ms737582(v=VS.85).aspx last comments
          sock = (int) INVALID_SOCKET; // do not make further use of sock.
          if (!unavailable)
            break;
        }
        ++iPort;
      }
      while (iPort <= USHRT_MAX);

      if (unavailable)
      {
        const auto error = lastSocketErrorMessage();
        qiLogError() << "freePort Socket Bind Error: "
                     << error << std::endl;
        iPort = 0;
      }

      qiLogDebug() << "freePort: Returning port: "
                   << iPort << std::endl;
      return iPort;
    }

    std::map<std::string, std::vector<std::string> > hostIPAddrs(bool ipv6Addr)
    {
      // TODO: replace usage of GetAdaptersInfo by a more "modern" Windows API (or Boost)

      // First call GetAdaptersInfo to get the necessary size for the adapter list.
      // See: http://msdn.microsoft.com/en-us/library/windows/desktop/aa365917(v=vs.85).aspx
      auto bufferSize = [&] () -> ULONG {
        ULONG result = 0;
        const DWORD dwRetVal = GetAdaptersInfo(nullptr, &result);
        if (dwRetVal != ERROR_BUFFER_OVERFLOW)
        {
          qiLogError() << "GetAdaptersInfo failed with error " << dwRetVal << " (1)";
          return 0;
        }
        return result;
      }();

      if(bufferSize == 0)
        return {};

      using AddressMap = std::map<std::string, std::vector<std::string>>;

      auto adaptersIPsMap = [&] () -> AddressMap {

        const auto scopedAdapters = ka::scoped((PIP_ADAPTER_INFO)malloc(bufferSize), &::free);
        if (scopedAdapters.value == NULL)
        {
          qiLogError() << "Error allocation memory needed to get hostIPAddrs";
          return {};
        }

        const DWORD dwRetVal = GetAdaptersInfo(scopedAdapters.value, &bufferSize);
        if (dwRetVal != NO_ERROR)
        {
          qiLogError() << "GetAdaptersInfo failed with error " << dwRetVal << " (2)";
          return {};
        }

        static const
        boost::container::flat_map<int, const char*> adapterTypes {
          { IF_TYPE_IEEE80211      , "802.11wireless"},
          { MIB_IF_TYPE_ETHERNET   , "Ethernet"      },
          { MIB_IF_TYPE_TOKENRING  , "TokenRing"     },
          { MIB_IF_TYPE_FDDI       , "FDDI"          },
          { MIB_IF_TYPE_PPP        , "PPP"           },
          { MIB_IF_TYPE_LOOPBACK   , "Loopback"      },
          { MIB_IF_TYPE_SLIP       , "Slip"          }
        };

        PIP_ADAPTER_INFO pAdapter = scopedAdapters.value;

        AddressMap foundIFS;

        while (pAdapter)
        {
          const auto typeIt = adapterTypes.find(pAdapter->Type);
          const char* type = typeIt != end(adapterTypes) ? typeIt->second : "Other";

          const boost::string_ref addr = pAdapter->IpAddressList.IpAddress.String;
          if (addr != "0.0.0.0")
          {
            foundIFS[type].push_back(pAdapter->IpAddressList.IpAddress.String);
          }
          pAdapter = pAdapter->Next;
        }

        return foundIFS;
      }();

      // not given by default
      if (adaptersIPsMap.find("Loopback") == adaptersIPsMap.end())
      {
        adaptersIPsMap["Loopback"].push_back("127.0.0.1");
      }

      return adaptersIPsMap;
    }

    void setCurrentThreadName(const std::string &name) {
      using THREADNAME_INFO = struct tagTHREADNAME_INFO
      {
        DWORD dwType; // must be 0x1000
        LPCSTR szName; // pointer to name (in user addr space)
        HANDLE dwThreadID; // thread ID (-1=caller thread)
        DWORD dwFlags; // reserved for future use, must be zero
      };

      THREADNAME_INFO info;
      info.dwType = 0x1000;
      info.szName = name.c_str();
      info.dwThreadID = (HANDLE)-1;
      info.dwFlags = 0;

      __try
      {
        RaiseException(0x406D1388, 0, sizeof(info)/sizeof(DWORD), (const ULONG_PTR*)&info);
      }
      __except (EXCEPTION_CONTINUE_EXECUTION)
      {
      }
    }

    std::string currentThreadName()
    {
      /*
       * It is impossible to get or set the name of the current thread on Windows.
       * Even if the setCurrentThreadName seems to do it, the RaiseException is
       * only catched by the debugger, which remember the name, it's not stored anywhere else.
       */
      return std::string("");
    }

    long numberOfCPUs()
    {
      SYSTEM_INFO info;
      GetSystemInfo(&info);
      return info.dwNumberOfProcessors;
    }

    bool setCurrentThreadCPUAffinity(const std::vector<int> &cpus) {

      if (cpus.size() == 0)
        return false;

      DWORD64 mask = 0;
      int i = 0;

      for (std::vector<int>::const_iterator it = cpus.begin(); it != cpus.end(); ++it)
      {
        mask |= 1ULL << *it;
        i++;
      }

      DWORD_PTR ret = SetThreadAffinityMask(GetCurrentThread(), (DWORD_PTR) mask);

      if (!ret)
      {
        const auto errorMessage = lastErrorMessage();
        qiLogError() << errorMessage;
        return false;
      }
      return true;
    }

    std::string timezone()
    {
      TIME_ZONE_INFORMATION tzInfo;
      DWORD err = GetTimeZoneInformation(&tzInfo);

      if (err == TIME_ZONE_ID_INVALID)
      {
        qiLogError() << "Cannot get timezone.";
        return std::string();
      }

      size_t origsize = wcslen(tzInfo.StandardName) + 1;
      const size_t newsize = 128;
      size_t convertedChars = 0;
      char nstring[newsize];
      wcstombs_s(&convertedChars, nstring, origsize, tzInfo.StandardName, _TRUNCATE);

      return std::string(nstring);
    }

    bool fnmatch(const std::string &pattern, const std::string &string)
    {
      return (PathMatchSpec(string.c_str(), pattern.c_str()) == TRUE);
    }

    std::pair<int64_t, int64_t> cputime()
    {
      FILETIME tCreate, tExit, tKernel, tUser;
      GetThreadTimes(GetCurrentThread(), &tCreate, &tExit, &tKernel, &tUser);
      // unit : 100ns
      return std::make_pair(
        ((int64_t)tUser.dwLowDateTime / 10 + ((int64_t)tUser.dwHighDateTime << 31) / 5),
        ((int64_t)tKernel.dwLowDateTime / 10 + ((int64_t)tKernel.dwHighDateTime << 31) / 5)
        );
    }

    size_t memoryUsage(unsigned int pid)
    {
      HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, (DWORD) pid);
      if (hProcess == NULL)
      {
        qiLogWarning() << "cannot get memory usage for PID " << pid << ": process doesn't exist";
        return 0;
      }

      PROCESS_MEMORY_COUNTERS counters;
      if (!GetProcessMemoryInfo(hProcess, &counters, sizeof(counters)))
      {
        const auto errorMessage = lastErrorMessage();
        qiLogWarning() << "cannot get memory usage for PID " << pid << ": " << errorMessage;
        CloseHandle(hProcess);
        return 0;
      }

      CloseHandle(hProcess);
      return (size_t) (counters.PagefileUsage / 1024);
    }
  }
}
