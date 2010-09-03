/*
**
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2010 Aldebaran Robotics
*/

#ifndef   TYPESIGNATURE_HPP_
# define  TYPESIGNATURE_HPP_

// decorated name length exceeded, name was truncated
#pragma warning( disable : 4503 )

# include <string>
# include <vector>
# include <map>

namespace AL {

  template <typename T>
  struct typeSignature {
    static std::string &value(std::string &valueRef) {
      valueRef = "UNKNOWN";
      return valueRef;
    }

  };

  template <>
  struct typeSignature<bool>  {
    static std::string &value(std::string &valueRef) {
      valueRef += "b";
      return valueRef;
    }
  };

  template <>
  struct typeSignature<char> {
    static std::string &value(std::string &valueRef) {
      valueRef += "c";
      return valueRef;
    }
  };

  template <>
  struct typeSignature<int> {
    static std::string &value(std::string &valueRef) {
      valueRef += "i";
      return valueRef;
    }
  };

  template <>
  struct typeSignature<float> {
    static std::string &value(std::string &valueRef) {
      valueRef += "f";
      return valueRef;
    }
  };

  template <>
  struct typeSignature<double> {
    static std::string &value(std::string &valueRef) {
      valueRef += "d";
      return valueRef;
    }
  };

  template <>
  struct typeSignature<void> {
    static std::string &value(std::string &valueRef) {
      valueRef += "v";
      return valueRef;
    }
  };

  template <>
  struct typeSignature<std::string> {
    static std::string &value(std::string &valueRef) {
      valueRef += "s";
      return valueRef;
    }
  };

  //STL
  template <typename U>
  struct typeSignature< std::vector<U> > {
    static std::string &value(std::string &valueRef) {
      valueRef += "[";
      typeSignature<U>::value(valueRef);
      valueRef += "]";
      return valueRef;
    }
  };

  template <typename T1, typename T2>
  struct typeSignature< std::map<T1, T2> > {
    static std::string &value(std::string &valueRef) {
      valueRef += "{";
      typeSignature<T1>::value(valueRef);
      typeSignature<T2>::value(valueRef);
      valueRef += "}";
      return valueRef;
    }
  };

  //TYPE QUALIFIER
  template <typename T>
  struct typeSignature<T*> {
    static std::string &value(std::string &valueRef) {
      typeSignature<T>::value(valueRef);
      valueRef += "*";
      return valueRef;
    }
  };

  template <typename T>
  struct typeSignature<T&> {
    static std::string &value(std::string &valueRef) {
      typeSignature<T>::value(valueRef);
      valueRef += "&";
      return valueRef;
    }
  };

  template <typename T>
  struct typeSignature<const T> {
    static std::string &value(std::string &valueRef) {
      typeSignature<T>::value(valueRef);
      valueRef += "#";
      return valueRef;
    }
  };

  //less optimised version (but hey nothing is perfect)
  template <typename T>
  struct typeSignatureWithCopy {
    static std::string value() {
      std::string valueRef;
      return typeSignature<T>::value(valueRef);
    }
  };
}

#endif   /* !TYPESIGNATURE_PP_ */
