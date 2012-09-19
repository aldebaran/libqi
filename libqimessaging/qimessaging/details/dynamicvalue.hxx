/*
*  Author(s):
*  - Chris  Kilner <ckilner@aldebaran-robotics.com>
*  - Cedric Gestes <gestes@aldebaran-robotics.com>
*
*  Copyright (C) 2010-2012 Aldebaran Robotics
*/


#pragma once
#ifndef _QIMESSAGING_DETAILS_DYNAMICVALUE_HXX_
#define _QIMESSAGING_DETAILS_DYNAMICVALUE_HXX_

#include <cstring>
#include <boost/type_traits/is_integral.hpp>
#include <boost/type_traits/is_floating_point.hpp>

#include <qi/log.hpp>
namespace qi {

  namespace detail {

  template <typename T>
  inline void DynamicValue::set(const T& v)
  {
    DynamicValueConverter<T>::writeDynamicValue(v, *this);
  }

  template<typename T>
  inline T DynamicValue::as() const
  {
    T res;
    DynamicValueConverter<T>::readDynamicValue(*this, res);
    return res;
  }


  template<typename T> struct NullConverter
  {
    static inline void writeDynamicValue(const T&, DynamicValue&)
    {
      qiLogError("qi.value") << "writeDynamicValue not implemented on this type";
    }
    static inline void readDynamicValue(const DynamicValue&, T&)
    {
      qiLogError("qi.value") << "readDynamicValue not implemented on this type";
    }
  };

  template<typename T> struct IntegralConverter
  {
    static inline void writeDynamicValue(const T& src, DynamicValue& dst)
    {
      dst.setDouble((double)src);
    }
    static inline void readDynamicValue(const DynamicValue& src, T& dst)
    {
      double d = src.toDouble();
      dst = static_cast<T>(d);
    }
  };

  template<typename T, typename B, typename F> struct InheritIfElse
  {
  };

  template<typename T, typename F> struct InheritIfElse<T, boost::true_type, F>: public T
  {
  };

  template<typename T, typename F> struct InheritIfElse<T, boost::false_type, F>: public F
  {
  };

  template<typename A, typename B> struct META_OR
  {
    typedef boost::true_type type;
  };

  template<> struct META_OR<boost::false_type, boost::false_type>
  {
    typedef boost::false_type type;
  };

  template<typename T> struct DynamicValueConverterDefault
  : public InheritIfElse<
      IntegralConverter<T>,
      typename META_OR<
        typename boost::is_integral<T>::type,
        typename boost::is_floating_point<T>::type>::type,
      NullConverter<T> >
  {
  };

  template<typename T> inline void DynamicValueConverter<T>::writeDynamicValue(const T& src, DynamicValue& dst)
  {
    DynamicValueConverterDefault<T>::writeDynamicValue(src, dst);
  }
  template<typename T> inline void DynamicValueConverter<T>::readDynamicValue(const DynamicValue& src, T& dst)
  {
    DynamicValueConverterDefault<T>::readDynamicValue(src, dst);
  }

  /// Container support
  template<typename T> struct ContainerDynamicValueConverter
  {
    static inline void writeDynamicValue(const T& src, DynamicValue& dst)
    {
      dst.setList(DynamicValue::DynamicValueList());
      DynamicValue::DynamicValueList& vl = *dst.data.list;
      for (typename T::const_iterator i = src.begin(); i!=src.end(); ++i)
      {
        vl.push_back(DynamicValue());
        DynamicValueConverter<typename T::value_type>::writeDynamicValue(*i, vl.back());
      }
    }
    static inline void readDynamicValue(const DynamicValue& src, T& dst)
    {
      const DynamicValue::DynamicValueList& vl = src.toList();
      for (DynamicValue::DynamicValueList::const_iterator i = vl.begin(); i!= vl.end(); ++i)
      {
        typename T::value_type elem;
        DynamicValueConverter<typename T::value_type>::readDynamicValue(*i, elem);
        dst.push_back(elem);
      }
    }
  };

  template<typename T> struct DynamicValueConverter<std::vector<T> >
  : public ContainerDynamicValueConverter<std::vector<T> > {};

  template<typename T> struct DynamicValueConverter<std::list<T> >
  : public ContainerDynamicValueConverter<std::list<T> > {};

  template<> struct DynamicValueConverter<std::string>
  {
    static inline void writeDynamicValue(const std::string& src, DynamicValue& dst)
    {
      dst.setString(src);
    }
    static inline void readDynamicValue(const DynamicValue& src, std::string& dst)
    {
      dst = src.toString();
    }
  };

  template<> struct DynamicValueConverter<char*>
  {
    static inline void writeDynamicValue(const char*& src, DynamicValue& dst)
    {
      dst.setString(src);
    }
    static inline void readDynamicValue(const DynamicValue& src, char*& dst)
    {
      dst = strdup(src.toString().c_str());
    }
  };

  inline DynamicValue::DynamicValue()
  : type(Invalid)
  {
    data.ptr = 0;
  }

  inline DynamicValue::DynamicValue(double d)
  : type(Double)
  {
    data.d = d;
  }

  inline DynamicValue::DynamicValue(const std::string& s)
  : type(String)
  {
    data.str = new std::string(s);
  }

  inline DynamicValue::DynamicValue(const DynamicValueList& v)
  : type(List)
  {
    data.list = new DynamicValueList(v);
  }

  inline DynamicValue::DynamicValue(const DynamicValueMap& v)
  : type(Map)
  {
    data.map = new DynamicValueMap(v);
  }

  inline DynamicValue::DynamicValue(const DynamicValue& b)
  : type(Invalid)
  {
    *this = b;
  }
  inline DynamicValue& DynamicValue::operator=(const DynamicValue& b)
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

  inline void DynamicValue::setDouble(double d)
  {
    clear();
    type = Double;
    data.d = d;
  }

  inline void DynamicValue::setString(const std::string& s)
  {
    clear();
    type = String;
    data.str = new std::string(s);
  }

  inline void DynamicValue::setList(const DynamicValueList& s)
  {
    clear();
    type = List;
    data.list = new DynamicValueList(s);
  }

  inline void DynamicValue::setMap(const DynamicValueMap& s)
  {
    clear();
    type = Map;
    data.map = new DynamicValueMap(s);
  }


  inline double DynamicValue::toDouble() const
  {
    if (type != Double)
      qiLogError("qi.DynamicValue") << "Invalid toDouble on type " << type;
    return data.d;
  }

  inline std::string DynamicValue::toString() const
  {
    if (type != String)
    {
      qiLogError("qi.DynamicValue") << "Invalid toString on type " << type;
      return "";
    }
    return *data.str;
  }

  inline const DynamicValue::DynamicValueList& DynamicValue::toList() const
  {
    if (type != List)
    {
      qiLogError("qi.DynamicValue") << "Invalid toList on type " << type;
      static  DynamicValue::DynamicValueList res;
      return res;
    }
    return *data.list;
  }

  inline const DynamicValue::DynamicValueMap& DynamicValue::toMap() const
  {
    if (type != Map)
    {
      qiLogError("qi.DynamicValue") << "Invalid toMap on type " << type;
      static DynamicValue::DynamicValueMap res;
      return res;
    }
    return *data.map;
  }

  }
}

#endif  // _QIMESSAGING_DETAILS_DYNAMICVALUE_HXX_
