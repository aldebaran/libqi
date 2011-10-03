#pragma once
/*
*  Author(s):
*  - Chris  Kilner <ckilner@aldebaran-robotics.com>
*  - Cedric Gestes <gestes@aldebaran-robotics.com>
*
*  Copyright (C) 2010, 2011 Aldebaran Robotics
*/


#ifndef _QIMESSAGING_VALUE_HPP_
#define _QIMESSAGING_VALUE_HPP_

#include <qimessaging/api.hpp>

#include <stdexcept>
#include <string>
#include <list>
#include <vector>
#include <map>

namespace qi {

  /** \class ValueError value.hpp "qi/value.hpp"
   *  \brief Thrown when an operation on a value fail
   */
  class QIMESSAGING_API ValueError : public std::runtime_error
  {
  public:
    /**
     * \brief Constructor
     * Create a message exception.
     * \param message Exception message.
     */
    explicit ValueError(const std::string &message)
      : std::runtime_error(message)
    {}

    /** \brief Destructor */
    virtual ~ValueError() throw()
    {}
  };


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
      Vector,
      Map,
      QString,
      QList,
      QVector,
      QMap
    };

    Value();
    explicit Value(Type t);
    Value(bool b);
    Value(char c);
    Value(int i);
    Value(unsigned int i);
    Value(long long l);
    Value(unsigned long long l);
    Value(float f);
    Value(double d);
    Value(const char *str);
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

    void clear();
    Type type() const;

    template <typename T>
    inline void setValue(const T &value);

    template <typename T>
    inline T &value();

    struct ValuePrivate {
      union {
        bool               b;
        char               c;
        int                i;
        unsigned int       ui;
        long long          l;
        unsigned long long ul;
        float              f;
        double             d;
        void              *ptr;
      } data;
      unsigned int type;
    };

  public:
    ValuePrivate _private;
  };


  typedef std::list<Value>             ValueList;
  typedef std::vector<Value>           ValueVector;
  typedef std::map<std::string, Value> ValueMap;



};

#include <qimessaging/value.hxx>

#endif  // _QIMESSAGING_VALUE_HPP_
