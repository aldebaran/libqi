#pragma once
/*
**  Copyright (C) 2013 Aldebaran Robotics
**  See COPYING for the license
*/

#ifndef _QITYPE_DETAIL_TYPEIMPL_HXX_
#define _QITYPE_DETAIL_TYPEIMPL_HXX_

#include <set>
#include <qi/type/detail/hasless.hxx>


/* This file contains class to help implementing the Type interface.
*/
namespace qi
{
  namespace detail {
    /// Report a type operation failure
    void QI_API typeFail(const char* typeName, const char* operation);

    /* Handle creation (create, createInPlace), copy (copy, clone, cloneInPlace), and
    *  destruction(destroy) of a given type.
    *  There is currently no portable way to detect if a given class has
    *  accesible constructor, destructor and/or copy operators.
    *  So we provide a reasonable default behavior (through our class/struct
    *  registration macros) and rely on the user to call the appropriate
    *  QI_TYPE_NOT_* macro to override.
    */
    template<typename T, bool b>
    struct TypeTraitCreate
    {
      static void* create() { return new T();}
      static void createInPlace(void* ptr) { new(ptr)T();}
    };

    template<typename T>
    struct TypeTraitCreate<T, false>
    {
      static void* create() { typeFail(qi::typeId<T>().name(), "default constructor"); return 0;}
      static void createInPlace(void* /*ptr*/) {typeFail(qi::typeId<T>().name(), "default constructor");}
    };

    template<typename T, bool b>
    struct TypeTraitCopy
    {
      static void copy(void* dst, const void* src) { *(T*)dst = *(const T*)src;}
      static void* clone(void* src) { return new T(*(T*)src);}
      static void cloneInPlace(void* ptr, void* src) { new (ptr)T(*(T*)src);}
    };

    template<typename T>
    struct TypeTraitCopy<T, false>
    {
      template<typename T1, typename T2>
      static void copy(const T1&, const T2&) {typeFail(qi::typeId<T>().name(), "copy operator");}
      static void* clone(void* /*src*/) { typeFail(qi::typeId<T>().name(), "clone"); return 0;}
      static void cloneInPlace(void* /*ptr*/, void* /*src*/) { typeFail(qi::typeId<T>().name(), "clone");}
    };

    template<typename T, bool b>
    struct TypeTraitDestroy
    {
      static void destroy(void* ptr) { delete (T*)ptr;}
    };

    template<typename T>
    struct TypeTraitDestroy<T, false>
    {
      template<typename U>
      static void destroy(const U& /*ptr*/) {typeFail(qi::typeId<T>().name(), "destructor");}
    };

    /* Use a two-stage override mechanism.
     * That way if a user uses our macro that specializes
     * TypeManager on one of the versions below, she can still
     * specializes the appropriate TypeManagerDefault{Struct,Class}.
     */
    template<typename T>
    struct TypeManagerDefaultStruct
      : public TypeTraitCreate<T, true>
      , public TypeTraitCopy<T, true>
      , public TypeTraitDestroy<T, true>
    {};

    template<typename T>
    struct TypeManagerDefaultInterface
      : public TypeTraitCreate<T, false>
      , public TypeTraitCopy<T, false>
      , public TypeTraitDestroy<T, true>
    {};

    template<typename T>
    struct TypeManagerNull
    : public TypeTraitCreate<T, false>
    , public TypeTraitCopy<T, false>
    , public TypeTraitDestroy<T, false>
    {};

    template<typename T>
    struct TypeManagerNotConstructible // not constructible but copyable
    : public TypeTraitCreate<T, false>
    , public TypeTraitCopy<T, true>
    , public TypeTraitDestroy<T, true>
    {};


    // TypeManager is accessed by this interface.
    // Only things for which we are sure are marked constructible and clonable
    template <typename T>
    struct TypeManager
        : public std::conditional_t<
              std::is_function_v<T>, TypeManagerNull<T>,
              std::conditional_t<std::is_pod_v<T>, TypeManagerDefaultStruct<T>,
                                 TypeManagerDefaultInterface<T>>> {};

    // Except for boost::function which matches is_function and is copyable
    template<typename T>
    struct TypeManager<boost::function<T> >: public TypeManagerDefaultStruct<boost::function<T> >
    {};
    template<typename T>
    struct TypeManager<const T>: public TypeManager<T>{};

  }

  /* To avoid the diamond inheritance problem (interface inheritance between
 * for instance Type and IntTypeInterface, and we want to reuse implementations),
 * Provide methods in a class that does not inherit from Type, and a macro
 * to generate the virtual bouncers.
 */
  /// Access API that stores a T* in storage
  template <typename T, typename Manager=detail::TypeManager<T> >
  class TypeByPointer
  {
  public:
    using type = T;

    static void* ptrFromStorage(void** storage)
    {
      return *storage;
    }

