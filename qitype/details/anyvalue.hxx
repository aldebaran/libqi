#pragma once
/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/

#ifndef _QITYPE_DETAILS_ANYVALUE_HXX_
#define _QITYPE_DETAILS_ANYVALUE_HXX_

#include <cmath>

#include <boost/type_traits/remove_const.hpp>
#include <boost/type_traits/is_floating_point.hpp>
#include <qitype/details/anyiterator.hpp>
#include <qitype/details/anyreference.hpp>

namespace qi {

  template<>
  class TypeImpl<AnyValue>: public DynamicTypeInterface
  {
  public:
    virtual AnyReference get(void* storage)
    {
      AnyValue* ptr = (AnyValue*)ptrFromStorage(&storage);
      return *ptr;
    }

    virtual void set(void** storage, AnyReference src)
    {
      AnyValue* val = (AnyValue*)ptrFromStorage(storage);
      val->reset(src, true, true);
    }

    // Default cloner will do just right since AnyValue is by-value.
    typedef DefaultTypeImplMethods<AnyValue> Methods;
    _QI_BOUNCE_TYPE_METHODS(Methods);
  };

  inline AnyValue
  AnyValue::makeTuple(const std::vector<AnyReference>& values)
  {
    return AnyValue(makeGenericTuple(values), false, true);
  }

  template<typename T>
  AnyValue AnyValue::makeList(const std::vector<AnyReference>& values)
  {
    AnyValue res = make<std::vector<T> >();
    for (unsigned i=0; i<values.size(); ++i)
      res.append(values[i].to<T>());
    return res;
  }
  inline
  AnyValue AnyValue::makeGenericList(const std::vector<AnyReference>& values)
  {
    return makeList<AnyValue>(values);
  }
  template<typename K, typename V>
  AnyValue AnyValue::makeMap(const std::map<AnyReference, AnyReference>& values)
  {
    AnyValue res = make<std::map<K, V> >();
    std::map<AnyReference, AnyReference>::const_iterator it;
    for(it = values.begin(); it != values.end(); ++it)
      res.insert(it->first.to<K>(), it->second.to<V>());
    return res;
  }

  inline
  AnyValue AnyValue::makeGenericMap(const std::map<AnyReference, AnyReference>& values)
  {
    return makeMap<AnyValue, AnyValue>(values);
  }

  inline AnyValue::AnyValue()
  : _allocated(false)
  {}


  inline AnyValue::AnyValue(const AnyValue& b)
  : _allocated(false)
  {
    *this = b;
  }

  inline AnyValue::AnyValue(qi::TypeInterface *type)
    : AnyReference(type)
    , _allocated(true)
  {
  }

  inline AnyValue::AnyValue(const AnyReference& b, bool copy, bool free)
  : _allocated(false)
  {
    reset(b, copy, free);
  }

  inline AnyValue::AnyValue(const AutoAnyReference& b)
  : _allocated(false)
  {
    reset(b);
  }

  template<typename T>
  AnyValue AnyValue::make()
  {
    return AnyValue(AnyReference(typeOf<T>()), false, true);
  }

  inline void AnyValue::operator=(const AnyValue& b)
  {
    reset(b, true, true);
  }

  inline void AnyValue::operator=(const AnyReference& b)
  {
    reset(b, true, true);
  }

  inline void AnyValue::reset(const AnyReference& b)
  {
    reset(b, true, true);
  }

  inline void AnyValue::reset(const AnyReference& b, bool copy, bool free)
  {
    reset();
    *(AnyReference*)this = b;
    _allocated = free;
    if (copy)
      *(AnyReference*)this = clone();
  }

  inline void AnyValue::reset()
  {
    if (_allocated)
      destroy();
    type = 0;
    value = 0;
  }

  inline void AnyValue::reset(qi::TypeInterface *ttype)
  {
    reset();
    _allocated = true;
    type = ttype;
    value = type->initializeStorage();
  }

  inline AnyValue::~AnyValue()
  {
    reset();
  }

  inline void AnyValue::swap(AnyValue& b)
  {
    std::swap((::qi::AnyReference&)*this, (::qi::AnyReference&)b);
    std::swap(_allocated, b._allocated);
  }


  inline bool operator != (const AnyValue& a, const AnyValue& b)
  {
    return !(a==b);
  }



  /// FutureValueConverter implementation for AnyReference -> T
  /// that destroys the value
  template <typename T>
  struct FutureValueConverterTakeAnyReference
  {
    void operator()(const AnyReference& in, T& out)
    {
      try {
        out = in.to<T>();
      }
      catch (const std::exception& e)
      {
        const_cast<AnyReference&>(in).destroy();
        throw e;
      }
      const_cast<AnyReference&>(in).destroy();
    }
  };

  /// FutureValueConverter implementation for AnyReference -> T
  /// that destroys the value
  template<>
  struct FutureValueConverterTakeAnyReference<AnyValue>
  {
    void operator()(const AnyReference& in, AnyValue& out)
    {
      out.reset(in, false, true);
    }
  };

  template <typename T1, typename T2>
  struct FutureValueConverter;

  template <typename T>
  struct FutureValueConverter<T, qi::AnyValue>
  {
    void operator()(const T& in, qi::AnyValue &out)
    {
      out = qi::AnyValue::from(in);
    }
  };

  template <>
  struct FutureValueConverter<void, qi::AnyValue>
  {
    void operator()(void *in, qi::AnyValue &out)
    {
    }
  };
}

namespace std
{
  inline void swap(::qi::AnyValue& a, ::qi::AnyValue& b)
  {
    a.swap(b);
  }
}


#endif  // _QITYPE_DETAILS_ANYVALUE_HXX_


