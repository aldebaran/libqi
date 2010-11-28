/*
** Author(s):
**  - Chris Kilner <ckilner@aldebaran-robotics.com>
**
** Copyright (C) 2010 Aldebaran Robotics
*/
#include <qi/transport/detail/network/process_id.hpp>

#ifdef _WIN32
# include <windows.h>
#else
# include <sys/types.h>
# include <unistd.h>
#endif

namespace qi {
  namespace detail {
    int getProcessID() {
#ifdef _WIN32
      return GetCurrentProcessId();
#else
      return getpid();
#endif
    }
  }
}
