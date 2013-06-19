#pragma once
/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/

#ifndef _QITYPE_DETAILS_GENERICVALUE_HXX_
#define _QITYPE_DETAILS_GENERICVALUE_HXX_

#include <cmath>

#include <boost/type_traits/remove_const.hpp>
#include <boost/type_traits/is_floating_point.hpp>

namespace qi {



  template<> class TypeImpl<GenericValue>: public DynamicTypeInterface
  {
  public:
    virtual AnyReference get(void* storage)
    {
      GenericValue* ptr = (GenericValue*)ptrFromStorage(&storage);
      return *ptr;
    }
    virtual void set(void** storage, AnyReference src)
    {
      GenericValue* val = (GenericValue*)ptrFromStorage(storage);
      val->reset(src, true, true);
    }
    // Default cloner will do just right since GenericValue is by-value.
    typedef DefaultTypeImplMethods<GenericValue> Methods;
    _QI_BOUNCE_TYPE_METHODS(Methods);
  };

  namespace detail
  {
    template<typename T> struct TypeCopy
    {
      void operator()(T &dst, const T &src)
      {
        dst = src;
      }
    };

    template<int I> struct TypeCopy<char[I] >
    {
      void operator() (char* dst, const char* src)
      {
        memcpy(dst, src, I);
      }
    };
  }

  inline
  AnyReference& AnyReference::operator = (const AutoAnyReference& b)
  {
    update(b);
    return *this;
  }

  inline
  AnyReference AnyReference::clone() const
  {
    AnyReference res;
    res.type = type;
    res.value = type?res.type->clone(value):0;
    return res;
  }

  inline AutoAnyReference::AutoAnyReference(const AutoAnyReference& b)
  {
    value = b.value;
    type = b.type;
  }

  template<typename T> AutoAnyReference::AutoAnyReference(const T& ptr)
  {
    *(AnyReference*)this = AnyReference(ptr);
  }

  inline AutoAnyReference::AutoAnyReference()
  {
    value = type = 0;
  }

  inline qi::Signature AnyReference::signature(bool resolveDynamic) const
  {
    if (!type)
      return qi::Signature();
    else
      return type->signature(value, resolveDynamic);
  }

  inline void AnyReference::destroy()
  {
    if (type)
      type->destroy(value);
    value = type = 0;
  }

  inline AnyReference::AnyReference()
    : type(0)
    , value(0)
  {
  }

  inline AnyReference::AnyReference(TypeInterface* type)
    : type(type)
    , value(type->initializeStorage())
  {
  }

  template<typename T>
  AnyReference AnyReference::fromPtr(const T* ptr)
  {
    static TypeInterface* t = 0;
    if (!t)
      t = typeOf<typename boost::remove_const<T>::type>();
    void *value = t->initializeStorage(const_cast<void*>((const void*)ptr));
    return AnyReference(t, value);
  }

  template<typename T>
  AnyReference::AnyReference(const T& ptr)
  {
    static TypeInterface* t = 0;
    if (!t)
      t = typeOf<typename boost::remove_const<T>::type>();
    type = t;
    value = type->initializeStorage(const_cast<void*>((const void*)&ptr));
  }

  inline TypeInterface::Kind AnyReference::kind() const
  {
    if (!type)
      return TypeInterface::Void;
    else
      return type->kind();
  }


  // Kind -> handler Type (IntTypeInterface, ListTypeInterface...)  accessor

  class KindNotConvertible;

  template<TypeInterface::Kind T> struct TypeOfKind
  {
    typedef KindNotConvertible type;
  };

#define TYPE_OF_KIND(k, t) template<> struct TypeOfKind<k> { typedef t type;}


  TYPE_OF_KIND(TypeInterface::Int, IntTypeInterface);
  TYPE_OF_KIND(TypeInterface::Float,  FloatTypeInterface);
  TYPE_OF_KIND(TypeInterface::String, StringTypeInterface);

#undef TYPE_OF_KIND


  // Type -> Kind  accessor

  namespace detail
  {
    struct Nothing {};
    template<TypeInterface::Kind k> struct MakeKind
    {
      static const TypeInterface::Kind value = k;
    };

    template<typename C, typename T, typename F> struct IfElse
    {
    };

    template<typename T, typename F> struct IfElse<boost::true_type, T, F>
    {
      typedef T type;
    };

