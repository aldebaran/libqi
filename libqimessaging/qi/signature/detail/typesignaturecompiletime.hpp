#pragma once
/*
*  Author(s):
*  - Cedric Gestes <gestes@aldebaran-robotics.com>
*  - Chris  Kilner <ckilner@aldebaran-robotics.com>
*
*  Copyright (C) 2010 Aldebaran Robotics
*/


#ifndef _QI_SIGNATURE_DETAIL_TYPESIGNATURECOMPILETIME_HPP_
#define _QI_SIGNATURE_DETAIL_TYPESIGNATURECOMPILETIME_HPP_

# include <string>
# include <vector>
# include <map>
# include <boost/mpl/string.hpp>
# include <boost/utility.hpp>
# include <boost/function_types/is_function_pointer.hpp>
# include <boost/function_types/is_function.hpp>
namespace qi {
    namespace detail {

#define _QI_SIMPLE_SIGNATURE(THETYPE, THENAME)           \
  template <>                                        \
      struct signature<THETYPE>  {                   \
      typedef boost::mpl::string<THENAME> value;     \
      };

      template <typename T, class Enable = void>
      struct signature {
        typedef boost::mpl::string<'U', 'N', 'K', 'N', 'O', 'W', 'N'> value;
      };

      _QI_SIMPLE_SIGNATURE(void       , 'v');
      _QI_SIMPLE_SIGNATURE(bool       , 'b');
      _QI_SIMPLE_SIGNATURE(char       , 'c');
      _QI_SIMPLE_SIGNATURE(int        , 'i');
      _QI_SIMPLE_SIGNATURE(float      , 'f');
      _QI_SIMPLE_SIGNATURE(double     , 'd');
      _QI_SIMPLE_SIGNATURE(std::string, 's');

      //pointer
      template <typename T>
      struct signature<T*, typename boost::disable_if< boost::function_types::is_function<T> >::type> {
        typedef typename boost::mpl::copy<boost::mpl::string<'*'>::type,
          boost::mpl::back_inserter< typename signature<T>::value >
        >::type value;
      };

      //& and const are not used for function signature, it's only a compiler detail
      template <typename T>
      struct signature<T&> {
        typedef typename signature<T>::value value;
      };

      template <typename T>
      struct signature<const T> {
        typedef typename signature<T>::value value;
      };

      //STL
      template <typename U>
      struct signature< std::vector<U> > {
      private:
        typedef typename boost::mpl::copy<typename signature<U>::value,
          boost::mpl::back_inserter< boost::mpl::string<'['>::type >
        >::type value0;
      public:
        typedef typename boost::mpl::copy<boost::mpl::string<']'>::type,
          boost::mpl::back_inserter< value0 >
        >::type value;
      };

      template <typename T1, typename T2>
      struct signature< std::map<T1, T2> > {
      private:
        typedef typename boost::mpl::copy<typename signature<T1>::value,
          boost::mpl::back_inserter< boost::mpl::string<'{'>::type >
        >::type value0;
        typedef typename boost::mpl::copy<typename signature<T2>::value,
          boost::mpl::back_inserter< value0 >
        >::type value1;
      public:
        typedef typename boost::mpl::copy<boost::mpl::string<'}'>::type,
          boost::mpl::back_inserter< value1 >
        >::type value;
      };

    }
}

#endif  // _QI_SIGNATURE_DETAIL_TYPESIGNATURECOMPILETIME_HPP_
