/*
*  Author(s):
*  - Cedric Gestes <gestes@aldebaran-robotics.com>
*  - Chris  Kilner <ckilner@aldebaran-robotics.com>
*
*  Copyright (C) 2010, 2011 Aldebaran Robotics
*/

#include <qimessaging/value.hpp>

namespace qi {


  Value::Value() {
  }

  Value::Value(Type t) {
    clear();
    _private.type = t;
    switch (t) {
      case Value::String:
        _private.data.ptr = new std::string();
        break;
      case Value::List:
        _private.data.ptr = new ValueList();
        break;
      case Value::Vector:
        _private.data.ptr = new ValueVector();
        break;
      case Value::Map:
        _private.data.ptr = new ValueMap();
        break;
    }
  }

  Value::Value(bool b) {
    clear();
    _private.type = Value::Bool;
    _private.data.b = b;
  }

  Value::Value(char c) {
    clear();
    _private.type = Value::Char;
    _private.data.c = c;
  }

  Value::Value(int i) {
    clear();
    _private.type = Value::Int32;
    _private.data.i = i;
  }

  Value::Value(unsigned int i) {
    clear();
    _private.type = Value::UInt32;
    _private.data.ui = i;
  }

  Value::Value(long long l) {
    clear();
    _private.type = Value::Int64;
    _private.data.l = l;
  }

  Value::Value(unsigned long long l) {
    clear();
    _private.type = Value::UInt64;
    _private.data.ul = l;
  }

  Value::Value(float f) {
    clear();
    _private.type = Value::Float;
    _private.data.f = f;
  }

  Value::Value(double d) {
    clear();
    _private.type = Value::Double;
    _private.data.d = d;
  }

  Value::Value(const char *str) {
    clear();
    _private.type = Value::String;
    _private.data.ptr = new std::string(str);
  }

  Value::Value(const std::string &str) {
    clear();
    _private.type = Value::String;
    _private.data.ptr = new std::string(str);
  }

  Value::Value(const std::list<Value> &l) {
    clear();
    _private.type = Value::List;
    _private.data.ptr = new std::list<Value>(l);
  }

  Value::Value(const std::vector<Value> &v) {
    clear();
    _private.type = Value::Vector;
    _private.data.ptr = new std::vector<Value>(v);
  }

  Value::Value(const std::map<std::string, Value> &m) {
    clear();
    _private.type = Value::Map;
    _private.data.ptr = new std::map<std::string, Value>(m);
  }

#ifdef QI_QT
  Value::Value(const QList<Value> &l);
  Value::Value(const QVector<Value> &l);
  Value::Value(const QMap<QString, Value> &l);
#endif


  bool                         Value::toBool() {
    if (_private.type == Value::Bool)
      return _private.data.b;
    throw ValueError("value can't be converted to bool");
  }

  char                         Value::toChar() {
    if (_private.type == Value::Char)
      return _private.data.c;
    throw ValueError("value can't be converted to char");
  }

  int                          Value::toInt32() {
    if (_private.type == Value::Int32)
      return _private.data.i;
    throw ValueError("value can't be converted to int32");
  }

  unsigned int                 Value::toUInt32() {
    if (_private.type == Value::UInt32)
      return _private.data.ui;
    throw ValueError("value can't be converted to uint32");
  }

  long long                    Value::toInt64() {
    if (_private.type == Value::Int32)
      return _private.data.i;
    if (_private.type == Value::Int64)
      return _private.data.l;
    throw ValueError("value can't be converted to int64");
  }

  unsigned long long           Value::toUInt64() {
    if (_private.type == Value::UInt32)
      return _private.data.ui;
    if (_private.type == Value::UInt64)
      return _private.data.ul;
    throw ValueError("value can't be converted to uint64");
  }

  float                        Value::toFloat() {
    if (_private.type == Value::Float)
      return _private.data.f;
    throw ValueError("value can't be converted to float");
  }

  double                       Value::toDouble() {
    if (_private.type == Value::Float)
      return _private.data.f;
    if (_private.type == Value::Double)
      return _private.data.d;
    throw ValueError("value can't be converted to double");
  }

  std::string                  Value::toString() {
    if (_private.type == Value::String)
      return *reinterpret_cast<std::string *>(_private.data.ptr);
    throw ValueError("value can't be converted to string");
  }

  std::list<Value>             Value::toList() {
    if (_private.type == Value::List)
      return *reinterpret_cast< std::list<Value> *>(_private.data.ptr);
    throw ValueError("value can't be converted to list");
  }

  std::vector<Value>           Value::toVector() {
    if (_private.type == Value::Vector)
      return *reinterpret_cast< std::vector<Value> *>(_private.data.ptr);
    throw ValueError("value can't be converted to vector");
  }

  std::map<std::string, Value> Value::toMap() {
    if (_private.type == Value::Map)
      return *reinterpret_cast< std::map<std::string, Value> *>(_private.data.ptr);
    throw ValueError("value can't be converted to map");
  }

#ifdef QI_QT
  QString                      Value::toQString();
  QList<Value>                 Value::toQList();
  QVector<Value>               Value::toQVector();
  QMap<QString, Value>         Value::toQMap();
#endif

  void Value::clear() {
    switch (_private.type) {
      case Value::String:
        delete reinterpret_cast< std::string *>(_private.data.ptr);;
      case Value::List:
        delete reinterpret_cast< std::list<std::string> *>(_private.data.ptr);;
      case Value::Vector:
        delete reinterpret_cast< std::vector<std::string> *>(_private.data.ptr);;
      case Value::Map:
        delete reinterpret_cast< std::map<std::string, Value> *>(_private.data.ptr);;
    }
    _private.type = Value::Invalid;
    _private.data.ptr = 0;
  }

  Value::Type Value::type() const {
    return (Value::Type) _private.type;
  }


}

