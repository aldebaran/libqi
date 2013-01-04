#pragma once
/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/

#ifndef _QITYPE_DETAILS_FUNCTIONTYPEFACTORY_HXX_
#define _QITYPE_DETAILS_FUNCTIONTYPEFACTORY_HXX_

#ifdef BOOST_FUSION_INVOKE_FUNCTION_OBJECT_MAX_ARITY
# undef BOOST_FUSION_INVOKE_FUNCTION_OBJECT_MAX_ARITY
#endif
#define BOOST_FUSION_INVOKE_FUNCTION_OBJECT_MAX_ARITY 10

#include <boost/fusion/include/mpl.hpp>
#include <boost/mpl/for_each.hpp>
#include <boost/mpl/transform_view.hpp>
#include <boost/mpl/find_if.hpp>
#include <boost/mpl/vector.hpp>
#include <boost/mpl/pop_front.hpp>
#include <boost/mpl/at.hpp>
#include <boost/mpl/placeholders.hpp>
#include <boost/mpl/max_element.hpp>
#include <boost/mpl/transform.hpp>
#include <boost/type_traits/remove_reference.hpp>
#include <boost/type_traits/add_pointer.hpp>
#include <boost/type_traits/remove_const.hpp>
#include <boost/type_traits/remove_pointer.hpp>
#include <boost/function_types/function_type.hpp>
#include <boost/function_types/function_arity.hpp>
#include <boost/function_types/function_pointer.hpp>
#include <boost/function_types/result_type.hpp>
#include <boost/function_types/parameter_types.hpp>
#include <boost/bind.hpp>
#include <boost/any.hpp>

namespace qi
{
  namespace detail
  {


    template<typename T> struct Ident
    {
    };

    struct checkForNonConstRef
    {
      template<typename T> void operator()(Ident<T>)
      {
        if (boost::is_reference<T>::value && !boost::is_const<
          typename boost::remove_reference<T>::type>::value)
          qiLogWarning("qi.meta") << "Function argument is a non-const reference: " << typeid(T).name();
      }
    };
    template<typename T> struct remove_constptr
    {
      typedef T type;
    };
    template<typename T> struct remove_constptr<const T*>
    {
      typedef T* type;
    };
    struct fill_arguments
    {
      inline fill_arguments(std::vector<Type*>* target)
      : target(target) {}

      template<typename T> void operator()(T*) const
      {
        Type* result = typeOf<
          typename remove_constptr<
            typename boost::remove_const<
               typename boost::remove_reference<T>::type
            >::type>::type>();
        target->push_back(result);
      }
      std::vector<Type*>* target;
    };

  } // namespace detail


  template<typename T, int arity> struct SafePopFront
  {
    typedef typename boost::mpl::pop_front<T>::type type;
  };

  template<typename T> struct SafePopFront<T, 0>
  {
    typedef T type;
  };

  template<typename T, int n> struct MakeCall
  {
    void* operator() (boost::function<T>&f, void** args);
  };

  // Generate MakeCall<F>::operator()(function<F>, void** args) for each
  // argument count.
  #define callArg(z, n, _) BOOST_PP_COMMA_IF(n) *(typename boost::remove_reference<P##n>::type*)t##n->ptrFromStorage(&args[n])
#define makeCall(n, argstypedecl, argstype, argsdecl, argsues, comma)     \
  template<typename R comma argstypedecl > struct MakeCall<R(argstype), n>  \
  {                                                                         \
    void* operator() (boost::function<R(argstype)>&f, void** args)          \
    {                                                                       \
      QI_GEN_PREPOST2(n, static qi::Type* t,  = qi::typeOf<P, >(););        \
      detail::GenericValuePtrCopy val;                                              \
      val(), f(                                                               \
        BOOST_PP_REPEAT(n, callArg, _)                                      \
        );                                                                  \
      return val.value;                                                     \
    }                                                                       \
  };
  QI_GEN(makeCall)
  #undef callArg
  #undef makeCall

