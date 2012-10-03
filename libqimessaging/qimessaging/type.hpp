#pragma once
/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/

#ifndef _QIMESSAGING_TYPE_HPP_
#define _QIMESSAGING_TYPE_HPP_

#include <typeinfo>
#include <string>

#include <boost/preprocessor.hpp>
#include <boost/type_traits/is_function.hpp>
#include <boost/mpl/if.hpp>

#include <qi/log.hpp>
#include <qimessaging/api.hpp>

namespace qi{

class TypeInt;
class TypeFloat;
class TypeString;
class TypePointer;
class TypeTuple;
class ObjectType;
class GenericValue;
class GenericList;
class GenericMap;
class GenericObject;
class IDataStream;
class ODataStream;
class Signature;


/** This class is used to uniquely identify a type.
 *
 */
class QIMESSAGING_API TypeInfo
{
public:
  TypeInfo();
  /// Construct a TypeInfo from a std::type_info
  TypeInfo(const std::type_info& info);
  /// Contruct a TypeInfo from a custom string.
  TypeInfo(const std::string& ti);
  std::string asString();

  bool operator ==(const TypeInfo& b) const;
  bool operator !=(const TypeInfo& b) const;
  bool operator < (const TypeInfo& b) const;
private:
  const std::type_info* stdInfo;
  std::string customInfo;
};

/** Interface for all the operations we need on any type:
 *
 *  - cloning/destruction in clone() and destroy()
 *  - Access to value from storage and storage creation in
 *    ptrFromStorage() and initializeStorage()
 *  - Type of specialized interface through kind()
 *
 * Our aim is to transport arbitrary values through:
 *  - synchronous calls: Nothing to do, values are just transported and
      converted.
 *  - asynchronous call/thread change: Values are copied.
 *  - process change: Values are serialized.
 *
 */
class QIMESSAGING_API Type
{
public:
  virtual TypeInfo info() =0;
  // Initialize and return a new storage, from nothing or a T*
  virtual void* initializeStorage(void* ptr=0)=0;
  // Get pointer to type from pointer to storage
  // No reference to avoid the case where the compiler makes a copy on the stack
  virtual void* ptrFromStorage(void**)=0;

  virtual void* clone(void*)=0;
  virtual void destroy(void*)=0;

  enum Kind
  {
    Void,
    Int,
    Float,
    String,
    List,
    Map,
    Object,
    Pointer,
    Tuple,
    Dynamic,
    Unknown,
  };
  virtual Kind kind() const;

  std::string infoString() { return info().asString();} // for easy gdb access

  std::string signature();
  static Type* fromSignature(const Signature& sig);

  GenericValue deserialize(IDataStream& in);
  void serialize(ODataStream& out, void* storage);

  // Dispatch based on type kind. Must implement TypeDispatcher.
  template<typename Dispatcher> Dispatcher& dispatch(const Dispatcher& disptacher, void** storage);
};

class QIMESSAGING_API TypeDispatcher
{
public:
  virtual void visitUnknown(Type* type, void* storage)=0;
  virtual void visitVoid(Type*)=0;
  virtual void visitInt(TypeInt* type, int64_t value, bool isSigned, int byteSize)=0;
  virtual void visitFloat(TypeFloat* type, double value, int byteSize)=0;
  virtual void visitString(TypeString* type, void* storage)=0;
  virtual void visitList(GenericList value)=0;
  virtual void visitMap(GenericMap value)=0;
  virtual void visitObject(GenericObject value)=0;
  virtual void visitPointer(TypePointer* type, void* storage, GenericValue pointee)=0;
  virtual void visitTuple(TypeTuple* type, void* storage)=0;
  virtual void visitDynamic(Type* type, GenericValue pointee);
};


namespace detail {
  //template<int I> struct Nothing{};
  template<typename T> struct TypeManagerDefault
  {
    static void* create() { return new T();}
    static void createInPlace(void* ptr) { new(ptr)T();}
    static void copy(void* dst, const void* src) { *(T*)dst = *(const T*)src;}
    static void destroy(void* ptr) { delete (T*)ptr;}
  };
  template<typename T> struct TypeManagerDefault<const T>: public TypeManagerDefault<T>{};
  template<typename T>
  struct TypeManagerNonDefaultConstructible
  {
    static void* create() { return 0;}
    static void createInPlace(void* ptr) {}
    static void copy(void* dst, const void* src) { *(T*)dst = *(const T*)src;}
    static void destroy(void* ptr) { delete (T*)ptr;}
  };
  struct TypeManagerNull
  {
    static void* create() { return 0;}
    static void createInPlace(void* ptr) {}
    template<typename T1, typename T2>
    static void copy(const T1& d, const T2&s) {}
    template<typename T>
    static void destroy(const T& ptr) {}
  };

