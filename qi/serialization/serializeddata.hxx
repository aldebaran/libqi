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

#define SIMPLE_32_BIT_SERIALIZER(TYPE)                       \
    template<>                                               \
    inline void SerializedData::read<TYPE>(TYPE& b)          \
    {                                                        \
      b = *((TYPE *) fData.data());                          \
      fData.erase(0, sizeof(TYPE));                          \
    }                                                        \
                                                             \
    template<>                                               \
    inline void SerializedData::write<TYPE>(const TYPE& b)   \
    {                                                        \
      fData.append((char *)&b, sizeof(b));                   \
    }                                                        \

SIMPLE_32_BIT_SERIALIZER(bool)
SIMPLE_32_BIT_SERIALIZER(char)
SIMPLE_32_BIT_SERIALIZER(int)
SIMPLE_32_BIT_SERIALIZER(float)

    // double
    template<>
    inline void SerializedData::read<double>(double& d)
    {
      memcpy(&d, fData.data(), sizeof(double));
      fData.erase(0, sizeof(double));
      //std::string sig = qi::signature<double>::value();
      //std::cout << "readD(" << d << "):" << sig << std::endl;
    }

    template<>
    inline void SerializedData::write<double>(const double& d)
    {
      fData.append((char *)&d, sizeof(d));
      //std::string sig = qi::signature<double>::value();
      //std::cout << "writeD(" << d << "):" << sig << std::endl;
    }

    // string
    template<>
    inline void SerializedData::read<std::string>(std::string& s)
    {
      int sz;
      read<int>(sz);
      s.clear();
      if (sz) {
        s.append(fData.data(), sz);
        fData.erase(0, sz);
      }
      //std::string sig = qi::signature<std::string>::value();
      //std::cout << "readS(" << s << "):" << sig << std::endl;
    }

    template<>
    inline void SerializedData::write<std::string>(const std::string &s)
    {
      write<int>(s.size());
      if (s.size())
        fData.append(s.data(), s.size());

      //std::string sig = qi::signature<std::string>::value();
      //std::cout << "writeSc(" << s << "):" << sig << std::endl;
    }

    //// vector
    //template<>
    //inline void SerializedData::read<std::vector>(std::vector& v)
    //{
    //  int sz;
    //  read<int>(sz);
    //  v.clear();
    //  fData.erase(0,1);
    //  if (sz) {
    //    v.resize(sz);
    //    for(unsigned int i=0; i<sz; ++i) {
    //      //v[i] = read(... something ...)
    //      //fData.erase(0, sz);
    //    }
    //  }
    //  //std::string sig = qi::signature<std::string>::value();
    //  //std::cout << "readS(" << s << "):" << sig << std::endl;
    //}

    //template<>
    //inline void SerializedData::write<const std::vector>(const std::vector &v)
    //{
    //  write<int>(v.size());
    //  if (v.size())
    //    // we should find out if the contents is a fixed size type
    //    // and directly assign the contents if we can
    //    std::vector::const_iterator it = v.begin();
    //    std::vector::const_iterator end = v.end();
    //    for (; it != end; ++it) {
    //      write(*it);
    //    }

    //  //std::string sig = qi::signature<std::vector>::value();
    //  //std::cout << "writeV(" << v << "):" << sig << std::endl;
    //}

  }
}

#endif // __QI_SERIALIZATION_SERIALIZEDDATA_HXX__
