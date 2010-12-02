#pragma once
/*
*  Author(s):
*  - Chris  Kilner <ckilner@aldebaran-robotics.com>
*  - Cedric Gestes <gestes@aldebaran-robotics.com>
*
*  Copyright (C) 2010 Aldebaran Robotics
*/

#include <qi/transport/detail/network/host_name.hpp>

#ifdef _WIN32
#include <windows.h>
#include <winsock2.h>
#include <iphlpapi.h>
#pragma comment(lib, "IPHLPAPI.lib")

namespace qi {
  namespace detail {
    std::string getHostName() {
      DWORD dwBufferSize = MAX_COMPUTERNAME_LENGTH + 1;
      TCHAR chrComputerName[MAX_COMPUTERNAME_LENGTH + 1];
      if(GetComputerName(chrComputerName, &dwBufferSize)) {
        return std::string(chrComputerName);
      } else {
        return "";
      }
    }
  }
}
#else  // end WIN32

# include <sys/types.h>
# include <unistd.h>

namespace qi {
  namespace detail {
    std::string getHostName() {
      char szHostName[128] = "";
      if (gethostname(szHostName, sizeof(szHostName) -1) == 0) {
        return std::string(szHostName);
      }
      return "";
    }
  }
}
#endif
