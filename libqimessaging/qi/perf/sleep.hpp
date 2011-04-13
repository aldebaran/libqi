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

  template<typename T>
  inline void msleep(const T& t) {
    ::Sleep((int)(t));
  }

#else
# include <unistd.h>

  inline void msleep(const long &t) {
    //TODO if sleeptime is bigger that 1sec, this will not work... prefer using nanosleep
    usleep(t * 1000);
  }

#endif

#endif  // _QI_PERF_SLEEP_HPP_