    static void* initializeStorage(void* ptr=0)
    {
      if (ptr)
        return ptr;
      //Warning:
      // If T is not clonable (no copy constructor or operator=)
      // add QI_TYPE_NOT_CLONABLE(T) next to the declaration of T
      // in your code.
      void* res = Manager::create();
      if (!res)
        qiLogError("qitype.bypointer") << "initializeStorage error on " << qi::typeId<T>().name();
      return res;
    }

    static void* clone(void* src)
    {
      return Manager::clone(src);
    }

    static void destroy(void* src)
    {
      T* ptr = (T*)ptrFromStorage(&src);
      Manager::destroy(ptr);
    }
  };

  // const ward
  template <typename T>
  class TypeByPointer<const T>: public TypeByPointer<T>
  {};
  // Helper to mark POD-like
  template <typename T>
  class TypeByPointerPOD: public TypeByPointer<T, detail::TypeManagerDefaultStruct<T> >{};
  /// Access api that stores a T in storage
  /* No longuer used since we have an in-place modification
  * of AnyReference, as by-value storage would break reference semantic
  */
  template <typename T>
  class TypeByValue
  {
  public:
    using type = T;
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
  template <typename T>
  class TypeByValue<const T>: public TypeByValue<T>
  {};


  /* implementation of Type methods that bounces to the various aspect
 * subclasses.
 *
 */
  template < typename T, typename _Access = TypeByPointer<T> >
  class DefaultTypeImplMethods
  {
  public:
    using Access = _Access;

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
      static const TypeInfo result(qi::typeId<T>());
      return result;
    }

    static void* clone(void* src)
    {
      return Access::clone(src);
    }

    static void destroy(void* ptr)
    {
      Access::destroy(ptr);
    }

    static bool less(void* a, void* b)
    {
      return ::qi::detail::Less<T>()((T*)ptrFromStorage(&a), (T*)ptrFromStorage(&b));
    }
  };

  ///Implement all methods of Type except clone/destroy as bouncers to Bouncer
#define _QI_BOUNCE_TYPE_METHODS_NOCLONE(Bounce)                                                     \
  const ::qi::TypeInfo& info() override { return Bounce::info();}                                   \
  void* initializeStorage(void* ptr=0) override { return Bounce::initializeStorage(ptr);}           \
  void* ptrFromStorage(void**s) override { return Bounce::ptrFromStorage(s);}                       \
  bool  less(void* a, void* b) override { return Bounce::less(a, b);}

  ///Implement all methods of Type as bouncers to Bouncer
#define _QI_BOUNCE_TYPE_METHODS(Bounce)                                                             \
  _QI_BOUNCE_TYPE_METHODS_NOCLONE(Bounce)                                                           \
  void* clone(void* ptr) override { return Bounce::clone(ptr);}                                     \
  void destroy(void* ptr) override { Bounce::destroy(ptr);}

  ///Implement all methods of Type except info() as bouncers to Bouncer.
#define _QI_BOUNCE_TYPE_METHODS_NOINFO(Bounce)                                                    \
  void* initializeStorage(void* ptr=0) override { return Bounce::initializeStorage(ptr);}         \
  void* ptrFromStorage(void**s) override { return Bounce::ptrFromStorage(s);}                     \
  void* clone(void* ptr) override { return Bounce::clone(ptr);}                                   \
  void  destroy(void* ptr) override { Bounce::destroy(ptr);}                                      \
  bool  less(void* a, void* b) override { return Bounce::less(a, b);}

  template < typename T, typename _Access = TypeByPointer<T> >
  class DefaultTypeImpl
      : public TypeInterface
  {
  public:
    using MethodsImpl = DefaultTypeImplMethods<T, _Access>;
    _QI_BOUNCE_TYPE_METHODS(MethodsImpl);
  };

  /** Compile-time Type "factory", used by typeOf<T>() in case no runtime Type
 * was registered. Specialize this class to provide a custom
 * Type for a given type, in a header, or use registerType() in a cpp.
 */
  template<typename T>
  class TypeImpl: public DefaultTypeImpl<T>
  {};

  // void
  template<>
  class TypeImpl<void>: public TypeInterface
  {
  public:
    const TypeInfo& info() override
    {
      static TypeInfo result = TypeInfo(qi::typeId<void>());
      return result;
    }
    // do not return 0 everywhere and provide a correct impl
    // this may be used as void*, that's why we need to keep storage
    void* initializeStorage(void* ptr) override { return ptr; }
    void* ptrFromStorage(void** storage) override { return (void*)storage; }
    void* clone(void* storage) override { return storage; }
    void destroy(void*) override {}
    TypeKind kind() override { return TypeKind_Void; }
    bool less(void*, void*) override { return false; }
  };

  //reference
  template<typename T>
  class TypeImpl<T&>: public TypeImpl<T>
  {};



}

namespace _qi_
{
  namespace qi
  {
    // Support for QI_TYPE_STRUCT_REGISTER and QI_TYPE_STRUCT_BOUNCE_REGISTER
    template<typename T>
    class TypeImpl
    {};
  }
}

#endif  // _QITYPE_DETAIL_TYPEIMPL_HXX_