  template<typename T> struct TypeManager
  : public boost::mpl::if_c<boost::is_function<T>::value,
  TypeManagerNull, TypeManagerDefault<T> >::type
  {};
};
#define QI_TYPE_NOT_CONSTRUCTIBLE(T) \
namespace qi { namespace detail { \
template<> struct TypeManager<T>: public TypeManagerNonDefaultConstructible<T> {};}}
/** Meta-type specialization.
 *  Use the aspect pattern, make a class per feature group
 *  (Clone, GenericValue, Serialize)
 *
 */

/// Access API that stores a T* in storage
template<typename T> class TypeByPointer
{
public:
  typedef T type;
  static void* ptrFromStorage(void** storage)
  {
    return *storage;
  }
  static void* initializeStorage(void* ptr=0)
  {
    if (ptr)
      return ptr;
    void* res = detail::TypeManager<T>::create();
    if (!res)
      qiLogError("qi.meta") << "initializeStorage error on " << typeid(T).name();
    return res;
  }
  static void* clone(void* src)
  {
    const T* ptr = (const T*)ptrFromStorage(&src);
    void* res = initializeStorage();
    T* tres = (T*)ptrFromStorage(&res);
    detail::TypeManager<type>::copy(tres, ptr);
    return res;
  }
  static void destroy(void* src)
  {
    T* ptr = (T*)ptrFromStorage(&src);
    detail::TypeManager<type>::destroy(ptr);
  }
};

template<typename T> class TypeByPointer<const T>: public TypeByPointer<T>{};

/// Access api that stores T in storage
template<typename T> class TypeByValue
{
public:
  typedef T type;
  static void* ptrFromStorage(void** storage)
  {
    return storage;
  }
  static void* initializeStorage(void* ptr=0)
  {
    void* result = 0;
    T* tresult=(T*)(void*)&result;
    if (ptr)
    {
      detail::TypeManager<T>::copy(tresult, (T*)ptr);
      return result;
    }
    else
    {
      detail::TypeManager<T>::createInPlace(tresult);
      return result;
    }
  }
  static void* clone(void* src)
  {
    return src;
  }

  static void destroy(void* storage)
  {
    T* ptr = (T*)ptrFromStorage(&storage);
    ptr->~T();
  }
};

// const ward
template<typename T> class TypeByValue<const T>: public TypeByValue<T> {};


/* implementation of Type methods that bounces to the various aspect
 * subclasses.
 *
 * It does not inherit from Type on purpose, to avoid diamond inheritance
 * when specializing the type interface
 * That way we can split the various aspects in different classes
 * for better reuse, without the cost of a second virtual call.
 */
template<typename T, typename _Access    = TypeByPointer<T>
         > class DefaultTypeImplMethods
{
public:
  typedef _Access Access;

  static void* initializeStorage(void* ptr=0)
  {
    return Access::initializeStorage(ptr);
  }

  static void* ptrFromStorage(void** storage)
  {
    return Access::ptrFromStorage(storage);
  }

  static const std::type_info& info()
  {
    return typeid(T);
  }

  static void* clone(void* src)
  {
    return Access::clone(src);
  }

  static void destroy(void* ptr)
  {
    Access::destroy(ptr);
  }
};

#define _QI_BOUNCE_TYPE_METHODS_NOCLONE(Bounce)                                          \
virtual TypeInfo info() { return Bounce::info();}                           \
virtual void* initializeStorage(void* ptr=0) { return Bounce::initializeStorage(ptr);}   \
virtual void* ptrFromStorage(void**s) { return Bounce::ptrFromStorage(s);}


#define _QI_BOUNCE_TYPE_METHODS(Bounce)  \
_QI_BOUNCE_TYPE_METHODS_NOCLONE(Bounce) \
virtual void* clone(void* ptr) { return Bounce::clone(ptr);}    \
virtual void destroy(void* ptr) { Bounce::destroy(ptr);}

#define _QI_BOUNCE_TYPE_METHODS_NOINFO(Bounce) \
virtual void* initializeStorage(void* ptr=0) { return Bounce::initializeStorage(ptr);} \
virtual void* ptrFromStorage(void**s) { return Bounce::ptrFromStorage(s);}             \
virtual void* clone(void* ptr) { return Bounce::clone(ptr);}    \
virtual void destroy(void* ptr) { Bounce::destroy(ptr);}


template<typename T, typename _Access    = TypeByPointer<T> >
class DefaultTypeImpl
: public Type
{
public:
  typedef DefaultTypeImplMethods<T, _Access> MethodsImpl;
  _QI_BOUNCE_TYPE_METHODS(MethodsImpl);
};

/* Type "factory". Specialize this class to provide a custom
 * Type for a given type.
 */
template<typename T> class TypeImpl: public DefaultTypeImpl<T>
{
};


/// Declare that a type has no metatype and cannot be used in a Value
#define QI_NO_TYPE(T) namespace qi {template<> class TypeImpl<T> {};}

/// Declare that a type with default TypeImpl is not clonable
#define QI_TYPE_NOT_CLONABLE(T) \
namespace qi { namespace detail { \
template<> struct TypeManager<T>: public TypeManagerNull {};}}

/// Type factory getter. All other type access mechanism bounce here
QIMESSAGING_API Type*  getType(const std::type_info& type);
/// Type factory setter
QIMESSAGING_API bool registerType(const std::type_info& typeId, Type* type);

/// Register TypeImpl<t> in runtime type factory for 't'. Must be called from a .cpp file
#define QI_TYPE_REGISTER(t) \
  QI_TYPE_REGISTER_CUSTOM(t, qi::TypeImpl<t>)
/// Register 'typeimpl' in runtime type factory for 'type'.
#define QI_TYPE_REGISTER_CUSTOM(type, typeimpl) \
static bool BOOST_PP_CAT(__qi_registration, __LINE__) = qi::registerType(typeid(type), new typeimpl)

/** Get type from a type. Will return a static TypeImpl<T> if T is not registered
 */
template<typename T> Type* typeOf();

/// Get type from a value. No need to delete the result
template<typename T> Type* typeOf(const T& v)
{
  return typeOf<T>();
}

}

#include <qimessaging/details/type.hxx>

#endif  // _QIMESSAGING_TYPE_HPP_