    template<typename T, typename F> struct IfElse<boost::false_type, T, F>
    {
      typedef F type;
    };

  }

#define IF(cond, type) \
  public detail::IfElse<cond, typename detail::MakeKind<type>, detail::Nothing>::type

  template<typename T> struct KindOfType
      : IF(typename boost::is_integral<T>::type, TypeInterface::Int)
  , IF(typename boost::is_floating_point<T>::type, TypeInterface::Float)
  {
  };

#undef IF

  namespace detail {

    // Optimized AnyReference::as<T> for direct access to a subType getter
    template<typename T, TypeInterface::Kind k>
    inline T valueAs(const AnyReference& v)
    {
      if (v.kind() == k)
        return static_cast<T>(
          static_cast<typename TypeOfKind<k>::type* const>(v.type)->get(v.value));
      // Fallback to default which will attempt a full conversion.
      return v.to<T>();
    }
  }

  template<typename T>
  inline T* AnyReference::ptr(bool check)
  {
    if (!type || (check && typeOf<T>()->info() != type->info()))
      return 0;
    else
      return (T*)type->ptrFromStorage(&value);
  }

  template<typename T>
  inline T& AnyReference::as()
  {
    T* p = ptr<T>(true);
    if (!p)
      throw std::runtime_error("Type mismatch");
    return *p;
  }

  template<typename T>
  inline T AnyReference::to(const T&) const
  {
    return to<T>();
  }

  namespace detail
  {
    QI_NORETURN  QITYPE_API void throwConversionFailure(TypeInterface* from, TypeInterface* to);
  }

  template<typename T>
  inline T AnyReference::to() const
  {
    TypeInterface* targetType = typeOf<T>();
    std::pair<AnyReference, bool> conv = convert(targetType);
    if (!conv.first.type)
    {
      detail::throwConversionFailure(type, targetType);
    }
    T result = *conv.first.ptr<T>(false);
    if (conv.second)
      conv.first.destroy();
    return result;
  }

  inline bool    AnyReference::isValid() const {
    return type != 0;
  }

  inline bool    AnyReference::isValue() const {
    return type != 0 && type->info() != typeOf<void>()->info();
  }

  inline int64_t AnyReference::toInt() const
  {
    return detail::valueAs<int64_t, TypeInterface::Int>(*this);
  }

  inline uint64_t AnyReference::toUInt() const
  {
    return detail::valueAs<uint64_t, TypeInterface::Int>(*this);
  }

  inline float AnyReference::toFloat() const
  {
    return detail::valueAs<float, TypeInterface::Float>(*this);
  }

  inline double AnyReference::toDouble() const
  {
    return detail::valueAs<double, TypeInterface::Float>(*this);
  }


  inline std::string AnyReference::toString() const
  {
    return to<std::string>();
  }

  template<typename T>
  std::vector<T>
  AnyReference::toList() const
  {
    return to<std::vector<T> >();
  }

  template<typename K, typename V>
  std::map<K, V>
  AnyReference::toMap() const
  {
    return to<std::map<K, V> >();
  }

  inline std::vector<AnyReference>
  AnyReference::asTupleValuePtr()
  {
    if (kind() == TypeInterface::Tuple)
      return static_cast<StructTypeInterface*>(type)->values(value);
    else if (kind() == TypeInterface::List || kind() == TypeInterface::Map)
    {
      std::vector<AnyReference> result;
      AnyIterator iend = end();
      AnyIterator it = begin();
      for(; it != iend; ++it)
        result.push_back(*it);
      return result;
    }
    else
      throw std::runtime_error("Expected tuple, list or map");
  }

  inline std::vector<AnyReference>
  AnyReference::asListValuePtr()
  {
    return asTupleValuePtr();
  }

  inline std::map<AnyReference, AnyReference>
  AnyReference::asMapValuePtr()
  {
    if (kind() != TypeInterface::Map)
      throw std::runtime_error("Expected a map");
    std::map<AnyReference, AnyReference> result;
    AnyIterator iend = end();
    AnyIterator it = begin();
    for(; it != iend; ++it)
    {
      AnyReference elem = *it;
      result[elem[0]] = elem[1];
    }
    return result;
  }

  inline GenericValue
  GenericValue::makeTuple(const std::vector<AnyReference>& values)
  {
    return GenericValue(makeGenericTuple(values), false, true);
  }

