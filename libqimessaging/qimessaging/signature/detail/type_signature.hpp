#pragma once
/*
*  Author(s):
*  - Chris  Kilner <ckilner@aldebaran-robotics.com>
*  - Cedric Gestes <gestes@aldebaran-robotics.com>
*
*  Copyright (C) 2010 Aldebaran Robotics
*/


#ifndef _QI_SIGNATURE_DETAIL_TYPE_SIGNATURE_HPP_
#define _QI_SIGNATURE_DETAIL_TYPE_SIGNATURE_HPP_

# include <string>
# include <vector>
# include <map>
# include <boost/utility.hpp>
# include <boost/function_types/is_function_pointer.hpp>
# include <boost/function_types/is_function.hpp>
# include <qimessaging/serialization/message.hpp>

namespace qi {
  namespace detail {

#define _QI_SIMPLE_SIGNATURE(THETYPE, THENAME)                  \
  template <>                                                   \
  struct signature<THETYPE>  {                                  \
    static std::string &value(std::string &val) {               \
      val += THENAME;                                           \
      return val;                                               \
    }                                                           \
  };

#define _QI_LIST_SIGNATURE(TYPE)                       \
  template <typename U>                                \
  struct signature< TYPE<U> > {                        \
    static std::string &value(std::string &val) {      \
      val += "[";                                      \
      qi::detail::signature<U>::value(val);            \
      val += "]";                                      \
      return val;                                      \
    }                                                  \
  };

#define _QI_MAP_SIGNATURE(TYPE)                        \
  template <typename T1, typename T2>                  \
  struct signature< TYPE<T1, T2> > {                   \
    static std::string &value(std::string &val) {      \
      val += "{";                                      \
      qi::detail::signature<T1>::value(val);           \
      qi::detail::signature<T2>::value(val);           \
      val += "}";                                      \
      return val;                                      \
    }                                                  \
  };


    template <typename T, class Enable = void>
    struct signature {
      static std::string &value(std::string &val) {
        val += "UNKNOWN";
        return val;
      }
    };

    _QI_SIMPLE_SIGNATURE(void       , "v");
    _QI_SIMPLE_SIGNATURE(bool       , "b");
    _QI_SIMPLE_SIGNATURE(char       , "c");
    _QI_SIMPLE_SIGNATURE(int        , "i");
    _QI_SIMPLE_SIGNATURE(float      , "f");
    _QI_SIMPLE_SIGNATURE(double     , "d");
    _QI_SIMPLE_SIGNATURE(qi::Message, "m");

    //pointer
    template <typename T>
    struct signature<T*, typename boost::disable_if< boost::function_types::is_function<T> >::type> {
      static std::string &value(std::string &val) {
        qi::detail::signature<T>::value(val);
        val += "*"; return val;
      }
    };

    //& and const are not used for function signature, it's only a compiler detail
    template <typename T>
    struct signature<T&> {
      static std::string &value(std::string &val) {
        return qi::detail::signature<T>::value(val);
      }
    };

    template <typename T>
    struct signature<const T> {
      static std::string &value(std::string &val) {
        return qi::detail::signature<T>::value(val);
      }
    };

  }
}

#endif  // _QI_SIGNATURE_DETAIL_TYPE_SIGNATURE_HPP_
