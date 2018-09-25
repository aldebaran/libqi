#pragma once
#ifndef _QI_TESTS_MESSAGING_NETWORKASIONOOPLOCK_HPP
#define _QI_TESTS_MESSAGING_NETWORKASIONOOPLOCK_HPP
#include <src/messaging/sock/networkasio.hpp>

namespace qi { namespace sock {
  /// In unit tests, we want to perform a no-op lock.
  inline int scopelock(const NetworkAsio::ssl_socket_type&)
  {
    return 0;
  }
}}

#endif // _QI_TESTS_MESSAGING_NETWORKASIONOOPLOCK_HPP
