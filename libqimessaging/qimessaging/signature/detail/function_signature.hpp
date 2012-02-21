#pragma once
/*
*  Author(s):
*  - Chris  Kilner <ckilner@aldebaran-robotics.com>
*  - Cedric Gestes <gestes@aldebaran-robotics.com>
*
*  Copyright (C) 2010 Aldebaran Robotics
*/


#ifndef _QIMESSAGING_SIGNATURE_DETAIL_FUNCTION_SIGNATURE_HPP_
#define _QIMESSAGING_SIGNATURE_DETAIL_FUNCTION_SIGNATURE_HPP_

#include <boost/utility.hpp>
#include <boost/function_types/function_arity.hpp>
#include <boost/function_types/parameter_types.hpp>
#include <boost/function_types/result_type.hpp>

#include <boost/function_types/is_function_pointer.hpp>
#include <boost/function_types/is_function.hpp>
#include <boost/function_types/is_function_reference.hpp>
#include <boost/function_types/is_member_function_pointer.hpp>
#include <boost/function_types/function_type.hpp>

//#include <boost/mpl/copy.hpp>
#include <boost/mpl/for_each.hpp>
#include <boost/mpl/transform_view.hpp>

//#include <boost/mpl/greater.hpp>
//#include <boost/mpl/string.hpp>
#include <boost/type_traits.hpp>

namespace qi {
  namespace detail {

    struct signature_function_arg_apply {
      signature_function_arg_apply(std::string &val) : val(val) {}
      template<typename T> void operator()(T x)    { (void) x; qi::detail::signature<T>::value(val); }
      std::string &val;
    };

    //function signature
    template <typename F>
    struct signature<F, typename boost::enable_if< boost::function_types::is_function<F> >::type> {
      static std::string &value(std::string &val) {
        typedef typename boost::function_types::parameter_types<F>::type ArgsType;
        ::qi::detail::signature< typename boost::function_types::result_type<F>::type >::value(val);
        val += "(";
        boost::mpl::for_each< boost::mpl::transform_view<ArgsType, boost::remove_reference<boost::mpl::_1> > >(signature_function_arg_apply(val));
        val += ")";
        //        for (i = 0; i < boost::function_types::function_arity<F>::value; ++i) {
        //          qi::detail::signature<typename boost::mpl::at<ArgsType, boost::mpl::int_<i> >::type>::value(val);
        //        }
        return val;
      }
    };

    //if this is a function dont store the pointer
    template <typename T>
    struct signature<T*, typename boost::enable_if< boost::function_types::is_function<T> >::type> {
      static std::string &value(std::string &val) {
        return qi::detail::signature<T>::value(val);
      }
    };

    template <typename T>
    struct signature<T, typename boost::enable_if< boost::function_types::is_member_function_pointer<T> >::type> {
      static std::string &value(std::string &val) {
        typedef typename boost::function_types::result_type<T>::type          ResultType;
        typedef typename boost::function_types::parameter_types<T>::type      MemArgsType;
        typedef typename boost::mpl::pop_front< MemArgsType >::type           ArgsType;
        typedef typename boost::mpl::push_front< ArgsType, ResultType >::type ReturnArgsType;
        return qi::detail::signature<typename boost::function_types::function_type<ReturnArgsType>::type>::value(val);
      }
    };
  }
}

#endif // QI_FUNCTORS_DETAIL_FUNCTIONSIGNATURE_HPP_

