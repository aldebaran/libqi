#pragma once
/*
**  Copyright (C) 2013 Aldebaran Robotics
**  See COPYING for the license
*/

#ifndef _QITYPE_DETAIL_FUNCTIONSIGNATURE_HXX_
#define _QITYPE_DETAIL_FUNCTIONSIGNATURE_HXX_

#include <boost/thread/mutex.hpp>
#include <qi/macro.hpp>

namespace qi {
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

    template<typename T>
    struct RawFunctionSignature
    {
      static qi::Signature makeSigreturn()
      {
        using ResultType = typename boost::function_types::result_type<T>::type;
        return typeOf<ResultType>()->signature();
      }
      static qi::Signature makeSignature()
      {
        std::string   signature;
        signature += '(';
        using ArgsType = typename boost::function_types::parameter_types<T>::type;
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
        using ResultType = typename qi::boost_bind_result_type<boost::_bi::bind_t<R, F, B>>::type;
        return typeOf<ResultType>()->signature();
      }

      static qi::Signature makeSignature()
      {
        std::string   signature;
        signature += '(';
        using ArgsType = typename qi::boost_bind_parameter_types<boost::_bi::bind_t<R, F, B>>::type;
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

    template<typename T>
    struct MemberFunctionSignature
    {
      static qi::Signature makeSigreturn()
      {
        using ResultType = typename boost::function_types::result_type<T>::type;
        return typeOf<ResultType>()->signature();
      }
      static qi::Signature makeSignature()
      {
        // Reconstruct the boost::bind(instance, _1, _2...) signature
        using RetType = typename boost::function_types::result_type<T>::type;
        using MemArgsType = typename boost::function_types::parameter_types<T>::type;
        using ArgsType = typename boost::mpl::pop_front<MemArgsType>::type;
        using EffectiveType = typename boost::mpl::push_front<ArgsType, RetType>::type;
        using type = typename boost::function_types::function_type<EffectiveType>::type;
        return RawFunctionSignature<type>::makeSignature();
      }
    };

    template<typename T>
    struct FunctionSignature
    {
      using Backend = typename boost::mpl::if_<
        typename boost::function_types::is_member_pointer<T>,
        MemberFunctionSignature<T>,
        RawFunctionSignature<T>
        >::type;
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

    template<typename T>
    struct FunctionSignature<boost::function<T> >: public FunctionSignature<T> {};

    template<typename T> inline
    qi::Signature _functionArgumentsSignature()
    {
      std::string sigs;
      sigs += '(';
      using ArgsType = typename boost::function_types::parameter_types<T>::type;
      boost::mpl::for_each<
        boost::mpl::transform_view<ArgsType,
        boost::add_pointer<
        boost::remove_const<
        boost::remove_reference<boost::mpl::_1>>>>> (qi::detail::signature_function_arg_apply(&sigs));
      sigs += ')';
      return Signature(sigs);
    }
    template<typename T> inline
    qi::Signature functionArgumentsSignature()
    {
      static Signature* res;
      QI_ONCE(res = new Signature(_functionArgumentsSignature<T>()));
      return *res;
    }
  }
}


#endif  // _QITYPE_DETAIL_FUNCTIONSIGNATURE_HXX_
