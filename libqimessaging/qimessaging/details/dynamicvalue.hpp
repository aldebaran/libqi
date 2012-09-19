#pragma once
/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/

#ifndef _QIMESSAGING_DETAILS_DYNAMICVALUE_HPP_
#define _QIMESSAGING_DETAILS_DYNAMICVALUE_HPP_

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
  class QIMESSAGING_API DynamicValue {
  public:
    typedef std::vector<DynamicValue> DynamicValueList;
    typedef std::map<std::string, DynamicValue> DynamicValueMap;
    enum Type {
      Invalid,
      Double,
      String,
      List,
      Map,
      Opaque
    };

    DynamicValue();
    DynamicValue(const DynamicValue& b);
    DynamicValue(double d);
    DynamicValue(const std::string& s);
    DynamicValue(const DynamicValueList& v);
    DynamicValue(const DynamicValueMap& v);
    ~DynamicValue();
    DynamicValue& operator = (const DynamicValue& b);

    void setDouble(double d);
    void setString(const std::string& s);
    void setList(const DynamicValueList& v);
    void setMap(const DynamicValueMap& m);

    double           toDouble() const;
    std::string      toString() const;
    const DynamicValueList& toList()   const;
    const DynamicValueMap&  toMap()    const;

    void clear();

    template<typename T> T as() const;
    template<typename T> void set(const T& v);

    union {
      double                        d;
      void                         *ptr;
      std::string                  *str;
      std::vector<DynamicValue>           *list;
      std::map<std::string, DynamicValue> *map;
    } data;
    Type type;
  };

  // Custom conversion to-from value
  template<typename T> struct DynamicValueConverter
  {
    static void writeDynamicValue(const T& src, DynamicValue& dst);
    static void readDynamicValue(const DynamicValue& src, T& dst);
  };

  }
}

#include <qimessaging/details/dynamicvalue.hxx>

#endif  // _QIMESSAGING_DETAILS_DYNAMICVALUE_HPP_
