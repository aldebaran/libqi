#pragma once
/*
** Author(s):
**  - Chris Kilner <ckilner@aldebaran-robotics.com>
**
** Copyright (C) 2010 Aldebaran Robotics
*/
#ifndef   __QI_MESSAGING_DETAIL_CONSTANTS_HPP__
#define   __QI_MESSAGING_DETAIL_CONSTANTS_HPP__

#include <string>
namespace qi {
  namespace detail {
    static const int         kDefaultMasterPort = 5555;
    static const std::string kLocalHost         = "127.0.0.1";
  }
}
#endif // __QI_MESSAGING_DETAIL_CONSTANTS_HPP__

