/*
**
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2010 Aldebaran Robotics
*/

#ifndef   TYPESIGNATURE_HPP_
# define  TYPESIGNATURE_HPP_

#include <alcommon-ng/functor/detail/typesignature.hpp>
#include <alcommon-ng/functor/detail/functionsignature.hpp>

namespace AL {
  /** Return a signature based on the type passed.
    * Ref and const are not computed (this is a function call detail)
    * those qualifiers are compiler details.
    */

  template <typename T>
  struct signature {
    static std::string &value(std::string &valueRef) {
      valueRef += boost::mpl::c_str< typename detail::signature<T>::value >::value;
      return valueRef;
    }

    static std::string value() {
      std::string valueRef;
      return value(valueRef);
    }
  };

  struct signatureFromObject {
//    template<typename T>
//    static std::string &value(T t, std::string &valueRef) {
//      (void) t;
//      return signature<T>::value(valueRef);
//    }

//    template<typename T>
//    static std::string value(T t) {
//      (void) t;
//      std::string valueRef;
//      return signature<T>::value(valueRef);
//    }

    template<typename T>
    static std::string &value(const T *t, std::string &valueRef) {
      (void) t;
      return signature<T*>::value(valueRef);
    }

    template<typename T>
    static std::string value(const T *t) {
      (void) t;
      std::string valueRef;
      return signature<T*>::value(valueRef);
    }

    template<typename T>
    static std::string &value(const T &t, std::string &valueRef) {
      (void) t;
      return signature<T>::value(valueRef);
    }

    template<typename T>
    static std::string value(const T &t) {
      (void) t;
      std::string valueRef;
      return signature<T>::value(valueRef);
    }

  };

  template<typename F>
  std::string makeSignature(const std::string name, F f) {
    std::string value(name);
    value += "::";
    signatureFromObject::value(f, value);
    return value;
  }
}

#endif   /* !TYPESIGNATURE_PP_ */
