/*
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2010 Aldebaran Robotics
*/

#ifndef   __QI_SIGNATURE_DETAIL_FUNCTIONSIGNATURECOMPILETIME_HPP__
#define   __QI_SIGNATURE_DETAIL_FUNCTIONSIGNATURECOMPILETIME_HPP__

#include <boost/utility.hpp>
#include <boost/function_types/function_arity.hpp>
#include <boost/function_types/parameter_types.hpp>
#include <boost/function_types/result_type.hpp>

#include <boost/function_types/is_function_pointer.hpp>
#include <boost/function_types/is_function.hpp>
#include <boost/function_types/is_function_reference.hpp>
#include <boost/function_types/is_member_function_pointer.hpp>
#include <boost/function_types/function_type.hpp>

#include <boost/mpl/copy.hpp>
#include <boost/mpl/greater.hpp>
#include <boost/mpl/string.hpp>

#include <boost/type_traits.hpp>

namespace qi {
  namespace detail {

    //efficient function signature (signature is generated at compile time)
    template <typename F>
    struct signature<F, typename boost::enable_if< boost::function_types::is_function<F> >::type> {
    private:
      //function's parameter list of type
      typedef typename boost::function_types::parameter_types<F>::type ArgsType;

      //helper function that get the type of an args if the arg exists
      //this will concat the result (if available) to Old, returning <Old><newtype>
      //count is the parameter number
      template <int count, typename Old>
      struct signature_helper {
        typedef typename boost::mpl::greater< boost::mpl::int_<boost::function_types::function_arity<F>::value>,
          boost::mpl::int_<count>
        >::type makeWork;
        typedef typename boost::mpl::copy< typename signature< typename boost::mpl::at<ArgsType, boost::mpl::int_<count> >::type >::value,
          boost::mpl::back_inserter< Old >
        >::type New;

        typedef typename boost::mpl::if_< makeWork,
          New,
          Old
        >::type value;
      };

      //signature of the return type
      typedef typename signature< typename boost::function_types::result_type<F>::type >::value returnValue;
      //add :
      typedef typename boost::mpl::copy<boost::mpl::string<':'>::type,
        boost::mpl::back_inserter< returnValue >
      >::type returnValueColon;
      //get the signature of each param
      typedef typename signature_helper<0, returnValueColon>::value value0;
      typedef typename signature_helper<1, value0>::value value1;
      typedef typename signature_helper<2, value1>::value value2;
      typedef typename signature_helper<3, value2>::value value3;
      typedef typename signature_helper<4, value3>::value value4;
      typedef typename signature_helper<5, value4>::value value5;
      typedef typename signature_helper<6, value5>::value value6;
      typedef typename signature_helper<7, value6>::value value7;
      typedef typename signature_helper<8, value7>::value value8;
    public:
      //this is the return value
      typedef typename signature_helper<9, value8>::value value;
    };

    //if this is a function dont store the pointer
    template <typename T>
    struct signature<T*, typename boost::enable_if< boost::function_types::is_function<T> >::type> {
      typedef typename signature< T >::value value;
      //To debug (yeah debug code is useless even commented, but may you find it easily ourself? )
      //      typedef typename signature< T >::value returnValue;
      //      typedef typename boost::mpl::copy<boost::mpl::string<'R-R'>::type,
      //                                        boost::mpl::back_inserter< returnValue >
      //                                       >::type value;
    };

    template <typename T>
    struct signature<T, typename boost::enable_if< boost::function_types::is_member_function_pointer<T> >::type> {
    private:
      typedef typename boost::function_types::result_type<T>::type          ResultType;
      typedef typename boost::function_types::parameter_types<T>::type      MemArgsType;

      typedef typename boost::mpl::pop_front< MemArgsType >::type           ArgsType;
      typedef typename boost::mpl::push_front< ArgsType, ResultType >::type ReturnArgsType;
    public:
      typedef typename signature< typename boost::function_types::function_type<ReturnArgsType>::type >::value value;
    };
  }

}

#endif  // QI_FUNCTORS_DETAIL_FUNCTIONSIGNATURECOMPILETIME_HPP_






//PLAYGROUND
#if 0
//this is my prefered solution, but impossible to get working (boost::mpl::_2 is not "detypified to the right simple type")
template <typename F>
struct newFunctionSignature {
  typedef typename newTypeSignature< typename boost::function_types::result_type<F>::type >::value returnValue;

  typedef typename boost::mpl::copy<boost::mpl::string<':'>::type,
    boost::mpl::back_inserter< returnValue >
  >::type returnValueColon;

  typedef typename boost::function_types::parameter_types<F>::type ArgsType;


  typedef typename boost::mpl::iter_fold<ArgsType,
    typename boost::mpl::string<>,
    typename boost::mpl::copy<newTypeSignature< typename boost::mpl::deref<boost::mpl::_2 > >::value,
    boost::mpl::back_inserter< boost::mpl::_1 >
    >
  >::type argsValue;
  typedef typename boost::mpl::copy<argsValue,
    boost::mpl::back_inserter< returnValueColon >
  >::type value;
};
#endif // __QI_SIGNATURE_DETAIL_FUNCTIONSIGNATURECOMPILETIME_HPP__

