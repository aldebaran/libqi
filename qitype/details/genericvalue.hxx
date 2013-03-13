#pragma once
/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/

#ifndef _QITYPE_DETAILS_GENERICVALUE_HXX_
#define _QITYPE_DETAILS_GENERICVALUE_HXX_

#include <boost/type_traits/remove_const.hpp>
#include <boost/type_traits/is_floating_point.hpp>

namespace qi {



  template<> class TypeImpl<GenericValue>: public TypeDynamic
  {
  public:
    virtual GenericValuePtr get(void* storage)
    {
      GenericValue* ptr = (GenericValue*)ptrFromStorage(&storage);
      return *ptr;
    }
    virtual void set(void** storage, GenericValuePtr src)
    {
      GenericValue* val = (GenericValue*)ptrFromStorage(storage);
      val->reset();
      val->set(src, true, true);
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

  template<typename T>
  GenericValuePtr GenericValuePtr::make(T* v)
  {
    static Type* type = 0;
    if (!type)
      type = typeOf<typename boost::remove_const<T>::type>();
    GenericValuePtr res;
    res.type = type;
    res.value = res.type->initializeStorage(const_cast<void*>((const void*)v));
    return res;
  }

  inline
  GenericValuePtr& GenericValuePtr::operator = (const GenericValuePtr& b)
  {
    type = b.type;
    value = b.value;
    return *this;
  }

  inline
  GenericValuePtr GenericValuePtr::clone() const
  {
    GenericValuePtr res;
    res.type = type;
    res.value = type?res.type->clone(value):0;
    return res;
  }

  inline AutoGenericValuePtr::AutoGenericValuePtr(const AutoGenericValuePtr& b)
  {
    value = b.value;
    type = b.type;
  }

  template<typename T> AutoGenericValuePtr::AutoGenericValuePtr(const T& ptr)
  {
    *(GenericValuePtr*)this = GenericValuePtr(&ptr);
  }

  inline AutoGenericValuePtr::AutoGenericValuePtr()
  {
    value = type = 0;
  }

  inline std::string GenericValuePtr::signature(bool resolveDynamic) const
  {
    if (!type)
      return "";
    else
      return type->signature(value, resolveDynamic);
  }

  inline void GenericValuePtr::destroy()
  {
    if (type)
      type->destroy(value);
    value = type = 0;
  }

  inline GenericValuePtr::GenericValuePtr()
    : type(0)
    , value(0)
  {
  }

  inline GenericValuePtr::GenericValuePtr(Type* type)
    : type(type)
    , value(type->initializeStorage())
  {
  }

  template<typename T>
  GenericValuePtr::GenericValuePtr(T* ptr)
  {
    static Type* t = 0;
    if (!t)
      t = typeOf<typename boost::remove_const<T>::type>();
    type = t;
    value = type->initializeStorage(const_cast<void*>((const void*)ptr));
  }

  inline Type::Kind GenericValuePtr::kind() const
  {
    if (!type)
      return Type::Void;
    else
      return type->kind();
  }


  // Kind -> handler Type (TypeInt, TypeList...)  accessor

  class KindNotConvertible;

  template<Type::Kind T> struct TypeOfKind
  {
    typedef KindNotConvertible type;
  };

#define TYPE_OF_KIND(k, t) template<> struct TypeOfKind<k> { typedef t type;}


  TYPE_OF_KIND(Type::Int, TypeInt);
  TYPE_OF_KIND(Type::Float,  TypeFloat);
  TYPE_OF_KIND(Type::String, TypeString);

#undef TYPE_OF_KIND


  // Type -> Kind  accessor

  namespace detail
  {
    struct Nothing {};
    template<Type::Kind k> struct MakeKind
    {
      static const Type::Kind value = k;
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
      : IF(typename boost::is_integral<T>::type, Type::Int)
  , IF(typename boost::is_floating_point<T>::type, Type::Float)
  {
  };

#undef IF

  namespace detail {

    // Optimized GenericValuePtr::as<T> for direct access to a subType getter
    template<typename T, Type::Kind k>
    inline T valueAs(const GenericValuePtr& v)
    {
      if (v.kind() == k)
        return static_cast<T>(
          static_cast<typename TypeOfKind<k>::type* const>(v.type)->get(v.value));
      // Fallback to default which will attempt a full conversion.
      return v.to<T>();
    }
  }

  template<typename T>
  inline T* GenericValuePtr::ptr(bool check)
  {
    if (check && typeOf<T>()->info() != type->info())
      return 0;
    else
      return (T*)type->ptrFromStorage(&value);
  }

  template<typename T>
  inline T& GenericValuePtr::as()
  {
    T* p = ptr<T>(true);
    if (!p)
      throw std::runtime_error("Type mismatch");
    return *p;
  }

  template<typename T>
  inline T GenericValuePtr::to(const T&) const
  {
    return to<T>();
  }

  template<typename T>
  inline T GenericValuePtr::to() const
  {
    qiLogCategory("qitype.genericvalue");
    std::pair<GenericValuePtr, bool> conv = convert(typeOf<T>());
    if (!conv.first.type)
    {
      std::stringstream msg;
      msg << "Conversion from " << type->infoString()
          << '(' << type->kind() << ')'
          << " to " << typeOf<T>()->infoString()
          << '(' << typeOf<T>()->kind() << ") failed";
      qiLogWarning() << msg.str();
      throw std::runtime_error(msg.str());
    }
    T result = *conv.first.ptr<T>(false);
    if (conv.second)
      conv.first.destroy();
    return result;
  }

  inline int64_t GenericValuePtr::toInt() const
  {
    return detail::valueAs<int64_t, Type::Int>(*this);
  }

  inline uint64_t GenericValuePtr::toUInt() const
  {
    return detail::valueAs<uint64_t, Type::Int>(*this);
  }

  inline float GenericValuePtr::toFloat() const
  {
    return detail::valueAs<float, Type::Float>(*this);
  }

  inline double GenericValuePtr::toDouble() const
  {
    return detail::valueAs<double, Type::Float>(*this);
  }


  inline std::string GenericValuePtr::toString() const
  {
    return to<std::string>();
  }

  template<typename T>
  std::vector<T>
  GenericValuePtr::toList() const
  {
    return to<std::vector<T> >();
  }

  template<typename K, typename V>
  std::map<K, V>
  GenericValuePtr::toMap() const
  {
    return to<std::map<K, V> >();
  }

  inline std::vector<GenericValuePtr>
  GenericValuePtr::asTupleValuePtr()
  {
    if (kind() == Type::Tuple)
      return static_cast<TypeTuple*>(type)->getValues(value);
    else if (kind() == Type::List || kind() == Type::Map)
    {
      std::vector<GenericValuePtr> result;
      GenericIterator iend = end();
      GenericIterator it = begin();
      for(; it != iend; ++it)
        result.push_back(*it);
      return result;
    }
    else
      throw std::runtime_error("Expected tuple, list or map");
  }

  inline std::map<GenericValuePtr, GenericValuePtr>
  GenericValuePtr::asMapValuePtr()
  {
    if (kind() != Type::Map)
      throw std::runtime_error("Expected a map");
    std::map<GenericValuePtr, GenericValuePtr> result;
    GenericIterator iend = end();
    GenericIterator it = begin();
    for(; it != iend; ++it)
    {
      GenericValuePtr elem = *it;
      result[elem[0]] = elem[1];
    }
    return result;
  }

  inline GenericValue
  GenericValue::makeTuple(const std::vector<GenericValuePtr>& values)
  {
    return GenericValue(makeGenericTuple(values), false, true);
  }

  template<typename T>
  GenericValue GenericValue::makeList(const std::vector<GenericValuePtr>& values)
  {
    GenericValue res = make<std::vector<T> >();
    for (unsigned i=0; i<values.size(); ++i)
      res.append(values[i].to<T>());
    return res;
  }
  inline
  GenericValue GenericValue::makeGenericList(const std::vector<GenericValuePtr>& values)
  {
    return makeList<GenericValue>(values);
  }
  template<typename K, typename V>
  GenericValue GenericValue::makeMap(const std::map<GenericValuePtr, GenericValuePtr>& values)
  {
    GenericValue res = make<std::map<K, V> >();
    std::map<GenericValuePtr, GenericValuePtr>::const_iterator it;
    for(it = values.begin(); it != values.end(); ++it)
      res.insert(it->first.to<K>(), it->second.to<V>());
    return res;
  }

  inline
  GenericValue GenericValue::makeGenericMap(const std::map<GenericValuePtr, GenericValuePtr>& values)
  {
    return makeMap<GenericValue, GenericValue>(values);
  }

  namespace detail
  {
    /** This class can be used to convert the return value of an arbitrary function
  * into a GenericValuePtr. It handles functions returning void.
  *
  *  Usage:
  *    ValueCopy val;
  *    val(), functionCall(arg);
  *
  *  in val(), parenthesis are useful to avoid compiler warning "val not used" when handling void.
  */
    class GenericValuePtrCopy: public GenericValuePtr
    {
    public:
      GenericValuePtrCopy &operator()() { return *this; }
    };

    template<typename T> void operator,(GenericValuePtrCopy& g, const T& any)
    {
      *(GenericValuePtr*)&g = GenericValuePtr::make(&any);
      *(GenericValuePtr*)&g = g.clone();
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

  inline GenericValue::GenericValue(qi::Type *type)
    : GenericValuePtr(type)
    , _allocated(true)
  {
  }

  inline GenericValue::GenericValue(const GenericValuePtr& b, bool copy, bool free)
  : _allocated(false)
  {
   set(b, copy, free);
  }

  inline GenericValue::GenericValue(const GenericValuePtr& b)
  : _allocated(false)
  {
   set(b);
  }

  template<typename T>
  GenericValue GenericValue::make()
  {
    return GenericValue(GenericValuePtr(typeOf<T>()), false, true);
  }

  inline void GenericValue::operator=(const GenericValue& b)
  {
    set(b, true, true);
  }

  inline void GenericValue::operator=(const GenericValuePtr& b)
  {
    set(b, true, true);
  }

  inline void GenericValue::set(const GenericValuePtr& b)
  {
    set(b, true, true);
  }

  inline void GenericValue::set(const GenericValuePtr& b, bool copy, bool free)
  {
    reset();
    *(GenericValuePtr*)this = b;
    _allocated = free;
    if (copy)
      *(GenericValuePtr*)this = clone();
  }

  inline void GenericValue::reset()
  {
    if (_allocated)
      destroy();
    type = 0;
    value = 0;
  }

  inline void GenericValue::reset(qi::Type *ttype)
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
  void GenericValuePtr::set(const T& v)
  {
    update(GenericValuePtr(&v));
   }

  inline void GenericValuePtr::setInt(int64_t val)
  {
    if (kind() == Type::Int)
      static_cast<TypeInt*>(type)->set(&value, val);
    else if (kind() == Type::Float)
      static_cast<TypeFloat*>(type)->set(&value, static_cast<double>(val));
    else
      throw std::runtime_error("Value is not Int or Float");
  }

  inline void GenericValuePtr::setUInt(uint64_t val)
  {
    setInt((int64_t)val);
  }

  inline void GenericValuePtr::setDouble(double v)
  {
    if (kind() == Type::Float)
      static_cast<TypeFloat*>(type)->set(&value, v);
    else if (kind() == Type::Int)
      static_cast<TypeInt*>(type)->set(&value, static_cast<int64_t>(v));
    else
      throw std::runtime_error("Value is not Int or Float");
  }

  inline void GenericValuePtr::setFloat(float v)
  {
    setDouble(static_cast<double>(v));
  }

  inline void GenericValuePtr::setString(const std::string& v)
  {
    if (kind() != Type::String)
      throw std::runtime_error("Value is not of kind string");
    static_cast<TypeString*>(type)->set(&value, &v[0], v.size());
  }

  template<typename E, typename K>
  E& GenericValuePtr::element(const K& key)
  {
    return (*this)[key].template as<E>();
  }

  template<typename K>
  GenericValueRef GenericValuePtr::operator[](const K& key)
  {
    return _element(make(&key), true);
  }

  inline size_t
  GenericValuePtr::size() const
  {
    if (kind() == Type::List)
      return static_cast<TypeList*>(type)->size(value);
    if (kind() == Type::Map)
      return static_cast<TypeMap*>(type)->size(value);
    else
      throw std::runtime_error("Expected List or Map.");
  }

  template<typename T> void GenericValuePtr::append(const T& element)
  {
    _append(make(&element));
  }

  template<typename K, typename V>
  void GenericValuePtr::insert(const K& key, const V& val)
  {
    _insert(make(&key), make(&val));
  }

  template<typename K>
  GenericValuePtr GenericValuePtr::find(const K& key)
  {
    return _element(make(&key), false);
  }

  inline GenericValuePtr GenericValuePtr::asDynamic() const
  {
    if (kind() != Type::Dynamic)
      throw std::runtime_error("Not of dynamic kind");
    TypeDynamic* d = static_cast<TypeDynamic*>(type);
    return d->get(value);
  }


  inline void GenericValue::swap(GenericValue& b)
  {
    std::swap((::qi::GenericValuePtr&)*this, (::qi::GenericValuePtr&)b);
    std::swap(_allocated, b._allocated);
  }

  inline GenericValueRef GenericValuePtr::operator*()
  {
    if (kind() == Type::Pointer)
      return static_cast<TypePointer*>(type)->dereference(value);
    else if (kind() == Type::Iterator)
      return static_cast<TypeIterator*>(type)->dereference(value);
    else
      throw std::runtime_error("Expected pointer or iterator");
  }

  inline GenericValueRef GenericIterator::operator*()
  {
    if (kind() == Type::Iterator)
      return static_cast<TypeIterator*>(type)->dereference(value);
    else
      throw std::runtime_error("Expected iterator");
  }

  template<typename T>
  GenericIterator::GenericIterator(const T& ref)
  : GenericValue(GenericValuePtr::ref(ref))
  {

  }
  inline GenericIterator::GenericIterator()
  {
  }

  inline GenericIterator::GenericIterator(const GenericValuePtr& p)
  :GenericValue(p)
  {}

  inline GenericIterator::GenericIterator(const GenericValue& v)
  : GenericValue(v)
  {}

  inline GenericValueRef::GenericValueRef(const GenericValuePtr& src)
  :GenericValuePtr(src)
  {
    if (!type)
      throw std::runtime_error("Reference to empty GenericValuePtr");
  }

  template<typename T>
  GenericValueRef::GenericValueRef(const T& v)
  {
    *(GenericValuePtr*)this = make(&v);
  }

  template<typename T>
  GenericValueRef& GenericValueRef::operator=(const T& v)
  {
    set(v);
    return *this;
  }

  inline GenericIterator
  GenericIterator::operator++()
  {
    if (kind() != Type::Iterator)
      throw std::runtime_error("Expected an iterator");
    static_cast<TypeIterator*>(type)->next(&value);
    return *this;
  }

  inline GenericIterator
  GenericValuePtr::begin() const
  {
    if (kind() == Type::List)
      return static_cast<TypeList*>(type)->begin(value);
    else if (kind() == Type::Map)
      return static_cast<TypeMap*>(type)->begin(value);
    else
      throw std::runtime_error("Expected list or map");
  }

  inline GenericIterator
  GenericValuePtr::end() const
  {
    if (kind() == Type::List)
      return static_cast<TypeList*>(type)->end(value);
    else if (kind() == Type::Map)
      return static_cast<TypeMap*>(type)->end(value);
    else
      throw std::runtime_error("Expected list or map");
  }

  inline bool operator != (const GenericValuePtr& a, const GenericValuePtr& b)
  {
    return !(a==b);
  }
  inline bool operator != (const GenericValue& a, const GenericValue& b)
  {
    return !(a==b);
  }
  inline bool operator != (const GenericIterator& a, const GenericIterator& b)
  {
    return !(a==b);
  }
}

namespace std
{
  inline void swap(::qi::GenericValue& a, ::qi::GenericValue& b)
  {
    a.swap(b);
  }
}

/* Since GenericValuePtr does not handle its memory, it cannot be used
* inside a GenericValuePtr. use GenericValue instead.
*/
QI_NO_TYPE(qi::GenericValuePtr);


#endif  // _QITYPE_DETAILS_GENERICVALUE_HXX_
