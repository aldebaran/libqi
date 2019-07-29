#pragma once

#include <boost/predef/os.h>

#if !BOOST_OS_WINDOWS
#  error "This header should only be included on Windows!"
#endif

#include <string>
#include <windows.h>

namespace qi { namespace os {

  // Returns a string representation of the last system error
  // acquired from `GetLastError()`.
  std::string lastErrorMessage();

  // Returns a string representation of the last socket error
  // acquired from `WSAGetLastError()`.
  std::string lastSocketErrorMessage();

  // Returns a string representation of the provided system error.
  // The error code should be provided by Windows API functions
  // like `GetLastError()`.
  std::string translateSystemError(DWORD errorCode);

}}
