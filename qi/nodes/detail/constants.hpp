#pragma once
/*
** Author(s):
**  - Chris Kilner <ckilner@aldebaran-robotics.com>
**
** Copyright (C) 2010 Aldebaran Robotics
*/
#ifndef QI_NODE_DETAIL_CONSTANTS_HPP
#define QI_NODE_DETAIL_CONSTANTS_HPP

#include <string>
namespace qi {
  namespace detail {
    static const int         kDefaultMasterPort = 5555;
    static const std::string kLocalHost         = "127.0.0.1";
  }
}
#endif  // QI_NODE_DETAIL_CONSTANTS_HPP

