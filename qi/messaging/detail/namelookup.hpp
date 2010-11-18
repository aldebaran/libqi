#pragma once
/*
** Author(s):
**  - Chris Kilner <ckilner@aldebaran-robotics.com>
**
** Copyright (C) 2010 Aldebaran Robotics
*/
#ifndef   __QI_MESSAGING_DETAIL_NAMELOOKUP_HPP__
#define   __QI_MESSAGING_DETAIL_NAMELOOKUP_HPP__

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

#endif // __QI_MESSAGING_DETAIL_NAMELOOKUP_HPP__

