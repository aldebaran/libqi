#pragma once
/*
*  Author(s):
*  - Chris  Kilner <ckilner@aldebaran-robotics.com>
*  - Cedric Gestes <gestes@aldebaran-robotics.com>
*
*  Copyright (C) 2010 Aldebaran Robotics
*/


#ifndef _QI_SERIALIZATION_SERIALIZE_HPP_
#define _QI_SERIALIZATION_SERIALIZE_HPP_

#include <iostream>
#include <typeinfo>
#include <qimessaging/signature.hpp>
#include <boost/preprocessor/seq/for_each.hpp>
#include <boost/preprocessor/tuple/elem.hpp>

namespace qi {
  namespace serialization {

    /// serialize a c++ type to a message
    /// \ingroup Serialization
    //Enable is need for protobuf (for conditional template specialization)
    template <typename T, class Enable = void>
    struct serialize {
      static void read(Message &sd, T &t){
        std::cout << "ERROR: The type \"" << typeid(T).name() << "\" is not serializable." << std::endl;
        //#error "This type is not serializable"
      }

      static void write(Message &sd, const T &t) {
        std::cout << "ERROR: The type \"" << typeid(T).name() << "\" is not serializable." << std::endl;
        //#error "This type is not serializable"
      }

      static void signature(std::string &sign, const T &t) {
        qi::signatureFromObject::value(sign, t);
      }
    };

  }
}

#define _QI_REFLECT_SERIALIZE_ELEM(r, data, elem)  \
  qi::serialization::serialize<BOOST_PP_TUPLE_ELEM(2, 0, elem)>::data(sd, val.BOOST_PP_TUPLE_ELEM(2, 1, elem));

#define _QI_REFLECT_SIGNATURE_ELEM(r, data, elem)  \
  qi::detail::signature<BOOST_PP_TUPLE_ELEM(2, 0, elem)>::value(val);

#define QI_REFLECT_SIGNATURE(TYPE, MEMBERS)                                         \
namespace qi { namespace detail {                                                   \
  template <>                                                                       \
    struct signature<TYPE>  {                                                       \
    static std::string &value(std::string &val) {                                   \
      val += "(";                                                                   \
      BOOST_PP_SEQ_FOR_EACH(_QI_REFLECT_SIGNATURE_ELEM, none, MEMBERS)                   \
      val += ")";                                                                   \
      return val;                                                                   \
    }                                                                               \
  };                                                                                \
}}                                                                                  \

#define QI_REFLECT_SERIALIZATION(TYPE, MEMBERS)                                     \
namespace qi { namespace serialization {                                            \
  template<> struct serialize<TYPE> {                                               \
    static inline void write(qi::Message &sd, const TYPE &val) {                    \
      BOOST_PP_SEQ_FOR_EACH(_QI_REFLECT_SERIALIZE_ELEM, write, MEMBERS)             \
    }                                                                               \
                                                                                    \
    static inline void read(qi::Message &sd, TYPE &val) {                           \
      BOOST_PP_SEQ_FOR_EACH(_QI_REFLECT_SERIALIZE_ELEM, read, MEMBERS)              \
    }                                                                               \
  };                                                                                \
}}

#define QI_REFLECT(TYPE, MEMBERS)                                                   \
  QI_REFLECT_SIGNATURE(TYPE, MEMBERS);                                              \
  QI_REFLECT_SERIALIZATION(TYPE, MEMBERS);

#include <qimessaging/serialization/serialize.hxx>

#endif  // _QI_SERIALIZATION_SERIALIZE_HPP_
