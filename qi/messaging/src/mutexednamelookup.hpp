#pragma once
/*
*  Author(s):
*  - Chris  Kilner <ckilner@aldebaran-robotics.com>
*  - Cedric Gestes <gestes@aldebaran-robotics.com>
*
*  Copyright (C) 2010 Aldebaran Robotics
*/


#ifndef _QI_MESSAGING_SRC_MUTEXEDNAMELOOKUP_HPP_
#define _QI_MESSAGING_SRC_MUTEXEDNAMELOOKUP_HPP_

#include <map>
#include <vector>
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
        typename std::map<std::string, T>::const_iterator it;
        it = _map.find(key);
        if (it != _map.end()) {
          return it->second;
        }
        return _invalidValue;
      }

      bool exists(std::string key) {
        boost::mutex::scoped_lock lock(_mutex);
        typename std::map<std::string, T>::const_iterator it;
        it = _map.find(key);
        return !(it == _map.end());
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

      void setInvalidValue(const T& val) {
        _invalidValue = val;
      }

      bool empty() {
        return _map.empty();
      }

      std::vector<std::string> getKeys() {
        boost::mutex::scoped_lock lock(_mutex);
        typename std::map<std::string, T>::const_iterator it;
        std::vector<std::string> result;
        result.resize(_map.size());
        int i = 0;
        for(it = _map.begin(); it != _map.end(); ++it) {
          result[i++] = it->first;
        }
        return result;
      }

      std::vector<std::string> getKeysWhereValueEquals(const T& val) {
        boost::mutex::scoped_lock lock(_mutex);
        typename std::map<std::string, T>::const_iterator it;
        std::vector<std::string> result;
        for(it = _map.begin(); it != _map.end(); ++it) {
          if (it->second == val) {
            result.push_back(it->first);
          }
        }
        return result;
      }

    };
  }
}

#endif  // _QI_MESSAGING_SRC_MUTEXEDNAMELOOKUP_HPP_

