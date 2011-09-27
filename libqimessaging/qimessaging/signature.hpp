#pragma once
/*
*  Author(s):
*  - Chris  Kilner <ckilner@aldebaran-robotics.com>
*  - Cedric Gestes <gestes@aldebaran-robotics.com>
*
*  Copyright (C) 2010 Aldebaran Robotics
*/


#ifndef _QI_SIGNATURE_HPP_
#define _QI_SIGNATURE_HPP_


#include <qimessaging/api.hpp>
#include <qimessaging/signature/error.hpp>
#include <qimessaging/signature/signature_iterator.hpp>
#include <qimessaging/signature/detail/type_signature.hpp>
#include <qimessaging/signature/detail/function_signature.hpp>
#include <qimessaging/signature/detail/stl_signature.hpp>

namespace qi {


  //this is the entry point of all the signature machinery

  /// Return a signature based on the templated type T, provided in parameter to the struct.
  /// Ref and const are not computed, those qualifiers are compiler details.
  /// \ingroup Signature
  /// \include example_qi_signature_type.cpp
  template <typename T>
  struct QIMESSAGING_API signature {
    static std::string &value(std::string &valueRef) {
      ::qi::detail::signature<T>::value(valueRef);
      return valueRef;
    }

    static std::string value() {
      std::string valueRef;
      return value(valueRef);
    }
  };

  /// Take the signature of an instanciated Object, it could be a references or a pointer.
  /// \ingroup Signature
  /// \include example_qi_signature_instance.cpp
  struct QIMESSAGING_API signatureFromObject {

    //POINTER
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


    //REF
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


  /// Take the signature of a function with it's name. This is a simple wrapper arround qi::signatureFromObject.
  /// <param name="name"> the function name </param>
  /// <param name="f"> a function pointer to take the signature of </param>
  /// \ingroup Signature
  /// \include example_qi_signature_function.cpp
  template<typename F>
  std::string QIMESSAGING_API makeFunctionSignature(const std::string name, F f) {
    std::string value(name);
    value += "::";
    signatureFromObject::value(f, value);
    return value;
  }

  /// return a pretty printed a signature.
  /// \ingroup Signature
  /// \include example_qi_signature_pp.cpp
  QIMESSAGING_API void signatureToString(const char *signature, std::string &result);

  /// return a pretty printed a signature.
  /// \ingroup Signature
  QIMESSAGING_API std::string signatureToString(const char *signature);

  /// return a pretty printed a signature.
  /// \ingroup Signature
  QIMESSAGING_API std::string signatureToString(const std::string& signature);

}

#endif  // _QI_SIGNATURE_HPP_
