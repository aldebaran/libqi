/*
 * Copyright (c) 2012, 2013, 2015 Aldebaran Robotics. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the COPYING file.
 */

#include <boost/chrono/chrono.hpp>
#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>

#include <boost/predef/os.h>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/version.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/synchronized_value.hpp>

// Headers required for checking processes
#if BOOST_OS_WINDOWS
# include <Windows.h>
# include <Psapi.h>
# if _WIN32_WINNT >= 0x0602 //_WIN32_WINNT_WIN8
#  include <Processthreadsapi.h>
# else
#  include <WinBase.h>
# endif
#elif BOOST_OS_MACOS
# include <libproc.h>
#endif // no header required on linux to check processes
#ifdef WITH_SYSTEMD
# include <systemd/sd-id128.h>
#endif
#include <qi/path.hpp>
#include <qi/os.hpp>
#include <qi/log.hpp>
#include <qi/types.hpp>
#include <qi/uuid.hpp>

#include "sdklayout.hpp"

#if ANDROID && BOOST_COMP_GNUC
#include <sstream>
#endif

qiLogCategory("qi.os");

namespace bfs = boost::filesystem;

#if defined(WITH_SYSTEMD)
namespace {
  qi::Uuid getMachineIdFromSystemd()
  {
    // Ask libsystemd for the system's machine-id (the content of
    // `/etc/machine-id`).
    // Since this id should be kept confidential, we will use a salted version.
    // Note: libsystemd's sd_id128_get_machine_app_specific() is not used
    // because it relies on some linux kernel feature which may be unavailable.
    sd_id128_t systemMachineId = SD_ID128_NULL;
    const int res = sd_id128_get_machine(&systemMachineId);
    if (res == 0)
    {
      char systemMachineIdStr[SD_ID128_STRING_MAX];
      // Always succeeds, and sets the terminal NULL char.
      sd_id128_to_string(systemMachineId, systemMachineIdStr);
      // Our app salt (generated with `systemd-id128 new`)
      constexpr qi::Uuid salt{0xdd, 0x96, 0x97, 0x1d, 0x09, 0x12, 0x44, 0xc2,
                              0xa4, 0x07, 0x8e, 0x79, 0xa8, 0x29, 0x7b, 0x89};
      // Note: processes with different ids won't be able to
      // communicate through localhost. If the hash algorithm or the salt
      // is changed (e.g. with a libqi update), it must be changed for
      // all the communicating processes at once.
      using name_generator_sha1 =
          boost::uuids::name_generator_sha1;
      return name_generator_sha1(salt)(systemMachineIdStr);
    }
    std::ostringstream msg;
    msg << "failed to get a machine_id from libsystemd, error code: " << res;
    throw std::runtime_error(msg.str());
  };
} // namespace
#endif

namespace qi {
  namespace os {
    int64_t ustime()
    {
      return boost::chrono::duration_cast<qi::MicroSeconds>(
          SteadyClock::now().time_since_epoch()).count();
    }

    static inline void normalize_tv(qi::os::timeval *t)
    {
      const qi::int64_t usecPerSec = 1000 * 1000;
      t->tv_sec += t->tv_usec/usecPerSec;
      t->tv_usec %= usecPerSec;
      if (t->tv_usec < 0) {
        t->tv_usec += usecPerSec;
        --t->tv_sec;
      }
    }

    qi::os::timeval::timeval(qi::int64_t usec) : tv_sec(0), tv_usec(usec)
    {
      normalize_tv(this);
    }

    qi::os::timeval::timeval(const qi::Duration &d) :
      tv_sec(0),
      tv_usec(boost::chrono::duration_cast<qi::MicroSeconds>(d).count())
    {
      normalize_tv(this);
    }

    qi::os::timeval::timeval(const qi::SystemClockTimePoint &t) :
      tv_sec(0),
      tv_usec(boost::chrono::duration_cast<qi::MicroSeconds>(t.time_since_epoch()).count())
    {
      normalize_tv(this);
    }

    qi::os::timeval operator+(const qi::os::timeval &lhs,
                              const qi::os::timeval &rhs)
    {
      qi::os::timeval res;
      res.tv_sec = lhs.tv_sec + rhs.tv_sec;
      res.tv_usec = lhs.tv_usec + rhs.tv_usec;
      normalize_tv(&res);

      return res;
    }

    qi::os::timeval operator+(const qi::os::timeval &lhs,
                              long                   us)
    {
      const long usecPerSec = 1000 * 1000;

      qi::os::timeval res;
      res.tv_sec = lhs.tv_sec + (us / usecPerSec);
      res.tv_usec = lhs.tv_usec + (us % usecPerSec);
      normalize_tv(&res);

      return res;
    }

    qi::os::timeval operator-(const qi::os::timeval &lhs,
                              const qi::os::timeval &rhs)
    {
      qi::os::timeval res;
      res.tv_sec = lhs.tv_sec - rhs.tv_sec;
      res.tv_usec = lhs.tv_usec - rhs.tv_usec;
      normalize_tv(&res);

      return res;
    }

    qi::os::timeval operator-(const qi::os::timeval &lhs,
                              long                   us)
    {
      const long usecPerSec = 1000 * 1000;

      qi::os::timeval res;
      res.tv_sec = lhs.tv_sec - (us / usecPerSec);
      res.tv_usec = lhs.tv_usec - (us % usecPerSec);
      normalize_tv(&res);

      return res;
    }

