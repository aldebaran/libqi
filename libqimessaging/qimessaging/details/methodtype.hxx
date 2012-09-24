#pragma once
/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/

#ifndef _QIMESSAGING_DETAILS_METHODTYPE_HXX_
#define _QIMESSAGING_DETAILS_METHODTYPE_HXX_

#include <boost/function_types/function_type.hpp>
#include <boost/function_types/result_type.hpp>
#include <boost/function_types/parameter_types.hpp>

#include <boost/fusion/functional/generation/make_unfused.hpp>
#include <boost/fusion/functional/invocation/invoke_function_object.hpp>

namespace qi
{
  namespace detail
  {
    // Convert method signature to function signature by putting
    // "class instance pointer" type as first argument
    // ex: int (Foo::*)(int) => int (*)(Foo*, int)
    template<typename F>
    struct MethodToFunctionTrait
    {
      // Result type
      typedef typename ::boost::function_types::result_type<F>::type RetType;
      // All arguments including class pointer
      typedef typename ::boost::function_types::parameter_types<F>::type ArgsType;
      // Class ref type
      typedef typename ::boost::mpl::front<ArgsType>::type ClassRefType;
      // Convert it to ptr type
      typedef typename boost::add_pointer<typename boost::remove_reference<ClassRefType>::type>::type ClassPtrType;
      // Argument list, changing ClassRef to ClassPtr
      typedef typename boost::mpl::push_front<
        typename ::boost::mpl::pop_front<ArgsType>::type,
      ClassRefType>::type ArgsTypeFixed;
      // Push result type in front
      typedef typename ::boost::mpl::push_front<ArgsTypeFixed, RetType>::type FullType;
      // Synthetise result function type
      typedef typename ::boost::function_types::function_type<FullType>::type type;
      // Compute bound type
      typedef typename boost::mpl::push_front<
        typename boost::mpl::pop_front<ArgsType>::type, RetType>::type BoundTypeSeq;
      // Synthetise method type
      typedef typename ::boost::function_types::function_type<BoundTypeSeq>::type BoundType;
    };
  } // namespace detail

  template<typename T>
  class MethodTypeImpl:
    public virtual MethodType,
    public virtual FunctionTypeImpl<T>
  {
    void* call(void* method, void* object,
      const std::vector<void*>& args)
    {
      std::vector<void*> nargs;
      nargs.reserve(args.size()+1);
      nargs.push_back(object);
      nargs.insert(nargs.end(), args.begin(), args.end());
      return FunctionTypeImpl<T>::call(method, nargs);
    }
    GenericValue call(void* method, GenericValue object,
      const std::vector<GenericValue>& args)
    {
      std::vector<GenericValue> nargs;
      nargs.reserve(args.size()+1);
      nargs.push_back(object);
      nargs.insert(nargs.end(), args.begin(), args.end());
      return FunctionType::call(method, nargs);
    }
  };

  template<typename T>
  MethodType* methodTypeOf()
  {
    static MethodTypeImpl<T> result;
    return &result;
  }

  namespace detail
  {

    // Drop first arg and make the call
    template<typename T> struct DropArg
    {
      template <class Seq>
      struct result
      {
        typedef typename boost::function_types::result_type<T>::type type;
      };
      template <class Seq>
      typename result<Seq>::type
      operator()(Seq const & s) const
      {
        return boost::fusion::invoke_function_object(fun,
          boost::fusion::pop_front(s));
      };
      DropArg(boost::function<T> fun) : fun(fun) {}
      boost::function<T> fun;
    };

    template<typename T>
    GenericMethod makeGenericMethodSwitch(const T& method, boost::true_type)
    {
      // convert M to a boost::function with an extra arg object_type
      typedef typename detail::MethodToFunctionTrait<T>::type Linearized;
      boost::function<Linearized> f = method;

      GenericFunction fv = makeGenericFunction(f);
      GenericMethod result;
      result.value = fv.value;
      result.type = methodTypeOf<Linearized>();
      return result;
    }

    template<typename T>
    GenericMethod makeGenericMethodSwitch(T method, boost::false_type)
    {
      typedef typename boost::function_types::function_type<
      typename boost::function_types::components<T>::type>::type FunctionType;
      typedef typename boost::function_types::parameter_types<FunctionType >::type ArgsType;
      // We will drop the first argument, but the system will still try to
      // convert it, so use an always-compatible conversion target: GenericValue
      typedef typename boost::mpl::push_front<ArgsType, GenericValue>::type NArgsType;
      typedef typename boost::function_types::result_type<T>::type ResType;
      typedef typename boost::mpl::push_front<NArgsType, ResType>::type FullType;
      typedef typename boost::function_types::function_type<FullType>::type DropperType;
      boost::function<FunctionType> bmethod (method);
      boost::function<DropperType> f = boost::fusion::make_unfused(DropArg<FunctionType>(bmethod));
      GenericMethod result;
      result.type = methodTypeOf<DropperType>();
      result.value = new boost::function<DropperType>(f);
      return result;
    }
  }

  template<typename M>
  GenericMethod makeGenericMethod(const M& method)
  {
    // Handle static methods
    return detail::makeGenericMethodSwitch<M>(method,
      typename boost::function_types::is_member_function_pointer<M>::type());
  }

  inline GenericFunction GenericMethod::toGenericFunction()
  {
    GenericFunction res;
    res.type = dynamic_cast<FunctionType*>(type);
    res.value = value;
    return res;
  }

} // namespace qi

#endif  // _QIMESSAGING_DETAILS_METHODTYPE_HXX_
