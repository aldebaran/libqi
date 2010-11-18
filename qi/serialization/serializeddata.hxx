/*
** serializeddata.hxx
** Login : <ctaf42@cgestes-de2>
** Started on  Wed Nov 17 19:46:49 2010 Cedric GESTES
** $Id$
**
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2010 Cedric GESTES
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 3 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program; if not, write to the Free Software
** Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
*/

#ifndef   __QI_SERIALIZATION_SERIALIZEDDATA_HXX__
#define   __QI_SERIALIZATION_SERIALIZEDDATA_HXX__


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
