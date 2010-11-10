#pragma once
/*
** Author(s):
**  - Chris Kilner <ckilner@aldebaran-robotics.com>
**
** Copyright (C) 2010 Aldebaran Robotics
*/
#ifndef COMMON_MUTEXEDNAMELOOKUP_HPP_
#define COMMON_MUTEXEDNAMELOOKUP_HPP_

#include <map>
#include <string>
#include <boost/thread/thread.hpp>
#include <boost/thread/mutex.hpp>

namespace qi {
  namespace Nodes {

    template<typename T>
    class MutexedNameLookup {
    private:
      std::map<std::string, T> fMap;
      T fInvalidValue;
      boost::mutex fMutex;
    public:
      MutexedNameLookup() {}

      void replace(const std::map<std::string, T>& other) {
        boost::mutex::scoped_lock lock(fMutex);
        fMap = other;
      }

      const std::map<std::string, T>& getMap() {
        boost::mutex::scoped_lock lock(fMutex);
        return fMap;
      }

      const T& get(std::string key) {
        boost::mutex::scoped_lock lock(fMutex);
        typename std::map<std::string, T>::const_iterator it = fMap.find(key);
        if (it != fMap.end()) {
          return it->second;
        }
        return fInvalidValue;
      }

      void insert(const std::string key, const T& val) {
        boost::mutex::scoped_lock lock(fMutex);
        typename std::map<std::string, T>::const_iterator it = fMap.find(key);
        if (it != fMap.end()) {
          fMap.erase(key);
        }
        fMap.insert(make_pair(key, val));
      }

      void remove(const std::string key) {
        boost::mutex::scoped_lock lock(fMutex);
        fMap.erase(key);
      }
    };
  }
}

#endif  // COMMON_MUTEXEDNAMELOOKUP_HPP_

