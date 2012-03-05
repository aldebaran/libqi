/*
*  Author(s):
*  - Chris  Kilner <ckilner@aldebaran-robotics.com>
*  - Cedric Gestes <gestes@aldebaran-robotics.com>
*
*  Copyright (C) 2010, 2011 Aldebaran Robotics
*/


#pragma once
#ifndef _QIMESSAGING_VALUE_HXX_
#define _QIMESSAGING_VALUE_HXX_

# include <qi/log.hpp>

namespace qi {

  template <typename T>
  inline void Value::setValue(const T &value) {
    qiLogDebug("qi::value") << "setValue not implemented for this type";
  }

  template <>
  inline void Value::setValue(const bool &value) {
    clear();
    _private.type   = Value::Bool;
    _private.data.b = value;
  }

  template <>
  inline void Value::setValue(const char &value) {
    clear();
    _private.type   = Value::Char;
    _private.data.c = value;
  }

  template <>
  inline void Value::setValue(const int &value) {
    clear();
    _private.type   = Value::Int32;
    _private.data.i = value;
  }

  template <>
  inline void Value::setValue(const float &value) {
    clear();
    _private.type   = Value::Float;
    _private.data.f = value;
  }

  template <>
  inline void Value::setValue(const double &value) {
    clear();
    _private.type   = Value::Double;
    _private.data.d = value;
  }

  template <>
  inline void Value::setValue(const std::string &value) {
    clear();
    _private.type   = Value::Int32;
    _private.data.ptr = new std::string(value);
  }

  template <>
  inline void Value::setValue(const ValueList &value) {
    clear();
    _private.type   = Value::List;
    _private.data.ptr = new ValueList(value);
  }

  template <>
  inline void Value::setValue(const ValueVector &value) {
    clear();
    _private.type   = Value::Vector;
    _private.data.ptr = new ValueVector(value);
  }

  template <>
  inline void Value::setValue(const ValueMap &value) {
    clear();
    _private.type   = Value::Map;
    _private.data.ptr = new ValueMap(value);
  }

  template <typename T>
  const T *Value::value() const {
    return 0;
  }

  template <>
  inline const bool *Value::value() const {
    if (_private.type == Value::Bool)
      return &_private.data.b;
    return 0;
  }

  template <>
  inline const char *Value::value() const {
    if (_private.type == Value::Char)
      return &_private.data.c;
    return 0;
  }

  template <>
  inline const int *Value::value() const {
    if (_private.type == Value::Int32)
      return &_private.data.i;
    return 0;
  }

  template <>
  inline const float *Value::value() const {
    if (_private.type == Value::Float)
      return &_private.data.f;
    return 0;
  }

  template <>
  inline const double *Value::value() const {
    if (_private.type == Value::Double)
      return &_private.data.d;
    return 0;
  }

  template <>
  inline const std::string *Value::value() const {
    if (_private.type == Value::String)
      return reinterpret_cast<std::string *>(_private.data.ptr);
    return 0;
  }

  template <>
  inline const std::list<qi::Value> *Value::value() const {
    if (_private.type == Value::List)
      return reinterpret_cast< std::list<Value> *>(_private.data.ptr);
    return 0;
  }

  template <>
  inline const std::vector<qi::Value> *Value::value() const {
    if (_private.type == Value::Vector)
      return reinterpret_cast< std::vector<Value> *>(_private.data.ptr);
    return 0;
  }

  template <>
  inline const std::map<std::string, qi::Value> *Value::value() const {
    if (_private.type == Value::Map)
      return reinterpret_cast< std::map<std::string, Value> *>(_private.data.ptr);
    return 0;
  }

};

#endif  // _QIMESSAGING_VALUE_HXX_
