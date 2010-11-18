#pragma once
/*
** Author(s):
**  - Chris Kilner <ckilner@aldebaran-robotics.com>
**
** Copyright (C) 2010 Aldebaran Robotics
*/
#ifndef COMMON_NAMELOOKUP_H_
#define COMMON_NAMELOOKUP_H_

#include <map>
#include <string>

namespace qi {
  namespace detail {

    // Use std::map for the moment. Future: HashMap or dense_hash_map
    template<typename T>
    class NameLookup : public std::map<std::string, T> {
    public:
      NameLookup() : std::map<std::string, T>() {}
    };
  }  // namespace Nodes
}  // namespace qi

#endif  // COMMON_NODE_H_

