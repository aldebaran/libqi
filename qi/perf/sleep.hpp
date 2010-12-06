#pragma once
/*
*  Author(s):
*  - Cedric Gestes <gestes@aldebaran-robotics.com>
*  - Chris  Kilner <ckilner@aldebaran-robotics.com>
*
*  Copyright (C) 2010 Aldebaran Robotics
*/


#ifndef _QI_PERF_SLEEP_HPP_
#define _QI_PERF_SLEEP_HPP_

#ifdef _WIN32
# include <winsock2.h>

  template<typename T>
  inline void sleep(const T& t) {
    ::Sleep((int)(t * 1000));
  }

#else
# include <unistd.h>
#endif

#endif  // _QI_PERF_SLEEP_HPP_
