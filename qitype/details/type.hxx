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
  };

  //reference

  template<typename T> class TypeImpl<T&>
      : public TypeImpl<T> {};

  //any
  template<> class TypeImpl<boost::any>: public TypeDynamic
  {
  public:
    std::pair<GenericValuePtr, bool> get(void* storage)
    {
      qiLogVerbose("qitype.impl") << "get on boost::any not implemented";
      return std::make_pair(GenericValuePtr(), false);
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
    template<typename T> inline Type* typeOfBackend()
    {
      Type* result = getType(typeid(T));
      if (!result)
      {

        static Type* defaultResult = 0;
        // Is this realy a problem?
        if (!defaultResult)
          qiLogDebug("qitype.typeof") << "typeOf request for unregistered type "
          << typeid(T).name();
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
        val += qi::typeOf<T>()->signature();
      }

      std::string &val;
    };



    template<typename T> struct RawFunctionSignature
    {
      static std::string makeSigreturn()
      {
        typedef typename boost::function_types::result_type<T>::type     ResultType;
        return typeOf<ResultType>()->signature();
      }
      static std::string makeSignature()
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
        >
        (qi::detail::signature_function_arg_apply(&signature));
        signature += ')';
        return signature;
      }
    };

    template<typename R, typename F, typename B>
    struct RawFunctionSignature<boost::_bi::bind_t<R, F, B> >
    {
      static std::string makeSigreturn()
      {
        typedef typename qi::boost_bind_result_type<boost::_bi::bind_t<R, F, B> >::type     ResultType;
        return typeOf<ResultType>()->signature();
      }
      static std::string makeSignature()
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
        >
        (qi::detail::signature_function_arg_apply(&signature));
        signature += ')';
        return signature;
      }
    };
    template<typename T> struct MemberFunctionSignature
    {
      static std::string makeSigreturn()
      {
        typedef typename boost::function_types::result_type<T>::type     ResultType;
        return typeOf<ResultType>()->signature();
      }
      static std::string makeSignature()
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
      static std::string signature()
      {
        static std::string result = Backend::makeSignature();
        return result;
      }
      static std::string sigreturn()
      {
        static std::string result = Backend::makeSigreturn();
        return result;
      }
    };
    template<typename T> struct FunctionSignature<boost::function<T> >
    : public FunctionSignature<T> {};

    template<typename T> inline
    std::string functionArgumentsSignature()
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
      return sigs;
    }

    // Bouncer to DefaultAccess or DirectAccess based on type size
    template<typename T>
    class TypeImplMethodsBySize
    {
    public:
      typedef typename boost::mpl::if_c<
        sizeof(T) <= sizeof(void*),
        DefaultTypeImplMethods<T,
                        TypeByValue<T>
                        >,
        DefaultTypeImplMethods<T,
                        TypeByPointer<T>
                        >
                        >::type type;
    };
  }

}

#endif  // _QITYPE_DETAILS_TYPE_HXX_
