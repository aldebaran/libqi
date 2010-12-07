#pragma once
/*
*  Author(s):
*  - Cedric Gestes <gestes@aldebaran-robotics.com>
*  - Chris  Kilner <ckilner@aldebaran-robotics.com>
*
*  Copyright (C) 2010 Aldebaran Robotics
*/


#ifndef _QI_PERF_TO_STRING_HPP_
#define _QI_PERF_TO_STRING_HPP_

#include <string>

template <typename T>
std::string toString(T arg)
{
  std::stringstream s;
  s << arg;
  return s.str();
}

#endif  // _QI_PERF_TO_STRING_HPP_
