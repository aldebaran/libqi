/*
**
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2010 Aldebaran Robotics
*/

#ifndef   TYPESIGNATURE_HPP_
# define  TYPESIGNATURE_HPP_

//runtime signature compile faster
//compile time signature execute faster  (this one need boost::mpl::string that is not available in boost < 40)
//but runtime signature may be good enought in most case
#define USE_RUNTIME_SIGNATURE

#ifdef USE_RUNTIME_SIGNATURE
#include <alcommon-ng/functor/detail/typesignature.hpp>
#include <alcommon-ng/functor/detail/functionsignature.hpp>
#else
#include <alcommon-ng/functor/detail/typesignaturecompiletime.hpp>
#include <alcommon-ng/functor/detail/functionsignaturecompiletime.hpp>
#endif

namespace AL {
  /** Return a signature based on the type passed.
    * Ref and const are not computed (this is a function call detail)
    * those qualifiers are compiler details.
    */

  //this is the entry point of all the signature machinery
  template <typename T>
  struct signature {
    static std::string &value(std::string &valueRef) {
      #ifdef USE_RUNTIME_SIGNATURE
      ::AL::detail::signature<T>::value(valueRef);
      #else
      valueRef += boost::mpl::c_str< typename detail::signature<T>::value >::value;
      #endif
      return valueRef;
    }

    static std::string value() {
      std::string valueRef;
      return value(valueRef);
    }
  };

  struct signatureFromObject {

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
