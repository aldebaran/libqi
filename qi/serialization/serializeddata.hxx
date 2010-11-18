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
      std::string sig = qi::signature<bool>::value();
      b = *((bool *) fData.data());
      fData.erase(0, sizeof(bool));
      //std::cout << "readB(" << b << "):" << sig << std::endl;
    }

    template<>
    inline void SerializedData::write<bool>(const bool& b)
    {
      std::string sig = qi::signature<bool>::value();
      fData.append((char *)&b, sizeof(b));
      //std::cout << "writeB(" << b << "):" << sig << std::endl;
    }

    // int
    template<>
    inline void SerializedData::read<int>(int& i)
    {
      std::string sig = qi::signature<int>::value();
      i = *((int *) fData.data());
      fData.erase(0, sizeof(int));
      //std::cout << "readI(" << i << "):" << sig << std::endl;
    }

    template<>
    inline void SerializedData::write<int>(const int& i)
    {
      std::string sig = qi::signature<int>::value();
      fData.append((char *)&i, sizeof(i));
      //std::cout << "writeI(" << i << "):" << sig << std::endl;
    }

    // float
    template<>
    inline void SerializedData::read<float>(float& f)
    {
      std::string sig = qi::signature<float>::value();
      f = *((float *) fData.data());
      fData.erase(0, sizeof(float));
      //std::cout << "readF(" << f << "):" << sig << std::endl;
    }

    template<>
    inline void SerializedData::write<float>(const float& f)
    {
      std::string sig = qi::signature<int>::value();
      fData.append((char *)&f, sizeof(f));
      //std::cout << "writeF(" << f << "):" << sig << std::endl;
    }

    // string
    template<>
    inline void SerializedData::read<std::string>(std::string& s)
    {
      std::string sig = qi::signature<std::string>::value();
      int sz;
      read<int>(sz);
      s.clear();
      if (sz) {
        s.append(fData.data(), sz);
        fData.erase(0, sz);
      }
      //std::cout << "readS(" << s << "):" << sig << std::endl;
    }

    template<>
    inline void SerializedData::write<std::string>(const std::string& s)
    {
      write<int>(s.size());
      if (s.size())
        fData.append(s.data(), s.size());

      std::string sig = qi::signature<std::string>::value();
      //std::cout << "writeS(" << s << "):" << sig << std::endl;
    }

    template<>
    inline void SerializedData::write<const std::string>(const std::string &s)
    {
      write<int>(s.size());
      if (s.size())
        fData.append(s.data(), s.size());

      std::string sig = qi::signature<std::string>::value();
      //std::cout << "writeSc(" << s << "):" << sig << std::endl;
    }

  }
}

#endif // __QI_SERIALIZATION_SERIALIZEDDATA_HXX__
