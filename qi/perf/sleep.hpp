/*
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2010 Aldebaran Robotics
*/

#ifndef   __QI_PERF_SLEEP_HPP__
#define   __QI_PERF_SLEEP_HPP__

#ifdef _WIN32
# include <winsock2.h>

  template<typename T>
  inline void sleep(const T& t) {
    ::Sleep((int)(t * 1000));
  }

#else
# include <unistd.h>
#endif

#endif // __QI_PERF_SLEEP_HPP__
