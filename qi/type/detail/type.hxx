#pragma once
/*
**  Copyright (C) 2013 Aldebaran Robotics
**  See COPYING for the license
*/

#ifndef _QITYPE_DETAIL_TYPE_HXX_
#define _QITYPE_DETAIL_TYPE_HXX_

#include <qi/atomic.hpp>
#include <qi/types.hpp>
#include <cstring>
#include <map>
#include <vector>
#include <list>
#include <type_traits>

/* This file contains the default-provided Type specialisations
 *
 */


namespace qi  {

  namespace detail {

    // Try to get a nice error message for QI_NO_TYPE
    class ForbiddenInTypeSystem: public TypeImpl<int>
    {
    private:
      ForbiddenInTypeSystem();
    };

    template<typename T>
    inline void initializeType(TypeInterface* &tgt)
    {
      qiLogDebug("qitype.typeof") << "first typeOf request for unregistered type " << qi::typeId<T>().name();
      tgt = new TypeImpl<T>();
    }

    template<typename T>
    inline TypeInterface* typeOfBackend()
    {
      TypeInterface* result = getType(qi::typeId<T>());
      if (!result)
      {

        static TypeInterface* defaultResult = 0;
        QI_ONCE(initializeType<T>(defaultResult));
        result = defaultResult;
      }
      return result;
    }

    template<typename T>
    struct TypeOfAdapter
    {
      using type = T;
    };

    template <typename T>
    using TypeOfAdapter_t = typename TypeOfAdapter<T>::type;

    template<typename T>
    struct TypeOfAdapter<T&>
    {
      using type = TypeOfAdapter_t<T>;
    };

    template<typename T>
    struct TypeOfAdapter<const T>
    {
      using type = TypeOfAdapter_t<T>;
    };

    template<typename T>
    struct TypeOfAdapter<T*>
    {
      using type = typename std::add_pointer<typename std::remove_const<TypeOfAdapter_t<T>>::type>::type;
    };

  }

  template<typename T>
  TypeInterface* typeOf()
  {
    return detail::typeOfBackend<detail::TypeOfAdapter_t<T>>();
  }

  inline TypeKind TypeInterface::kind()
  {
    return TypeKind_Unknown;
  }

  namespace detail {

    // Bouncer to DefaultAccess or DirectAccess based on type size
    template<typename T>
    class TypeImplMethodsBySize
    {
    public:
      /* DISABLE. Inplace modification does not work with TypeByValue.
      * TODO: be able to switch between ByVal and ByPointer on the
      * same type.
      */
      using type = DefaultTypeImplMethods<T>;
      /*
      typedef typename boost::mpl::if_c<
        sizeof(T) <= sizeof(void*),
        DefaultTypeImplMethods<T,
                        TypeByValue<T>
                        >,
        DefaultTypeImplMethods<T,
                        TypeByPointer<T>
                        >
                        >::type type;
      */
    };

    template <typename T>
    using TypeImplMethodsBySize_t = typename TypeImplMethodsBySize<T>::type;
  }

  // To detect a templated type, make all the Type of its instanciations
  // inherit fro a single class
  template<template<typename> class T>
  class QITYPE_TEMPLATE_API TypeOfTemplate;

  template<template<typename> class T, typename I>
  class TypeOfTemplateImpl;
}

#endif  // _QITYPE_DETAIL_TYPE_HXX_
