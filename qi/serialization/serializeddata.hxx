/*
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**  - Chris KILNER  <ckilner@aldebaran-robotics.com>
**
** Copyright (C) 2010 Aldebaran Robotics
*/


#ifndef   __QI_SERIALIZATION_SERIALIZEDDATA_HXX__
#define   __QI_SERIALIZATION_SERIALIZEDDATA_HXX__

#if 0
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

#include <protobuf/message.h>
//#include <vector>

namespace qi {
  namespace serialization {


//Inline this function => they do nothing, they just call SerializedData method
//we keep vector/map not inlined at the moment because they take space.
#define QI_SIMPLE_SERIALIZER(Name, Type)                                 \
  template <>                                                         \
  struct serialize<Type>  {                                           \
    static inline void write(SerializedData &sd, const Type &val) {   \
      sd.write##Name(val);                                            \
    }                                                                 \
    static inline void read(SerializedData &sd, Type &val) {          \
      sd.read##Name(val);                                             \
    }                                                                 \
  };

    QI_SIMPLE_SERIALIZER(Bool, bool);
    QI_SIMPLE_SERIALIZER(Char, char);
    QI_SIMPLE_SERIALIZER(Int, int);
    QI_SIMPLE_SERIALIZER(Float, float);
    QI_SIMPLE_SERIALIZER(String, std::string);


    template <typename T>
    struct serialize<T&> {
      static inline void write(SerializedData &sd, const T &val) {
        __QI_DEBUG_SERIALIZATION_W(T, "&");
        serialize<T>::write(sd, val);
      }
      static inline void read(SerializedData &sd, T &val) {
        __QI_DEBUG_SERIALIZATION_R(T, "&");
        serialize<T>::read(sd, val);
      }
    };

    template <typename T>
    struct serialize<const T> {
      static inline void write(SerializedData &sd, const T &val) {
        __QI_DEBUG_SERIALIZATION_W(T, "Const");
        serialize<T>::write(sd, val);
      }
      static inline void read(SerializedData &sd, T &val) {
        __QI_DEBUG_SERIALIZATION_R(T, "Const");
        serialize<T>::read(sd, val);
      }
    };

    template <typename T>
    struct serialize<T, typename boost::enable_if< typename boost::is_base_of<google::protobuf::Message , T>::type >::type > {
      static void write(SerializedData &sd, const T &val) {
        __QI_DEBUG_SERIALIZATION_W(T, "Proto");
        std::string ser;
        val.SerializeToString(&ser);
        sd.writeString(ser);
      }

      static void read(SerializedData &sd, T &val) {
        __QI_DEBUG_SERIALIZATION_R(T, "Proto");
        std::string ser;
        sd.readString(ser);
        val.ParseFromString(ser);
      }
    };

    template<typename U>
    struct serialize< std::vector<U> >  {

      static void write(SerializedData &sd, const std::vector<U> &v) {
        sd.writeInt(v.size());
        if (v.size()) {
          // we should find out if the contents is a fixed size type
          // and directly assign the contents if we can
          typename std::vector<U>::const_iterator it = v.begin();
          typename std::vector<U>::const_iterator end = v.end();
          for (; it != end; ++it) {
            serialize<U>::write(*it);
          }
        }
        __QI_DEBUG_SERIALIZATION_CONTAINER_W(std::vector<U>, v);
      }

      static void read(SerializedData &sd, std::vector<U> &v) {
        int sz;
        sd.readInt(sz);
        v.clear();

        if (sz) {
          v.resize(sz);
          for(unsigned int i=0; i < sz; ++i) {
            serialize<U>::read(v[i]);
          }
        }
        __QI_DEBUG_SERIALIZATION_CONTAINER_R(std::vector<U>, v);
      }
    };



    template<typename K, typename V>
    struct serialize< std::map<K, V> >  {

      static void write(SerializedData &sd, const std::map<K, V> &m) {
        sd.writeInt(m.size());
        if (m.size()) {
          typename std::map<K, V>::const_iterator it = m.begin();
          typename std::map<K, V>::const_iterator end = m.end();
          for (; it != end; ++it) {
            serialize<K>::write(sd, it->first);
            serialize<V>::write(sd, it->second);
          }
        }
        typedef std::map<K, V> debugMap;
        __QI_DEBUG_SERIALIZATION_CONTAINER_W(debugMap, m);
      }

      static void read(SerializedData &sd, std::map<K, V>  &m) {
        int sz;
        sd.readInt(sz);
        m.clear();

        if (sz) {
          for(int i=0; i < sz; ++i) {
            K k;
            V v;
            serialize<K>::read(sd, k);
            serialize<V>::read(sd, v);
            m[k] = v;
          }
        }
        typedef std::map<K, V> debugMap;
        __QI_DEBUG_SERIALIZATION_CONTAINER_R(debugMap, m);
      }
    };

  }
}

#endif // __QI_SERIALIZATION_SERIALIZEDDATA_HXX__
