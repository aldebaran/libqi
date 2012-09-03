/*
*  Author(s):
*  - Chris  Kilner <ckilner@aldebaran-robotics.com>
*  - Cedric Gestes <gestes@aldebaran-robotics.com>
*
*  Copyright (C) 2010-2012 Aldebaran Robotics
*/


#pragma once
#ifndef _QIMESSAGING_VALUE_HPP_
#define _QIMESSAGING_VALUE_HPP_

#include <qi/types.hpp>

#include <qimessaging/api.hpp>

#include <string>
#include <list>
#include <vector>
#include <map>

namespace qi {

  namespace detail {
  /** Generic type used as common ground for conversions.
  *
  */
  class QIMESSAGING_API Value {
  public:
    typedef std::vector<Value> ValueList;
    typedef std::map<std::string, Value> ValueMap;
    enum Type {
      Invalid,
      Double,
      String,
      List,
      Map,
      Opaque
    };

    Value();
    Value(double d);
    Value(const std::string& s);
    Value(const ValueList& v);
    Value(const ValueMap& v);
    ~Value();

    void setDouble(double d);
    void setString(const std::string& s);
    void setList(const ValueList& v);
    void setMap(const ValueMap& m);

    double           toDouble() const;
    std::string      toString() const;
    const ValueList& toList()   const;
    const ValueMap&  toMap()    const;

    void clear();

    template<typename T> T as() const;
    template<typename T> void set(const T& v);

    union {
      double                        d;
      void                         *ptr;
      std::string                  *str;
      std::vector<Value>           *list;
      std::map<std::string, Value> *map;
    } data;
    Type type;
  };

  // Custom conversion to-from value
  template<typename T> struct ValueConverter
  {
    static void writeValue(const T& src, Value& dst);
    static void readValue(const Value& src, T& dst);
  };

  }
}

#include <qimessaging/details/value.hxx>

#endif  // _QIMESSAGING_VALUE_HPP_
