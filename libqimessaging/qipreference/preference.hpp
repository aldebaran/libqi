/*
**
** Author(s):
**  - Herve Cuche <hcuche@aldebaran-robotics.com>
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2011 Aldebaran Robotics
*/


/** @file
 *  @brief
 */


#pragma once

#ifndef _LIBQI_PREFERENCE_HPP_
# define _LIBQI_PREFERENCE_HPP_

# include <string>
# include <map>
# include <vector>

# include <qimessaging/value.hpp>

/**
 *  @namespace qi::pref
 *
 *  @brief
 *
 */

namespace qi {
  namespace pref {

    class PreferenceMapPrivate;

    class PreferenceMap
    {
    public:
      PreferenceMap();
      ~PreferenceMap();

      void load(const std::string &file);
      void save(const std::string &file);

      //get a value
      const qi::Value &get(const std::string &name);
      //set a value
      void set(const std::string &name, const qi::Value &val);

      // delete a preference entry
      void remove(const std::string &name);
      // find existing keys, which names start with `prefix'
      std::vector<std::string> keys(const std::string &prefix = "");

      //return all values
      std::map<std::string, qi::Value> values();

    protected:
    private:
      PreferenceMapPrivate* _private;
    };
  }
} // !qi

#endif // !_LIBQI_PREFERENCE_HPP_
