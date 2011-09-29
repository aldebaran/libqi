#pragma once
/*
*  Author(s):
*  - Chris  Kilner <ckilner@aldebaran-robotics.com>
*  - Cedric Gestes <gestes@aldebaran-robotics.com>
*
*  Copyright (C) 2010, 2011 Aldebaran Robotics
*/


#ifndef _QI_VALUE_HPP_
#define _QI_VALUE_HPP_

#include <iostream>
#include <qimessaging/api.hpp>
//#include <qimessaging/signature.hpp>

namespace qi {

  class QIMESSAGING_API Value {
  public:
    enum Type {
      Invalid,
      Bool,
      Char,
      Int32,
      UInt32,
      Int64,
      UInt64,
      Float,
      Double,
      String,
      List,
      Map
    };

    Value();
    Value(Type t);
    Value(bool b);
    Value(char c);
    Value(int i);
    Value(unsigned int i);
    Value(long long l);
    Value(unsigned long long l);
    Value(float f);
    Value(double d);
    Value(const std::string &str);
    Value(const std::list<Value> &l);
    Value(const std::vector<Value> &l);
    Value(const std::map<std::string, Value> &l);
#ifdef QI_QT
    Value(const QList<Value> &l);
    Value(const QVector<Value> &l);
    Value(const QMap<QString, Value> &l);
#endif


    bool                         toBool();
    char                         toChar();
    int                          toInt32();
    unsigned int                 toUInt32();
    long long                    toInt64();
    unsigned long long           toUInt64();
    float                        toFloat();
    double                       toDouble();
    std::string                  toString();
    std::list<Value>             toList();
    std::vector<Value>           toVector();
    std::map<std::string, Value> toMap();
#ifdef QI_QT
    QString                      toQString();
    QList<Value>                 toQList();
    QVector<Value>               toQVector();
    QMap<QString, Value>         toQMap();
#endif

    static Value fromMessage(const qi::Message &msg);

    void clear();
    Type type() const;

    template <typename T>
    void setValue(const T &value);

    template <typename T>
    T value() const;

    struct ValuePrivate {
      union Data {
        bool               b;
        int                i;
        unsigned int       ui;
        long long          l;
        unsigned long long ul;
        float              f;
        double             d;
        void              *ptr;
      };
      unsigned int type;
    };

  private:
    ValuePrivate _private;
  };


  typedef std::list<Value>             ValueList;
  typedef std::map<std::string, Value> ValueMap;

};

#endif  // _QI_SERIALIZATION_MESSAGE_HPP_
