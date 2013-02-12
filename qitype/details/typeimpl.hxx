#pragma once
/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/

#ifndef _QITYPE_DETAILS_TYPEIMPL_HXX_
#define _QITYPE_DETAILS_TYPEIMPL_HXX_

#include <set>

/* This file contains class to help implementing the Type interface.
*/
namespace qi
{
  namespace detail {
  /// Report a type operation failure
  void QITYPE_API typeFail(const char* typeName, const char* operation);
  /** Methods to construct, destroy, and copy values of a given type.
  */
  template<typename T> struct TypeManagerDefault
  {
    static void* create() { return new T();}
    static void createInPlace(void* ptr) { new(ptr)T();}
    static void copy(void* dst, const void* src) { *(T*)dst = *(const T*)src;}
    static void destroy(void* ptr) { delete (T*)ptr;}
    static void* clone(void* src) { return new T(*(T*)src);}
    static void cloneInPlace(void* ptr, void* src) { new (ptr)T(*(T*)src);}
  };
  template<typename T> struct TypeManagerDefault<const T>: public TypeManagerDefault<T>{};
  template<typename T>
  struct TypeManagerNonDefaultConstructible
  {
    static void* create() { typeFail(typeid(T).name(), "default constructor"); return 0;}
    static void createInPlace(void* ptr) {typeFail(typeid( T).name(), "default constructor");}
    static void copy(void* dst, const void* src) { *(T*)dst = *(const T*)src;}
    static void destroy(void* ptr) { delete (T*)ptr;}
    static void* clone(void* src) { return new T(*(T*)src);}
    static void cloneInPlace(void* ptr, void* src) { new (ptr)T(*(T*)src);}
  };
  template<typename T>
  struct TypeManagerNull
  {
    static void* create() { typeFail(typeid(T).name(), "default constructor"); return 0;}
    static void createInPlace(void* ptr) {typeFail(typeid(T).name(), "default constructor"); }
    template<typename T1, typename T2>
    static void copy(const T1& d, const T2&s) {typeFail(typeid(T).name(), "copy operator");}
    template<typename U>
    static void destroy(const U& ptr) {typeFail(typeid(T).name(), "destructor");}
    static void* clone(void* src) { typeFail(typeid(T).name(), "clone"); return 0;}
    static void cloneInPlace(void* ptr, void* src) { typeFail(typeid(T).name(), "clone");}
  };

  // TypeManager is accessed by this interface. By default, everything is
  // constructible and copyable except functions
  template<typename T>
  struct TypeManager
  : public boost::mpl::if_c<boost::is_function<T>::value,
  TypeManagerNull<T>, TypeManagerDefault<T> >::type
  {};

  // Except for boost::function which maches is_function and is copyable
  template<typename T>
  struct TypeManager<boost::function<T> >
  : public TypeManagerDefault<boost::function<T> > {};

  }

/* To avoid the diamond inheritance problem (interface inheritance between
 * for instance Type and TypeInt, and we want to reuse implementations),
 * Provide methods in a class that does not inherit from Type, and a macro
 * to generate the virtual bouncers.
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
      qiLogError("qitype.bypointer") << "initializeStorage error on " << typeid(T).name();
    return res;
  }
  static void* clone(void* src)
  {
    return detail::TypeManager<T>::clone(src);
  }
  static void destroy(void* src)
  {
    T* ptr = (T*)ptrFromStorage(&src);
    detail::TypeManager<type>::destroy(ptr);
  }
};

// const ward
template<typename T> class TypeByPointer<const T>: public TypeByPointer<T>{};

/// Access api that stores a T in storage
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
    detail::TypeManager<T>::createInPlace(tresult);

    if (ptr)
      detail::TypeManager<T>::copy(tresult, (T*)ptr);

    return result;
  }
  static void* clone(void* src)
  {
    void* res;
    detail::TypeManager<T>::cloneInPlace(&res, (T*)&src);
    return res;
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

  static const TypeInfo&  info()
  {
    static TypeInfo* result = 0;
    if (!result)
      result = new TypeInfo(typeid(T));
    return *result;
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

///Implement all methods of Type except clone/destroy as bouncers to Bouncer
#define _QI_BOUNCE_TYPE_METHODS_NOCLONE(Bounce)                                          \
virtual const ::qi::TypeInfo& info() { return Bounce::info();}                           \
virtual void* initializeStorage(void* ptr=0) { return Bounce::initializeStorage(ptr);}   \
virtual void* ptrFromStorage(void**s) { return Bounce::ptrFromStorage(s);}

///Implement all methods of Type as bouncers to Bouncer
#define _QI_BOUNCE_TYPE_METHODS(Bounce)  \
_QI_BOUNCE_TYPE_METHODS_NOCLONE(Bounce) \
virtual void* clone(void* ptr) { return Bounce::clone(ptr);}    \
virtual void destroy(void* ptr) { Bounce::destroy(ptr);}

///Implement all methods of Type except info() as bouncers to Bouncer.
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

/** Compile-time Type "factory", used by typeOf<T>() in case no runtime Type
 * was registered. Specialize this class to provide a custom
 * Type for a given type, in a header, or use registerType() in a cpp.
 */
template<typename T> class TypeImpl: public DefaultTypeImpl<T>
{
};

}
namespace _qi_
{
  namespace qi
  {
    // Support for QI_TYPE_STRUCT_REGISTER and QI_TYPE_STRUCT_BOUNCE_REGISTER
    template<typename T> class TypeImpl {};
  }
}

#endif  // _QITYPE_DETAILS_TYPEIMPL_HXX_
