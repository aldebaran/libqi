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
  }

  Value::Value(bool b) {
  }

  Value::Value(char c) {
  }

  Value::Value(int i) {
  }

  Value::Value(unsigned int i) {
  }

  Value::Value(long long l) {
  }

  Value::Value(unsigned long long l) {
  }

  Value::Value(float f) {
  }

  Value::Value(double d) {
  }

  Value::Value(const std::string &str) {
  }

  Value::Value(const std::list<Value> &l) {
  }

  Value::Value(const std::vector<Value> &l) {
  }

  Value::Value(const std::map<std::string, Value> &l) {
    clear();
    _private.type = Value::Map;
    _private.data.ptr = new std::map<std::string, Value>(l);
  }

#ifdef QI_QT
  Value::Value(const QList<Value> &l);
  Value::Value(const QVector<Value> &l);
  Value::Value(const QMap<QString, Value> &l);
#endif


  bool                         Value::toBool() {
  }

  char                         Value::toChar() {
  }

  int                          Value::toInt32() {
  }

  unsigned int                 Value::toUInt32() {
  }

  long long                    Value::toInt64() {
  }

  unsigned long long           Value::toUInt64() {
  }

  float                        Value::toFloat() {
  }

  double                       Value::toDouble() {
  }

  std::string                  Value::toString() {
    if (_private.type != Value::String)
      throw ValueError("value can't be converted to string");
    return *reinterpret_cast<std::string *>(_private.data.ptr);
  }

  std::list<Value>             Value::toList() {
    if (_private.type != Value::List)
      throw ValueError("value can't be converted to list");
    return *reinterpret_cast< std::list<Value> *>(_private.data.ptr);
  }

  std::vector<Value>           Value::toVector() {

  }

  std::map<std::string, Value> Value::toMap() {
    if (_private.type != Value::Map)
      throw ValueError("value can't be converted to map");
    return *reinterpret_cast< std::map<std::string, Value> *>(_private.data.ptr);
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
  }


}

