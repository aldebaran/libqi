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


//    //& and const are not used for function signature, it's only a compiler detail
//    template <typename T>
//    inline void SerializedData::read<T&>(T& i) {
//        read<T>(i);
//    };

//    template <typename T>
//    inline void SerializedData::write<T&>(const T& i) {
//        write<T>(i);
//    };

//    template <typename T>
//    inline void SerializedData::read<const T>(T& i) {
//        read<T>(i);
//    };

//    template <typename T>
//    inline void SerializedData::write<const T>(const T& i) {
//        write<T>(i);
//    };

    // bool
    template<>
    inline void SerializedData::read<bool>(bool& b)
    {
      b = *((bool *) fData.data());
      fData.erase(0, sizeof(bool));
      //std::string sig = qi::signature<bool>::value();
      //std::cout << "readB(" << b << "):" << sig << std::endl;
    }

    template<>
    inline void SerializedData::write<bool>(const bool& b)
    {
      fData.append((char *)&b, sizeof(b));
      //std::string sig = qi::signature<bool>::value();
      //std::cout << "writeB(" << b << "):" << sig << std::endl;
    }

    // int
    template<>
    inline void SerializedData::read<int>(int& i)
    {
      i = *((int *) fData.data());
      fData.erase(0, sizeof(int));
      //std::string sig = qi::signature<int>::value();
      //std::cout << "readI(" << i << "):" << sig << std::endl;
    }

    template<>
    inline void SerializedData::write<int>(const int& i)
    {
      fData.append((char *)&i, sizeof(i));
      //std::string sig = qi::signature<int>::value();
      //std::cout << "writeI(" << i << "):" << sig << std::endl;
    }

    // float
    template<>
    inline void SerializedData::read<float>(float& f)
    {
      f = *((float *) fData.data());
      fData.erase(0, sizeof(float));
      //std::string sig = qi::signature<float>::value();
      //std::cout << "readF(" << f << "):" << sig << std::endl;
    }

    template<>
    inline void SerializedData::write<float>(const float& f)
    {
      std::string sig = qi::signature<int>::value();
      fData.append((char *)&f, sizeof(f));
      //std::cout << "writeF(" << f << "):" << sig << std::endl;
    }

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

    // char
    template<>
    inline void SerializedData::read<char>(char& c)
    {
      c = *((char *) fData.data());
      fData.erase(0, sizeof(char));
      //std::string sig = qi::signature<char>::value();
      //std::cout << "readC(" << c << "):" << sig << std::endl;
    }

    template<>
    inline void SerializedData::write<char>(const char& c)
    {
      fData.append((char *)&c, sizeof(c));
      //std::string sig = qi::signature<char>::value();
      //std::cout << "writeC(" << c << "):" << sig << std::endl;
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

    //template<>
    //inline void SerializedData::write<std::string>(const std::string& s)
    //{
    //  write<int>(s.size());
    //  if (s.size())
    //    fData.append(s.data(), s.size());

    //  //std::string sig = qi::signature<std::string>::value();
    //  //std::cout << "writeS(" << s << "):" << sig << std::endl;
    //}

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
