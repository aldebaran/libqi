#pragma once
/*
*  Author(s):
*  - Chris  Kilner <ckilner@aldebaran-robotics.com>
*  - Cedric Gestes <gestes@aldebaran-robotics.com>
*
*  Copyright (C) 2010, 2011, 2012 Aldebaran Robotics
*/


#ifndef _QIMESSAGING_SERIALIZATION_SERIALIZE_HPP_
#define _QIMESSAGING_SERIALIZATION_SERIALIZE_HPP_

#include <iostream>
#include <typeinfo>
#include <qimessaging/signature.hpp>
#include <boost/preprocessor/seq/for_each.hpp>
#include <boost/preprocessor/tuple/elem.hpp>

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

#define _QI_SERIALIZATION_REFLECT_ELEM(r, data, elem)  \
  qi::serialization::serialize<BOOST_PP_TUPLE_ELEM(2, 0, elem)>::data(sd, val.BOOST_PP_TUPLE_ELEM(2, 1, elem));

#define QI_SERIALIZATION_REFLECT(TYPE, MEMBERS)                                     \
namespace qi { namespace serialization {                                            \
  template<> struct serialize<TYPE> {                                               \
    static inline void write(qi::DataStream &sd, const TYPE &val) {                    \
      BOOST_PP_SEQ_FOR_EACH(_QI_SERIALIZATION_REFLECT_ELEM, write, MEMBERS)         \
    }                                                                               \
                                                                                    \
    static inline void read(qi::DataStream &sd, TYPE &val) {                           \
      BOOST_PP_SEQ_FOR_EACH(_QI_SERIALIZATION_REFLECT_ELEM, read, MEMBERS)          \
    }                                                                               \
  };                                                                                \
}}

#define QI_SERIALIZATION_ALLOW_VISITOR(TYPE)                                        \
 friend class qi::serialization::serialize<TYPE>;

//Inline this function => they do nothing, they just call DataStream method
//we keep vector/map not inlined at the moment because they take space.
#define QI_SIMPLE_SERIALIZER(Name, Type)                                   \
  template <>                                                              \
  struct serialize<Type>  {                                                \
    static inline void write(qi::DataStream &sd, const Type &val) {               \
      sd.write##Name(val);                                                 \
      __QI_DEBUG_SERIALIZATION_W(Type, val)                                \
    }                                                                      \
    static inline void read(qi::DataStream &sd, Type &val) {                      \
      sd.read##Name(val);                                                  \
      __QI_DEBUG_SERIALIZATION_R(Type, val)                                \
    }                                                                      \
  };


#define QI_LIST_SERIALIZER(TYPE)                                           \
  template<typename U>                                                     \
  struct serialize< TYPE<U> >  {                                           \
    typedef TYPE<U> _typefordebug;                                         \
                                                                           \
    static void write(qi::DataStream &sd, const TYPE<U> &v) {                     \
      sd.writeInt(v.size());                                               \
      if (v.size()) {                                                      \
        typename TYPE<U>::const_iterator it = v.begin();                   \
        typename TYPE<U>::const_iterator end = v.end();                    \
        for (; it != end; ++it) {                                          \
          serialize<U>::write(sd, *it);                                    \
        }                                                                  \
      }                                                                    \
      __QI_DEBUG_SERIALIZATION_CONTAINER_W(_typefordebug, v)               \
    }                                                                      \
    static void write(qi::DataStream &sd, TYPE<U> &v) {                           \
      sd.writeInt(v.size());                                               \
      if (v.size()) {                                                      \
        typename TYPE<U>::iterator it = v.begin();                         \
        typename TYPE<U>::iterator end = v.end();                          \
        for (; it != end; ++it) {                                          \
          serialize<U>::write(sd, *it);                                    \
        }                                                                  \
      }                                                                    \
      __QI_DEBUG_SERIALIZATION_CONTAINER_W(_typefordebug, v);              \
    }                                                                      \
    static void read(qi::DataStream &sd, TYPE<U> &v) {                            \
      int sz;                                                              \
      sd.readInt(sz);                                                      \
      v.clear();                                                           \
      if (sz) {                                                            \
        v.resize(sz);                                                      \
        typename TYPE<U>::iterator it = v.begin();                         \
        typename TYPE<U>::iterator end = v.end();                          \
        for (; it != end; ++it) {                                          \
          serialize<U>::read(sd, *it);                                     \
        }                                                                  \
      }                                                                    \
      __QI_DEBUG_SERIALIZATION_CONTAINER_R(_typefordebug, v);              \
    }                                                                      \
  };


#define QI_MAP_SERIALIZER(TYPE)                                          \
  template<typename K, typename V>                                       \
  struct serialize< TYPE<K, V> >  {                                      \
    typedef TYPE<K, V> _typefordebug;                                    \
                                                                         \
    static void write(qi::DataStream &sd, const TYPE<K, V> &m) {                \
      sd.writeInt(m.size());                                             \
      if (m.size()) {                                                    \
        typename TYPE<K,V>::const_iterator it = m.begin();               \
        typename TYPE<K,V>::const_iterator end = m.end();                \
        for (; it != end; ++it) {                                        \
          serialize<K>::write(sd, it->first);                            \
          serialize<V>::write(sd, it->second);                           \
        }                                                                \
      }                                                                  \
      __QI_DEBUG_SERIALIZATION_CONTAINER_W(_typefordebug, m);            \
    }                                                                    \
                                                                         \
    static void write(qi::DataStream &sd, TYPE<K, V> &m) {                      \
      sd.writeInt(m.size());                                             \
      if (m.size()) {                                                    \
        typename TYPE<K,V>::iterator it = m.begin();                     \
        typename TYPE<K,V>::iterator end = m.end();                      \
        for (; it != end; ++it) {                                        \
          serialize<K>::write(sd, it->first);                            \
          serialize<V>::write(sd, it->second);                           \
        }                                                                \
      }                                                                  \
      __QI_DEBUG_SERIALIZATION_CONTAINER_W(_typefordebug, m);            \
    }                                                                    \
                                                                         \
    static void read(qi::DataStream &sd, TYPE<K, V>  &m) {              \
      int sz;                                                            \
      sd.readInt(sz);                                                    \
      m.clear();                                                         \
                                                                         \
      if (sz) {                                                          \
        for(int i=0; i < sz; ++i) {                                      \
          K k;                                                           \
          V v;                                                           \
          serialize<K>::read(sd, k);                                     \
          serialize<V>::read(sd, v);                                     \
          m[k] = v;                                                      \
        }                                                                \
      }                                                                  \
      __QI_DEBUG_SERIALIZATION_CONTAINER_R(_typefordebug, m);            \
    }                                                                    \
  };



namespace qi {
  namespace serialization {

    /** \class SerializationError serialization.hpp "qi/serialization.hpp"
     *  \brief Thrown when serialization fail
     */
    class QIMESSAGING_API SerializationError : public std::runtime_error
    {
    public:
      explicit SerializationError(const std::string &msg) : std::runtime_error(msg) {}
      virtual ~SerializationError() throw() {}
    };

    /// serialize a c++ type to a message
    /// \ingroup Serialization
    // Enable is need for conditional template specialization
    template <typename T, class Enable = void>
    struct serialize {
      //empty to crash a compile time when the type cant be serialized
    };

  }
}



#include <qimessaging/serialization/serialize_pod.hxx>
#include <qimessaging/serialization/serialize_stl.hxx>
#include <qimessaging/serialization/serialize_value.hxx>

#endif  // _QIMESSAGING_SERIALIZATION_SERIALIZE_HPP_
