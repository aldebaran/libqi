#pragma once
/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/

#ifndef _QITYPE_DETAILS_TYPE_HXX_
#define _QITYPE_DETAILS_TYPE_HXX_

#include <qi/types.hpp>
#include <cstring>
#include <map>
#include <vector>
#include <list>
#include <qitype/details/bindtype.hxx>
#include <boost/mpl/for_each.hpp>
#include <boost/mpl/transform_view.hpp>
#include <boost/type_traits/remove_reference.hpp>
#include <boost/type_traits/add_pointer.hpp>
#include <boost/function_types/parameter_types.hpp>
#include <boost/function_types/result_type.hpp>
#include <boost/function_types/function_type.hpp>
#include <boost/function_types/is_member_pointer.hpp>

/* This file contains the default-provided Type specialisations
 *
 */


namespace qi {
  // void
  template<> class TypeImpl<void>: public Type
  {
  public:
   const TypeInfo& info()
    {
      static TypeInfo result = TypeInfo(typeid(void));
      return result;
    }
    void* initializeStorage(void*) { return 0;}
    void* ptrFromStorage(void** ) { return 0;}
    void* clone(void*)                       { return 0;}
    void destroy(void* ptr)                  {}
    Kind kind() const { return Void;}
    bool less(void* a, void* b) { return false;}
  };

  //reference

  template<typename T> class TypeImpl<T&>
      : public TypeImpl<T> {};

  //any
  template<> class TypeImpl<boost::any>: public DynamicTypeInterface
  {
  public:
    GenericValuePtr get(void* storage)
    {
      qiLogVerbose("qitype.impl") << "get on boost::any not implemented";
      return GenericValuePtr();
    };
    void set(void** storage, GenericValuePtr source)
    {
      qiLogVerbose("qitype.impl") << "set on boost::any not implemented";
    }
    typedef DefaultTypeImplMethods<boost::any> Methods;
    _QI_BOUNCE_TYPE_METHODS(Methods);
  };
}



namespace qi  {

  namespace detail {
    // Try to get a nice error message for QI_NO_TYPE
    class ForbiddenInTypeSystem: public TypeImpl<int>
    {
    private:
      ForbiddenInTypeSystem();
    };
    template<typename T> inline Type* typeOfBackend()
    {
      Type* result = getType(typeid(T));
      if (!result)
      {

        static Type* defaultResult = 0;
        // Is this really a problem?
        if (!defaultResult)
        {
          qiLogDebug("qitype.typeof") << "first typeOf request for unregistered type "
          << typeid(T).name();
        }
        if (!defaultResult)
          defaultResult = new TypeImpl<T>();
        result = defaultResult;
      }

      return result;
    }

    template<typename T> struct TypeOfAdapter
    {
      typedef T type;
    };
    template<typename T> struct TypeOfAdapter<T&>
    {
      typedef typename TypeOfAdapter<T>::type type;
    };
    template<typename T> struct TypeOfAdapter<const T>
    {
      typedef typename TypeOfAdapter<T>::type type;
    };
    template<typename T> struct TypeOfAdapter<T*>
    {
      typedef typename boost::add_pointer<typename boost::remove_const<typename TypeOfAdapter<T>::type>::type>::type type;
    };
  }

  template<typename T> Type* typeOf()
  {
    return detail::typeOfBackend<typename detail::TypeOfAdapter<T>::type>();
  }

  inline Type::Kind Type::kind() const
  {
    return Unknown;
  }

  namespace detail {
    struct signature_function_arg_apply {
      signature_function_arg_apply(std::string* val)
        : val(*val)
      {}

      template<typename T> void operator()(T *x) {
        val += qi::typeOf<T>()->signature().toString();
      }

      std::string &val;
    };



    template<typename T> struct RawFunctionSignature
    {
      static qi::Signature makeSigreturn()
      {
        typedef typename boost::function_types::result_type<T>::type     ResultType;
        return typeOf<ResultType>()->signature();
      }
      static qi::Signature makeSignature()
      {
        std::string   signature;
        signature += '(';
        typedef typename boost::function_types::parameter_types<T>::type ArgsType;
        boost::mpl::for_each<
          boost::mpl::transform_view<ArgsType,
            boost::add_pointer<
              boost::remove_const<
                boost::remove_reference<boost::mpl::_1>
              >
            >
          >
        >(qi::detail::signature_function_arg_apply(&signature));
        signature += ')';
        return qi::Signature(signature);
      }
    };

