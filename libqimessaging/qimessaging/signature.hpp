/*
*  Author(s):
*  - Chris  Kilner <ckilner@aldebaran-robotics.com>
*  - Cedric Gestes <gestes@aldebaran-robotics.com>
*
*  Copyright (C) 2010 Aldebaran Robotics
*/


#pragma once
#ifndef _QIMESSAGING_SIGNATURE_HPP_
#define _QIMESSAGING_SIGNATURE_HPP_

#include <qimessaging/api.hpp>
#include <boost/shared_ptr.hpp>

#include <qimessaging/details/type_signature.hpp>
#include <qimessaging/details/function_signature.hpp>
#include <qimessaging/details/stl_signature.hpp>

namespace qi {

  //this is the entry point of all the signature machinery

  /// Return a signature based on the templated type T, provided in parameter to the struct.
  /// Ref and const are not computed, those qualifiers are compiler details.
  /// \ingroup Signature
  /// \include example_qi_signature_type.cpp
  template <typename T>
  class signatureFromType {
  public:
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
  class signatureFromObject {
  public:
    //POINTER
    template<typename T>
    static std::string &value(const T *t, std::string &valueRef) {
      (void) t;
      return signatureFromType<T*>::value(valueRef);
    }

    template<typename T>
    static std::string value(const T *t) {
      (void) t;
      std::string valueRef;
      return signatureFromType<T*>::value(valueRef);
    }


    //REF
    template<typename T>
    static std::string &value(const T &t, std::string &valueRef) {
      (void) t;
      return signatureFromType<T>::value(valueRef);
    }

    template<typename T>
    static std::string value(const T &t) {
      (void) t;
      std::string valueRef;
      return signatureFromType<T>::value(valueRef);
    }
  };

  QIMESSAGING_API std::vector<std::string> signatureSplit(const std::string &fullSignature);

  class SignaturePrivate;

  class QIMESSAGING_API Signature {
  public:

  public:
    Signature(const char *signature = 0);
    Signature(const std::string &signature);

    bool isValid() const;

    class iterator;
    iterator begin() const;
    iterator end() const;

    //TODO use the type than "network type"
    enum Type {
      None     = 0,
      Bool     = 'b',
      Char     = 'c',
      UChar    = 'C',
      Void     = 'v',
      Int      = 'i',
      UInt     = 'I',
      Float    = 'f',
      Double   = 'd',
      String   = 's',
      List     = '[',
      Map      = '{',
      Tuple    = '(',
      Object   = '@'
    };

    class iterator {
    public:
      iterator() : _current(0), _end(0) {}
      iterator          &operator++();
      iterator          &operator++(int);
      inline bool        operator!=(const iterator &rhs) const { return _current != rhs._current; }
      inline bool        operator==(const iterator &rhs) const { return _current == rhs._current; };
      inline std::string operator*() const                     { return signature(); }
      inline std::string operator->() const                    { return signature(); }

      // accesors
      Type        type()const;
      std::string signature()const;
      bool        isValid()const;
      int         pointer()const;

      bool        hasChildren()const;
      Signature   children()const;

    protected:
      iterator(const char *begin, const char *end) : _current(begin), _end(end) {}
      const char *_current;
      const char *_end;
      friend class qi::Signature;
    };



    std::string toSTLSignature(bool constify = false) const;
    std::string toQtSignature(bool constify = false) const;
    std::string toString() const;

  protected:
    boost::shared_ptr<SignaturePrivate> _p;
  };


}

#endif  // _QIMESSAGING_SIGNATURE_HPP_
