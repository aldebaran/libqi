/*
*  Author(s):
*  - Chris  Kilner <ckilner@aldebaran-robotics.com>
*  - Cedric Gestes <gestes@aldebaran-robotics.com>
*
*  Copyright (C) 2010 Aldebaran Robotics
*/


#pragma once
#ifndef _QIMESSAGING_DETAILS_TYPE_SIGNATURE_HPP_
#define _QIMESSAGING_DETAILS_TYPE_SIGNATURE_HPP_

# include <string>
# include <boost/cstdint.hpp>
# include <vector>
# include <map>
# include <boost/utility.hpp>
# include <boost/function_types/is_function_pointer.hpp>
# include <boost/function_types/is_function.hpp>
# include <qimessaging/value.hpp>
# include <qi/types.hpp>
//# include <qimessaging/message.hpp>

namespace qi {


  namespace detail {

#define _QI_SIMPLE_SIGNATURE(THETYPE, THENAME)                  \
  template <>                                                   \
  struct signature<THETYPE>  {                                  \
    static std::string &value(std::string &val) {               \
      val += (char)(THENAME);                                   \
      return val;                                               \
    }                                                           \
  };

#define _QI_LIST_SIGNATURE(TYPE)                       \
  template <typename U>                                \
  struct signature< TYPE<U> > {                        \
    static std::string &value(std::string &val) {      \
      val += (char)qi::Signature::Type_List;           \
      qi::detail::signature<U>::value(val);            \
      val += (char)qi::Signature::Type_List_End;       \
      return val;                                      \
    }                                                  \
  };

#define _QI_MAP_SIGNATURE(TYPE)                        \
  template <typename T1, typename T2>                  \
  struct signature< TYPE<T1, T2> > {                   \
    static std::string &value(std::string &val) {      \
      val += (char)qi::Signature::Type_Map;            \
      qi::detail::signature<T1>::value(val);           \
      qi::detail::signature<T2>::value(val);           \
      val += (char)qi::Signature::Type_Map_End;        \
      return val;                                      \
    }                                                  \
  };



    _QI_SIMPLE_SIGNATURE(void          , qi::Signature::Type_Void);
    _QI_SIMPLE_SIGNATURE(bool          , qi::Signature::Type_Bool);
    _QI_SIMPLE_SIGNATURE(char          , qi::Signature::Type_Int8);
    _QI_SIMPLE_SIGNATURE(unsigned char , qi::Signature::Type_UInt8);
    _QI_SIMPLE_SIGNATURE(qi::int16_t   , qi::Signature::Type_Int16);
    _QI_SIMPLE_SIGNATURE(qi::uint16_t  , qi::Signature::Type_UInt16);
    _QI_SIMPLE_SIGNATURE(qi::int32_t   , qi::Signature::Type_Int32);
    _QI_SIMPLE_SIGNATURE(qi::uint32_t  , qi::Signature::Type_UInt32);
    _QI_SIMPLE_SIGNATURE(qi::int64_t   , qi::Signature::Type_Int64);
    _QI_SIMPLE_SIGNATURE(qi::uint64_t  , qi::Signature::Type_UInt64);
    _QI_SIMPLE_SIGNATURE(float         , qi::Signature::Type_Float);
    _QI_SIMPLE_SIGNATURE(double        , qi::Signature::Type_Double);
    _QI_SIMPLE_SIGNATURE(qi::Value     , qi::Signature::Type_Dynamic);

    //pointer
    template <typename T>
    struct signature<T*, typename boost::disable_if< boost::function_types::is_function<T> >::type> {
      static std::string &value(std::string &val) {
        std::string sub;
        std::string sigc;
        qi::detail::signature<char>::value(sigc);
        qi::detail::signature<T>::value(sub);
        if (sub == sigc)
          val += qi::Signature::Type_String;
        else
          val += sub + "*";
        return val;
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

#endif  // _QIMESSAGING_DETAILS_TYPE_SIGNATURE_HPP_