    template<typename R, typename F, typename B>
    struct RawFunctionSignature<boost::_bi::bind_t<R, F, B> >
    {
      static qi::Signature makeSigreturn()
      {
        typedef typename qi::boost_bind_result_type<boost::_bi::bind_t<R, F, B> >::type     ResultType;
        return typeOf<ResultType>()->signature();
      }
      static qi::Signature makeSignature()
      {
        std::string   signature;
        signature += '(';
        typedef typename qi::boost_bind_parameter_types<boost::_bi::bind_t<R, F, B> >::type ArgsType;
        boost::mpl::for_each<
          boost::mpl::transform_view<ArgsType,
            boost::add_pointer<
              boost::remove_const<
                boost::remove_reference<boost::mpl::_1>
              >
            >
          >
        >(qi::detail::signature_function_arg_apply(&signature));
        signature += ')';
        return Signature(signature);
      }
    };
    template<typename T> struct MemberFunctionSignature
    {
      static qi::Signature makeSigreturn()
      {
        typedef typename boost::function_types::result_type<T>::type     ResultType;
        return typeOf<ResultType>()->signature();
      }
      static qi::Signature makeSignature()
      {
        // Reconstruct the boost::bind(instance, _1, _2...) signature
        typedef typename boost::function_types::result_type<T>::type     RetType;
        typedef typename boost::function_types::parameter_types<T>::type MemArgsType;
        typedef typename boost::mpl::pop_front< MemArgsType >::type                ArgsType;
        typedef typename boost::mpl::push_front<ArgsType, RetType>::type EffectiveType;
        typedef typename boost::function_types::function_type<EffectiveType>::type type;
        return RawFunctionSignature<type>::makeSignature();
      }
    };
    template<typename T> struct FunctionSignature
    {
      typedef typename  boost::mpl::if_<
        typename boost::function_types::is_member_pointer<T>,
        MemberFunctionSignature<T>,
        RawFunctionSignature<T>
        >::type Backend;
      static qi::Signature signature()
      {
        static qi::Signature result = Backend::makeSignature();
        return result;
      }
      static qi::Signature sigreturn()
      {
        static qi::Signature result = Backend::makeSigreturn();
        return result;
      }
    };
    template<typename T> struct FunctionSignature<boost::function<T> >
    : public FunctionSignature<T> {};

    template<typename T> inline
    qi::Signature functionArgumentsSignature()
    {
      static bool done = false;
      static std::string sigs;
      if (!done)
      {
        sigs += '(';
        typedef typename boost::function_types::parameter_types<T>::type ArgsType;
        boost::mpl::for_each<
        boost::mpl::transform_view<ArgsType,
        boost::add_pointer<
        boost::remove_const<
        boost::remove_reference<boost::mpl::_1> > > > > (qi::detail::signature_function_arg_apply(&sigs));
        sigs += ')';
        done = true;
      }
      return Signature(sigs);
    }

    // Bouncer to DefaultAccess or DirectAccess based on type size
    template<typename T>
    class TypeImplMethodsBySize
    {
    public:
      /* DISABLE. Inplace modification does not work with TypeByValue.
      * TODO: be able to switch between ByVal and ByPointer on the
      * same type.
      */
      typedef DefaultTypeImplMethods<T> type;
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
  }

  // Provide a base class for all templated type impls
  class QITYPE_API TypeTemplate: public Type
  {
  public:
    // Return erased template argument type
    virtual Type* templateArgument() = 0;
    // If this type is also an object, return it.
    virtual Type* next() { return 0;}
  };

  // To detect a templated type, make all the Type of its instanciations
  // inherit fro a single class
  template<template<typename> class T> class QITYPE_TEMPLATE_API TypeOfTemplate: public TypeTemplate
  {
  public:

  };

  // Default Type for template type T instanciated with type I
  template<template<typename> class T, typename I> class QITYPE_TEMPLATE_API TypeOfTemplateDefaultImpl:
  public TypeOfTemplate<T>
  {
  public:
     virtual Type* templateArgument()
    {
      return typeOf<I>();
    }
    typedef DefaultTypeImplMethods<T<I> > Methods;
    _QI_BOUNCE_TYPE_METHODS(Methods);
  };
  template<template<typename> class T, typename I> class QITYPE_TEMPLATE_API TypeOfTemplateImpl
  : public TypeOfTemplateDefaultImpl<T, I> {};
}

#endif  // _QITYPE_DETAILS_TYPE_HXX_
