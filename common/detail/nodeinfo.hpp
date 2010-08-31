#pragma once
/*
** Author(s):
**  - Chris Kilner <ckilner@aldebaran-robotics.com>
**
** Copyright (C) 2010 Aldebaran Robotics
*/
#ifndef COMMON_NODEINFO_HPP_
#define COMMON_NODEINFO_HPP_

#include <string>

namespace AL {
  namespace Common {
    struct NodeInfo {

      std::string name;
      std::string address;

      NodeInfo() {}

      NodeInfo(const std::string& pName,
        const std::string & pAddress) :
          name(pName), address(pAddress) {}
    };
  }
}

#endif  // COMMON_NODEINFO_HPP_

