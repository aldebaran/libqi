/*
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**  - Chris KILNER  <ckilner@aldebaran-robotics.com>
**
** Copyright (C) 2010 Aldebaran Robotics
*/


#ifndef   __QI_SERIALIZATION_SERIALIZEDDATA_HXX__
#define   __QI_SERIALIZATION_SERIALIZEDDATA_HXX__

//#include <vector>

namespace qi {
  namespace serialization {


#define SIMPLE_SERIALIZER(Name, Type)                                 \
  template <>                                                         \
  struct serialize<Type>  {                                           \
    static void write(SerializedData &sd, const Type &val) {          \
      sd.write##Name(val);                                            \
    }                                                                 \
    static void read(SerializedData &sd, Type &val) {                 \
      sd.read##Name(val);                                             \
    }                                                                 \
  };

    SIMPLE_SERIALIZER(Bool, bool);
    SIMPLE_SERIALIZER(Char, char);
    SIMPLE_SERIALIZER(Int, int);
    SIMPLE_SERIALIZER(Float, float);
    SIMPLE_SERIALIZER(String, std::string);


  // vector
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
        //std::string sig = qi::signature<std::vector>::value();
        //std::cout << "writeVect(" << v.size() << "):" << sig << std::endl;
      }

      static void read(SerializedData &sd, std::vector<U> &v) {
        int sz;
        sd.readInt(sz);
        v.clear();

        if (sz) {
          v.resize(sz);
          for(unsigned int i=0; i < sz; ++i) {
            serialize<U>::read(v[i]);
            //U &myref = v[i];
            //read<U>(myref);
          }
        }
        //std::string sig = qi::signature<std::string>::value();
        //std::cout << "readVect(" << sz << "):" << sig << std::endl;
      }
    };

  }
}

#endif // __QI_SERIALIZATION_SERIALIZEDDATA_HXX__