  template<typename T> class FunctionTypeImpl:
  public FunctionType
  {
  public:
    FunctionTypeImpl(bool isMethod = false)
    {
      _resultType = typeOf<typename boost::function_types::result_type<T>::type >();

      typedef typename boost::function_types::parameter_types<T>::type ArgsType;
      // Detect and warn about non-const reference arguments
      if (isMethod) // skip first argument. Runtime switch so cant pop_front directly
        boost::mpl::for_each<
         boost::mpl::transform_view<
           typename SafePopFront<ArgsType, boost::function_types::function_arity<T>::value>::type,
           detail::Ident<boost::mpl::_1>
           > >(detail::checkForNonConstRef());
      else
      boost::mpl::for_each<
         boost::mpl::transform_view<ArgsType,
           detail::Ident<boost::mpl::_1>
           > >(detail::checkForNonConstRef());
      // Generate and store a Type* for each argument
      boost::mpl::for_each<
        boost::mpl::transform_view<ArgsType,
        boost::add_pointer<
        boost::remove_const<
        boost::remove_reference<boost::mpl::_1> > > > >(detail::fill_arguments(&_argumentsType));
    }

    virtual void* call(void* func, void** args, unsigned int argc)
    {
      boost::function<T>* f = (boost::function<T>*)ptrFromStorage(&func);
      return MakeCall<T, boost::function_types::function_arity<T>::value>()(*f, args);
    }

    _QI_BOUNCE_TYPE_METHODS(DefaultTypeImplMethods<boost::function<T> >);
  };

  template<typename T> FunctionType* makeFunctionType()
  {
    static FunctionTypeImpl<T>* result = 0;
    if (!result)
      result = new FunctionTypeImpl<T>();
    return result;
  }

  namespace detail
  {
    // Use helper structures for which template partial specialisation is possible
    template<typename T> struct GenericFunctionMaker
    {
      static GenericFunction make(T func)
      {
        return GenericFunctionMaker<typename boost::function<T> >::make(boost::function<T>(func));
      }
    };
    template<typename T> struct GenericFunctionMaker<T*>
    {
      static GenericFunction make(T* func)
      {
         return GenericFunctionMaker<typename boost::function<T> >::make(boost::function<T>(func));
      }
    };
    template<typename R, typename F, typename B>
    struct GenericFunctionMaker<boost::_bi::bind_t<R, F, B> >
    {
      static GenericFunction make(boost::_bi::bind_t<R, F, B> v)
      {
        typedef typename boost::function<typename boost_bind_function_type<
        boost::_bi::bind_t<R, F, B> >::type> CompatType;
        CompatType f = v;
        return makeGenericFunction(f);
      }
    };
    template<typename T> struct GenericFunctionMaker<boost::function<T> >
    {
      static GenericFunction make(boost::function<T> func)
      {
         assert(sizeof(boost::function<T>) == sizeof(boost::function<void ()>));
         GenericFunction res;
         res.type = makeFunctionType<T>();
         res.value = res.type->clone(res.type->initializeStorage(&func));
         return res;
      }
    };
    template<typename T> struct GenericFunctionMaker<const T&>
    : public GenericFunctionMaker<T> {};
    template<> struct GenericFunctionMaker<GenericFunction>
    {
      static GenericFunction make(GenericFunction func)
      {
        return func;
      }
    };
  }

  template<typename T>
  GenericFunction makeGenericFunction(T f)
  {
    return detail::GenericFunctionMaker<T>::make(f);
  }

  namespace detail
  {

    // Generate bindFirst(function, arg) that returns a function  binding
    // arg as the first argument, for each argument count.
#define underscoren(z, n, x) BOOST_PP_COMMA_IF(n) BOOST_PP_CAT(_, QI_GEN_SYMINC(n))

#define BindFirst(n, argstypedecl, argstype, argsdecl, argsues, comma)                          \
template<typename R, typename EFF, typename PX comma argstypedecl>                              \
boost::function<R(argstype)> bindFirst(boost::function<R(PX comma argstype)> f, EFF *instance)  \
{                                                                                               \
  return boost::bind(f, boost::ref(*instance) comma BOOST_PP_REPEAT(n, underscoren, _));        \
}

  QI_GEN(BindFirst)

#undef BindFirst
#undef underscoren
  }

template<typename C, typename F> GenericFunction makeGenericFunction(C* inst, F func)
{

  typedef typename ::boost::function_types::function_type<F>::type FType;
  boost::function<FType> memberFunction = func;
  return makeGenericFunction(detail::bindFirst(memberFunction, inst));

}
}
#endif
