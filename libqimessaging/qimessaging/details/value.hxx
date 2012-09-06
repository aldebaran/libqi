/*
*  Author(s):
*  - Chris  Kilner <ckilner@aldebaran-robotics.com>
*  - Cedric Gestes <gestes@aldebaran-robotics.com>
*
*  Copyright (C) 2010-2012 Aldebaran Robotics
*/


#pragma once
#ifndef _QIMESSAGING_VALUE_HXX_
#define _QIMESSAGING_VALUE_HXX_

#include <cstring>
#include <boost/type_traits/is_integral.hpp>
#include <boost/type_traits/is_floating_point.hpp>

#include <qi/log.hpp>
namespace qi {

  namespace detail {

  template <typename T>
  inline void Value::set(const T& v)
  {
    ValueConverter<T>::writeValue(v, *this);
  }

  template<typename T>
  inline T Value::as() const
  {
    T res;
    ValueConverter<T>::readValue(*this, res);
    return res;
  }

  template<typename T> struct IntegralConverter
  {
    static inline void writeValue(const T& src, Value& dst)
    {
      dst.setDouble((double)src);
    }
    static inline void readValue(const Value& src, T& dst)
    {
      double d = src.toDouble();
      dst = d;
    }
  };

  template<typename T, typename B, int U> struct InheritIf
  {
  };

  template<typename T, int U> struct InheritIf<T, boost::true_type, U>: public T
  {
  };

  template<typename T> struct ValueConverterDefault
  : public InheritIf<IntegralConverter<T>, typename boost::is_integral<T>::type, 0 >
  , public InheritIf<IntegralConverter<T>, typename boost::is_floating_point<T>::type, 1>
  {
  };

  template<typename T> inline void ValueConverter<T>::writeValue(const T& src, Value& dst)
  {
    ValueConverterDefault<T>::writeValue(src, dst);
  }
  template<typename T> inline void ValueConverter<T>::readValue(const Value& src, T& dst)
  {
    ValueConverterDefault<T>::readValue(src, dst);
  }

  /// Container support
  template<typename T> struct ContainerValueConverter
  {
    static inline void writeValue(const T& src, Value& dst)
    {
      dst.setList(Value::ValueList());
      Value::ValueList& vl = *dst.data.list;
      for (typename T::const_iterator i = src.begin(); i!=src.end(); ++i)
      {
        vl.push_back(Value());
        ValueConverter<typename T::value_type>::writeValue(*i, vl.back());
      }
    }
    static inline void readValue(const Value& src, T& dst)
    {
      const Value::ValueList& vl = src.toList();
      for (Value::ValueList::const_iterator i = vl.begin(); i!= vl.end(); ++i)
      {
        typename T::value_type elem;
        ValueConverter<typename T::value_type>::readValue(*i, elem);
        dst.push_back(elem);
      }
    }
  };

  template<typename T> struct ValueConverter<std::vector<T> >
  : public ContainerValueConverter<std::vector<T> > {};

  template<typename T> struct ValueConverter<std::list<T> >
  : public ContainerValueConverter<std::list<T> > {};

  template<> struct ValueConverter<std::string>
  {
    static inline void writeValue(const std::string& src, Value& dst)
    {
      dst.setString(src);
    }
    static inline void readValue(const Value& src, std::string& dst)
    {
      dst = src.toString();
    }
  };

  template<> struct ValueConverter<char*>
  {
    static inline void writeValue(const char*& src, Value& dst)
    {
      dst.setString(src);
    }
    static inline void readValue(const Value& src, char*& dst)
    {
      dst = strdup(src.toString().c_str());
    }
  };

  inline Value::Value()
  : type(Invalid)
  {
    data.ptr = 0;
  }

  inline Value::Value(double d)
  : type(Double)
  {
    data.d = d;
  }

  inline Value::Value(const std::string& s)
  : type(String)
  {
    data.str = new std::string(s);
  }

  inline Value::Value(const ValueList& v)
  : type(List)
  {
    data.list = new ValueList(v);
  }

  inline Value::Value(const ValueMap& v)
  : type(Map)
  {
    data.map = new ValueMap(v);
  }

  inline Value::Value(const Value& b)
  : type(Invalid)
  {
    *this = b;
  }
  inline Value& Value::operator=(const Value& b)
  {
    switch (b.type)
    {
    case Double: setDouble(b.toDouble()); break;
    case String: setString(b.toString()); break;
    case List:   setList  (b.toList());   break;
    case Map:    setMap   (b.toMap());    break;

    case Invalid:
    case Opaque:
      type = b.type;
      data.ptr = b.data.ptr;
      break;
    }
    return *this;
  }

  inline void Value::setDouble(double d)
  {
    clear();
    type = Double;
    data.d = d;
  }

  inline void Value::setString(const std::string& s)
  {
    clear();
    type = String;
    data.str = new std::string(s);
  }

  inline void Value::setList(const ValueList& s)
  {
    clear();
    type = List;
    data.list = new ValueList(s);
  }

  inline void Value::setMap(const ValueMap& s)
  {
    clear();
    type = Map;
    data.map = new ValueMap(s);
  }


  inline double Value::toDouble() const
  {
    if (type != Double)
      qiLogError("qi.Value") << "Invalid toDouble on type " << type;
    return data.d;
  }

  inline std::string Value::toString() const
  {
    if (type != String)
    {
      qiLogError("qi.Value") << "Invalid toString on type " << type;
      return "";
    }
    return *data.str;
  }

  inline const Value::ValueList& Value::toList() const
  {
    if (type != List)
    {
      qiLogError("qi.Value") << "Invalid toList on type " << type;
      static  Value::ValueList res;
      return res;
    }
    return *data.list;
  }

  inline const Value::ValueMap& Value::toMap() const
  {
    if (type != Map)
    {
      qiLogError("qi.Value") << "Invalid toMap on type " << type;
      static Value::ValueMap res;
      return res;
    }
    return *data.map;
  }

  }
}

#endif  // _QIMESSAGING_VALUE_HXX_
