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

#define _QI_SERIALIZATION_REFLECT_ELEM(r, data, elem)  \
  qi::serialization::serialize<BOOST_PP_TUPLE_ELEM(2, 0, elem)>::data(sd, val.BOOST_PP_TUPLE_ELEM(2, 1, elem));

#define QI_SERIALIZATION_REFLECT(TYPE, MEMBERS)                                     \
namespace qi { namespace serialization {                                            \
  template<> struct serialize<TYPE> {                                               \
    static inline void write(qi::Message &sd, const TYPE &val) {                    \
      BOOST_PP_SEQ_FOR_EACH(_QI_SERIALIZATION_REFLECT_ELEM, write, MEMBERS)         \
    }                                                                               \
                                                                                    \
    static inline void read(qi::Message &sd, TYPE &val) {                           \
      BOOST_PP_SEQ_FOR_EACH(_QI_SERIALIZATION_REFLECT_ELEM, read, MEMBERS)          \
    }                                                                               \
  };                                                                                \
}}

#define QI_SERIALIZATION_ALLOW_VISITOR(TYPE)                                        \
 friend class qi::serialization::serialize<TYPE>;

//Inline this function => they do nothing, they just call Message method
//we keep vector/map not inlined at the moment because they take space.
#define QI_SIMPLE_SERIALIZER(Name, Type)                                 \
  template <>                                                         \
  struct serialize<Type>  {                                           \
    static inline void write(Message &sd, const Type &val) {   \
      sd.write##Name(val);                                            \
    }                                                                 \
    static inline void read(Message &sd, Type &val) {          \
      sd.read##Name(val);                                             \
    }                                                                 \
  };


namespace qi {
  namespace serialization {

    /// serialize a c++ type to a message
    /// \ingroup Serialization
    // Enable is need for conditional template specialization
    template <typename T, class Enable = void>
    struct serialize {
      //empty to crash a compile time when the type cant be serialized
    };

  }
}

#if 0
#include <iostream>
#define __QI_DEBUG_SERIALIZATION_W(x, extra) {                \
  std::string sig = qi::signature< x >::value();              \
  std::cout << "write(" << sig << ")" << extra << std::endl;  \
}

#define __QI_DEBUG_SERIALIZATION_R(x, extra) {                \
  std::string sig = qi::signature< x >::value();              \
  std::cout << "read (" << sig << ")" << extra << std::endl;  \
}

#define __QI_DEBUG_SERIALIZATION_CONTAINER_W(x, c) {                   \
  std::string sig = qi::signature< x >::value();                       \
  std::cout << "write(" << sig << ") size: " << c.size() << std::endl; \
}

#define __QI_DEBUG_SERIALIZATION_CONTAINER_R(x, c) {                  \
  std::string sig = qi::signature< x >::value();                      \
  std::cout << "read (" << sig << ") size: " << c.size() << std::endl; \
}
#else
# define __QI_DEBUG_SERIALIZATION_W(x, extra)
# define __QI_DEBUG_SERIALIZATION_R(x, extra)
# define __QI_DEBUG_SERIALIZATION_CONTAINER_W(x, c)
# define __QI_DEBUG_SERIALIZATION_CONTAINER_R(x, c)
#endif

#include <qimessaging/serialization/serialize_pod.hxx>
#include <qimessaging/serialization/serialize_stl.hxx>

#endif  // _QI_SERIALIZATION_SERIALIZE_HPP_