    bool isProcessRunning(int pid, const std::string &fileName)
    {
      if (pid <= 0)
        return false;

      qiLogDebug() << "Checking if process #" << pid << " is running";
      std::string commandLine;

#if BOOST_OS_WINDOWS
      HANDLE processHandle = OpenProcess(
            PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, static_cast<DWORD>(pid));
      if (!processHandle)
        return false;
      qiLogDebug() << "Got handle for process #" << pid;

      DWORD exitCode = 0xFFFFFF;
      if (!GetExitCodeProcess(processHandle, &exitCode))
        return false;

      if (exitCode != STILL_ACTIVE)
        return false;

      qiLogDebug() << "Process #" << pid << " is running";
      if (fileName.empty())
        return true;

      qiLogDebug() << "Checking process name for #" << pid;
      WCHAR winCommandLine[MAX_PATH];
      DWORD clSize = GetProcessImageFileNameW(
            processHandle, winCommandLine,
            sizeof(winCommandLine) / sizeof(*winCommandLine));
      CloseHandle(processHandle);

      if(clSize <= 0)
        return false;

      commandLine = qi::Path::fromNative(winCommandLine).str();

#elif BOOST_OS_MACOS
      int numberOfProcesses = proc_listpids(PROC_ALL_PIDS, 0, NULL, 0);
      if (numberOfProcesses == 0)
      {
        qiLogError() << "Cannot get number of processes";
        return false;
      }

      std::vector<int> pids(numberOfProcesses, 0);
      numberOfProcesses = proc_listpids(PROC_ALL_PIDS, 0, pids.data(), sizeof(int) * pids.size());
      if (numberOfProcesses == 0)
      {
        qiLogError() << "Cannot get list of processes";
        return false;
      }

      if (std::find(pids.begin(), pids.end(), pid) == pids.end())
        return false;

      if (fileName.empty())
        return true;

      char procPidPath[PROC_PIDPATHINFO_MAXSIZE];
      int res = proc_pidpath(pid, procPidPath, sizeof(char) * PROC_PIDPATHINFO_MAXSIZE);
      if (!res)
      {
        qiLogDebug() << "Failed to get process info: " << strerror(errno);
        return false;
      }

      commandLine = procPidPath;

#else // Linux
      std::string pathInProc = "/proc/" + to_string(pid) + "/cmdline";

      std::ifstream file(pathInProc);
      qiLogDebug() << "process #" << pid << " " << (file.is_open() ? "exists" : "does not exist");

      if (!file)
        return false;

      if (fileName.empty())
      {
        file.close();
        return true;
      }

      std::getline(file, commandLine, '\0');
      file.close();
#endif

      qiLogDebug() << "process #" << pid << " full command was: " << commandLine;
      const std::string actualProcessName = qi::Path(commandLine).filename();
      qiLogDebug() << "process #" << pid << " executable was: "
                   << actualProcessName << ", expected: "
                   << (fileName + qi::path::detail::binSuffix());
      return actualProcessName == (fileName + qi::path::detail::binSuffix());
    }

    std::string getMachineId()
    {
      static const auto id = [] {
#if defined(WITH_SYSTEMD)
        return to_string(getMachineIdFromSystemd());
#elif ANDROID
        return generateUuid();
#else
        const qi::Path idFilePath(qi::path::userWritableConfPath("qimessaging", "machine_id"));
        boost::filesystem::ifstream idFile(idFilePath);

        if (idFile)
        {
          std::string id;
          idFile >> id;
          idFile.close();
          if (!id.empty()) {
            return id;
          } //else machine id is empty...
          qiLogWarning() << "machine_id is empty, generating a new one";
        }

        boost::filesystem::ofstream newIdFile(idFilePath);
        const auto id = generateUuid();
        if (newIdFile)
        {
          newIdFile << id;
          newIdFile.close();
        }
        else
        {
          qiLogError() << "Unable to create file: '" << idFilePath << "'";
        }
        return id;
#endif
      }();
      QI_ASSERT(id != "00000000-0000-0000-0000-000000000000");
      return id;
    }

    /// This is implemented in terms of getMachineId(), which is not as
    /// efficient as it could be, but :
    /// 1) it's guaranteed to not change the existing behaviour
    /// 2) we don't care because the result is only computed on the first call
    const Uuid& getMachineIdAsUuid()
    {
      static const Uuid uuid = [] {
        Uuid u;
        std::istringstream ss{getMachineId()};
        ss >> u;
        return u;
      }();
      return uuid;
    }

    namespace detail
    {
      static Uuid uuidGenerator()
      {
        static boost::synchronized_value<UuidRandomGenerator> g;
        return g->operator()();
      }
    }

    const Uuid& getProcessUuid()
    {
      static const auto uuid = detail::uuidGenerator();
      return uuid;
    }

    std::string generateUuid()
    {
      return to_string(detail::uuidGenerator());
    }

    void symlink(const qi::Path& source, const qi::Path& destination)
    {
      bfs::create_symlink(bfs::path(source.str(), qi::unicodeFacet()), bfs::path(destination.str(), qi::unicodeFacet()));
    }

    int gettimeofday(qi::os::timeval *t)
    {
      *t = qi::os::timeval(qi::SystemClock::now());
      return 0;
    }

    PtrUid ptrUid(void * address)
    {
      return PtrUid(os::getMachineIdAsUuid(), os::getProcessUuid(), address);
    }
  }
}