  template<typename T>
  GenericValue GenericValue::makeList(const std::vector<AnyReference>& values)
  {
    GenericValue res = make<std::vector<T> >();
    for (unsigned i=0; i<values.size(); ++i)
      res.append(values[i].to<T>());
    return res;
  }
  inline
  GenericValue GenericValue::makeGenericList(const std::vector<AnyReference>& values)
  {
    return makeList<GenericValue>(values);
  }
  template<typename K, typename V>
  GenericValue GenericValue::makeMap(const std::map<AnyReference, AnyReference>& values)
  {
    GenericValue res = make<std::map<K, V> >();
    std::map<AnyReference, AnyReference>::const_iterator it;
    for(it = values.begin(); it != values.end(); ++it)
      res.insert(it->first.to<K>(), it->second.to<V>());
    return res;
  }

  inline
  GenericValue GenericValue::makeGenericMap(const std::map<AnyReference, AnyReference>& values)
  {
    return makeMap<GenericValue, GenericValue>(values);
  }

  namespace detail
  {
    /** This class can be used to convert the return value of an arbitrary function
  * into a AnyReference. It handles functions returning void.
  *
  *  Usage:
  *    ValueCopy val;
  *    val(), functionCall(arg);
  *
  *  in val(), parenthesis are useful to avoid compiler warning "val not used" when handling void.
  */
    class AnyReferenceCopy: public AnyReference
    {
    public:
      AnyReferenceCopy &operator()() { return *this; }
    };

    template<typename T> void operator,(AnyReferenceCopy& g, const T& any)
    {
      *(AnyReference*)&g = AnyReference(any).clone();
    }
  }

  inline GenericValue::GenericValue()
  : _allocated(false)
  {}


  inline GenericValue::GenericValue(const GenericValue& b)
  : _allocated(false)
  {
    *this = b;
  }

  inline GenericValue::GenericValue(qi::TypeInterface *type)
    : AnyReference(type)
    , _allocated(true)
  {
  }

  inline GenericValue::GenericValue(const AnyReference& b, bool copy, bool free)
  : _allocated(false)
  {
    reset(b, copy, free);
  }

  inline GenericValue::GenericValue(const AutoAnyReference& b)
  : _allocated(false)
  {
    reset(b);
  }

  template<typename T>
  GenericValue GenericValue::make()
  {
    return GenericValue(AnyReference(typeOf<T>()), false, true);
  }

  inline void GenericValue::operator=(const GenericValue& b)
  {
    reset(b, true, true);
  }

  inline void GenericValue::operator=(const AnyReference& b)
  {
    reset(b, true, true);
  }

  inline void GenericValue::reset(const AnyReference& b)
  {
    reset(b, true, true);
  }

  inline void GenericValue::reset(const AnyReference& b, bool copy, bool free)
  {
    reset();
    *(AnyReference*)this = b;
    _allocated = free;
    if (copy)
      *(AnyReference*)this = clone();
  }

  inline void GenericValue::reset()
  {
    if (_allocated)
      destroy();
    type = 0;
    value = 0;
  }

  inline void GenericValue::reset(qi::TypeInterface *ttype)
  {
    reset();
    _allocated = true;
    type = ttype;
    value = type->initializeStorage();
  }

  inline GenericValue::~GenericValue()
  {
    reset();
  }

  template<typename T>
  void AnyReference::set(const T& v)
  {
    update(AnyReference(v));
   }

  inline void AnyReference::setFloat(float v)
  {
    setDouble(static_cast<double>(v));
  }

  inline void AnyReference::setString(const std::string& v)
  {
    if (kind() != TypeInterface::String)
      throw std::runtime_error("Value is not of kind string");
    static_cast<StringTypeInterface*>(type)->set(&value, &v[0], v.size());
  }

  template<typename E, typename K>
  E& AnyReference::element(const K& key)
  {
    return (*this)[key].template as<E>();
  }

  template<typename K>
  AnyReference AnyReference::operator[](const K& key)
  {
    return _element(AnyReference(key), true);
  }

  inline size_t
  AnyReference::size() const
  {
    if (kind() == TypeInterface::List)
      return static_cast<ListTypeInterface*>(type)->size(value);
    if (kind() == TypeInterface::Map)
      return static_cast<MapTypeInterface*>(type)->size(value);
    if (kind() == TypeInterface::Tuple)
      return static_cast<StructTypeInterface*>(type)->memberTypes().size();
    else
      throw std::runtime_error("Expected List, Map or Tuple.");
  }

