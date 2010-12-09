#pragma once
/*
*  Author(s):
*  - Chris  Kilner <ckilner@aldebaran-robotics.com>
*  - Cedric Gestes <gestes@aldebaran-robotics.com>
*
*  Copyright (C) 2010 Aldebaran Robotics
*/


#ifndef _QI_SIGNATURE_SIGNATURE_HPP_
#define _QI_SIGNATURE_SIGNATURE_HPP_

// runtime signature compiles faster
// compile time signature execute faster  (this one needs boost::mpl::string that is not available in boost < 40)
// but runtime signature may be good enough in most cases
#define _QI_USE_RUNTIME_SIGNATURE

#ifdef _QI_USE_RUNTIME_SIGNATURE
#include <qi/signature/detail/type_signature.hpp>
#include <qi/signature/detail/function_signature.hpp>
#include <qi/signature/detail/protobuf_signature.hpp>
#else
#include <qi/signature/detail/typesignaturecompiletime.hpp>
#include <qi/signature/detail/functionsignaturecompiletime.hpp>
#endif

namespace qi {
    /** Return a signature based on the type passed.
    * Ref and const are not computed
    * those qualifiers are compiler details.
    */

    //this is the entry point of all the signature machinery
    template <typename T>
    struct signature {
      static std::string &value(std::string &valueRef) {
#ifdef _QI_USE_RUNTIME_SIGNATURE
        ::qi::detail::signature<T>::value(valueRef);
#else
        valueRef += ::boost::mpl::c_str< typename ::qi::detail::signature<T>::value >::value;
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

    }
  /// Take the signature of a function with it's name. This is a simple wrapper arround qi::signatureFromObject.
  /// <param name="name"> the function name </param>
  /// <param name="f"> a function pointer to take the signature of </param>
  /// \ingroup Signature
  /// \include example_qi_signature_function.cpp
  template<typename F>
  std::string makeFunctionSignature(const std::string name, F f) {
    std::string value(name);
    value += "::";
    signatureFromObject::value(f, value);
    return value;
  }
}

#endif  // _QI_SIGNATURE_SIGNATURE_HPP_
