#pragma once
/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/

#ifndef _QIMESSAGING_DETAILS_FUNCTIONTYPE_HXX_
#define _QIMESSAGING_DETAILS_FUNCTIONTYPE_HXX_

#ifdef BOOST_FUSION_INVOKE_FUNCTION_OBJECT_MAX_ARITY
# undef BOOST_FUSION_INVOKE_FUNCTION_OBJECT_MAX_ARITY
#endif
#define BOOST_FUSION_INVOKE_FUNCTION_OBJECT_MAX_ARITY 10

#include <boost/fusion/include/mpl.hpp>
#include <boost/mpl/for_each.hpp>
#include <boost/mpl/transform_view.hpp>
#include <boost/type_traits/remove_reference.hpp>
#include <boost/type_traits/add_pointer.hpp>
#include <boost/type_traits/remove_const.hpp>
#include <boost/type_traits/remove_pointer.hpp>
#include <boost/function_types/function_type.hpp>
#include <boost/function_types/result_type.hpp>
#include <boost/function_types/parameter_types.hpp>
#include <boost/fusion/container/vector/convert.hpp>
#include <boost/fusion/include/as_vector.hpp>
#include <boost/fusion/include/as_list.hpp>
#include <boost/fusion/algorithm/transformation/transform.hpp>
#include <boost/fusion/include/transform.hpp>
#include <boost/fusion/functional/invocation/invoke_function_object.hpp>
#include <boost/fusion/container/generation/make_vector.hpp>
#include <boost/fusion/include/make_vector.hpp>
#include <boost/fusion/algorithm/iteration/for_each.hpp>
#include <boost/fusion/functional/adapter/unfused.hpp>
#include <boost/fusion/functional/generation/make_unfused.hpp>
#include <boost/fusion/functional/generation/make_fused.hpp>
#include <qimessaging/genericvalue.hpp>

namespace qi
{
  inline Type* CallableType::resultType()
  {
    return _resultType;
  }

  inline const std::vector<Type*>& CallableType::argumentsType()
  {
    return _argumentsType;
  }

  inline GenericValue GenericFunction::operator()(const std::vector<GenericValue>& args)
  {
    return call(args);
  }

  namespace detail
  {
    struct PtrToConstRef
    {
      // Drop the const, it prevents method calls from working
      template <typename Sig>
      struct result;

      template <class Self, typename T>
      struct result< Self(T) >
      {
        typedef typename boost::add_reference<
        //typename boost::add_const<
        typename boost::remove_pointer<
        typename boost::remove_reference<T>::type
        >::type
        //  >::type
        >::type type;
      };
      template<typename T>
      T& operator() (T* const &ptr) const
      {
        // Careful here, a wrong cast will create a variable on the stack, but
        // we need to pass &ptr
        void* res  = typeOf<T>()->ptrFromStorage((void**)&ptr);
        return *(T*)res;
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
        target->push_back(typeOf<
          typename remove_constptr<
            typename boost::remove_const<
               typename boost::remove_reference<T>::type
            >::type>::type>());
      }
      std::vector<Type*>* target;
    };

    struct Transformer
    {
    public:
      inline Transformer(const std::vector<void*>* args)
      : args(args)
      , pos(0)
      {}
      template <typename Sig>
      struct result;

      template <class Self, typename T>
      struct result< Self(T) >
      {
        typedef T type;
      };
      template<typename T>
      void
      operator() (T* &v) const
      {
        v = (T*)(*args)[pos++];
      }
      const std::vector<void*> *args;
      mutable unsigned int pos;
    };

    template<typename SEQ, typename F> void* apply(SEQ sequence,
      F& function, const std::vector<void*> args)
    {
      GenericValueCopy res;
      boost::fusion::for_each(sequence, Transformer(&args));
      res(), boost::fusion::invoke_function_object(function,
        boost::fusion::transform(sequence,
          PtrToConstRef()));
      return res.value;
    }
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
  } // namespace detail