  template<typename T> void AnyReference::append(const T& element)
  {
    _append(AnyReference(element));
  }

  template<typename K, typename V>
  void AnyReference::insert(const K& key, const V& val)
  {
    _insert(AnyReference(key), AnyReference(val));
  }

  template<typename K>
  AnyReference AnyReference::find(const K& key)
  {
    return _element(AnyReference(key), false);
  }

  inline AnyReference AnyReference::asDynamic() const
  {
    if (kind() != TypeInterface::Dynamic)
      throw std::runtime_error("Not of dynamic kind");
    DynamicTypeInterface* d = static_cast<DynamicTypeInterface*>(type);
    return d->get(value);
  }


  inline void GenericValue::swap(GenericValue& b)
  {
    std::swap((::qi::AnyReference&)*this, (::qi::AnyReference&)b);
    std::swap(_allocated, b._allocated);
  }

  inline AnyReference AnyReference::operator*()
  {
    if (kind() == TypeInterface::Pointer)
      return static_cast<PointerTypeInterface*>(type)->dereference(value);
    else if (kind() == TypeInterface::Iterator)
      return static_cast<IteratorTypeInterface*>(type)->dereference(value);
    else
      throw std::runtime_error("Expected pointer or iterator");
  }

  inline AnyReference AnyIterator::operator*()
  {
    if (kind() == TypeInterface::Iterator)
      return static_cast<IteratorTypeInterface*>(type)->dereference(value);
    else
      throw std::runtime_error("Expected iterator");
  }

  template<typename T>
  AnyIterator::AnyIterator(const T& ref)
  : GenericValue(AnyReference(ref))
  {

  }
  inline AnyIterator::AnyIterator()
  {
  }

  inline AnyIterator::AnyIterator(const AnyReference& p)
    : GenericValue(p)
  {}

  inline AnyIterator::AnyIterator(const GenericValue& v)
    : GenericValue(v)
  {}

  inline AnyIterator
  AnyIterator::operator++()
  {
    if (kind() != TypeInterface::Iterator)
      throw std::runtime_error("Expected an iterator");
    static_cast<IteratorTypeInterface*>(type)->next(&value);
    return *this;
  }

  inline AnyIterator
  AnyReference::begin() const
  {
    if (kind() == TypeInterface::List)
      return static_cast<ListTypeInterface*>(type)->begin(value);
    else if (kind() == TypeInterface::Map)
      return static_cast<MapTypeInterface*>(type)->begin(value);
    else
      throw std::runtime_error("Expected list or map");
  }

  inline AnyIterator
  AnyReference::end() const
  {
    if (kind() == TypeInterface::List)
      return static_cast<ListTypeInterface*>(type)->end(value);
    else if (kind() == TypeInterface::Map)
      return static_cast<MapTypeInterface*>(type)->end(value);
    else
      throw std::runtime_error("Expected list or map");
  }

  inline bool operator != (const AnyReference& a, const AnyReference& b)
  {
    return !(a==b);
  }
  inline bool operator != (const GenericValue& a, const GenericValue& b)
  {
    return !(a==b);
  }
  inline bool operator != (const AnyIterator& a, const AnyIterator& b)
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
  template<> struct FutureValueConverterTakeAnyReference<GenericValue>
  {
    void operator()(const AnyReference& in, GenericValue& out)
    {
      out.reset(in, false, true);
    }
  };

  template <typename T1, typename T2>
  struct FutureValueConverter;

  template <typename T>
  struct FutureValueConverter<T, qi::GenericValue>
  {
    void operator()(const T& in, qi::GenericValue &out)
    {
      out = qi::GenericValue::from(in);
    }
  };

  template <>
  struct FutureValueConverter<void, qi::GenericValue>
  {
    void operator()(void *in, qi::GenericValue &out)
    {
    }
  };
}

namespace std
{
  inline void swap(::qi::GenericValue& a, ::qi::GenericValue& b)
  {
    a.swap(b);
  }
}

/* Since AnyReference does not handle its memory, it cannot be used
* inside a AnyReference. use GenericValue instead.
*/
QI_NO_TYPE(qi::AnyReference);

#endif  // _QITYPE_DETAILS_GENERICVALUE_HXX_
