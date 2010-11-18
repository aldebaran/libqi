#pragma once
/*
** Author(s):
**  - Chris Kilner <ckilner@aldebaran-robotics.com>
**
** Copyright (C) 2010 Aldebaran Robotics
*/
#ifndef   __QI_MESSAGING_DETAIL_MUTEXEDNAMELOOKUP_HPP__
#define   __QI_MESSAGING_DETAIL_MUTEXEDNAMELOOKUP_HPP__

#include <map>
#include <string>
#include <boost/thread/thread.hpp>
#include <boost/thread/mutex.hpp>

namespace qi {
  namespace detail {

    template<typename T>
    class MutexedNameLookup {
    private:
      std::map<std::string, T> _map;
      T                        _invalidValue;
      boost::mutex             _mutex;
    public:
      MutexedNameLookup() {}

      void replace(const std::map<std::string, T>& other) {
        boost::mutex::scoped_lock lock(_mutex);
        _map = other;
      }

      const std::map<std::string, T>& getMap() {
        boost::mutex::scoped_lock lock(_mutex);
        return _map;
      }

      const T& get(std::string key) {
        boost::mutex::scoped_lock lock(_mutex);
        typename std::map<std::string, T>::const_iterator it = _map.find(key);
        if (it != _map.end()) {
          return it->second;
        }
        return _invalidValue;
      }

      void insert(const std::string key, const T& val) {
        boost::mutex::scoped_lock lock(_mutex);
        typename std::map<std::string, T>::const_iterator it = _map.find(key);
        if (it != _map.end()) {
          _map.erase(key);
        }
        _map.insert(make_pair(key, val));
      }

      void remove(const std::string key) {
        boost::mutex::scoped_lock lock(_mutex);
        _map.erase(key);
      }
    };
  }
}

#endif // __QI_MESSAGING_DETAIL_MUTEXEDNAMELOOKUP_HPP__