  template<typename T> class FunctionTypeImpl:
  public FunctionType
  {
  public:
    FunctionTypeImpl()
    {
      _resultType = typeOf<typename boost::function_types::result_type<T>::type >();

      typedef typename boost::function_types::parameter_types<T>::type ArgsType;
      // Detect and warn about non-const reference arguments
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
    virtual void* call(void* func, const std::vector<void*>& args)
    {
      boost::function<T>* f = (boost::function<T>*)func;
      typedef typename boost::function_types::parameter_types<T>::type ArgsType;
      typedef typename  boost::mpl::transform_view<ArgsType,
      boost::remove_const<
      boost::remove_reference<boost::mpl::_1> > >::type BareArgsType;
      typedef typename boost::mpl::transform_view<BareArgsType,
      boost::add_pointer<boost::mpl::_1> >::type PtrArgsType;
      return detail::apply(boost::fusion::as_vector(PtrArgsType()), *f, args);
    }
    _QI_BOUNCE_TYPE_METHODS(DefaultTypeImplMethods<boost::function<T> >);
  };

  template<typename T> FunctionType* makeFunctionType()
  {
    static FunctionTypeImpl<T> result;
    return &result;
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
    template<typename T> struct GenericFunctionMaker<boost::function<T> >
    {
      static GenericFunction make(boost::function<T> func)
      {
         assert(sizeof(boost::function<T>) == sizeof(boost::function<void ()>));
         GenericFunction res;
         *(boost::function<T>*)(void*)&res.value = func;
         res.type = makeFunctionType<T>();
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
  GenericFunction makeGenericFunction(const T& f)
  {
    return detail::GenericFunctionMaker<T>::make(f);
  }


namespace detail
{
  /* Call a boost::function<F> binding the first argument.
  * Can't be done just with boost::bind without code generation.
  */
  template<typename F>
  struct FusedBindOne
  {
    template <class Seq>
    struct result
    {
      typedef typename boost::function_types::result_type<F>::type type;
    };

    template <class Seq>
    typename result<Seq>::type
    operator()(Seq const & s) const
    {
      return ::boost::fusion::invoke_function_object(func,
        ::boost::fusion::push_front(s, boost::ref(const_cast<ArgType&>(*arg1))));
    }
    ::boost::function<F> func;
    typedef typename boost::remove_reference<
      typename ::boost::mpl::front<
        typename ::boost::function_types::parameter_types<F>::type
        >::type>::type ArgType;
    void setArg(ArgType* val) { arg1 = val;}
    ArgType* arg1;

  };

}

template<typename C, typename F> GenericFunction makeGenericFunction(C* inst, F func)
{
  // Return type
  typedef typename ::boost::function_types::result_type<F>::type RetType;
  // All arguments including class pointer
  typedef typename ::boost::function_types::parameter_types<F>::type MemArgsType;
  // Pop class pointer
  typedef typename ::boost::mpl::pop_front< MemArgsType >::type ArgsType;
  // Synthethise exposed function type
  typedef typename ::boost::mpl::push_front<ArgsType, RetType>::type ResultMPLType;
  typedef typename ::boost::function_types::function_type<ResultMPLType>::type ResultType;
  // Synthethise non-member function equivalent type of F
  typedef typename ::boost::mpl::push_front<MemArgsType, RetType>::type MemMPLType;
  typedef typename ::boost::function_types::function_type<MemMPLType>::type LinearizedType;
  // See func as R (C*, OTHER_ARGS)
  boost::function<LinearizedType> memberFunction = func;
  boost::function<ResultType> res;
  // Create the fusor
  detail::FusedBindOne<LinearizedType> fusor;
  // Bind member function and instance
  fusor.setArg(inst);
  fusor.func = memberFunction;
  // Convert it to a boost::function
  res = boost::fusion::make_unfused(fusor);

  return makeGenericFunction(res);
}

} // namespace qi
#endif  // _QIMESSAGING_DETAILS_FUNCTIONTYPE_HXX_
