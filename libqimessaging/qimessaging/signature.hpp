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

//#include <boost/preprocessor/seq/for_each.hpp>
//#include <boost/preprocessor/tuple/elem.hpp>

//#define _QI_SIGNATURE_REFLECT_ELEM(r, data, elem)  \
//  qi::detail::signature<BOOST_PP_TUPLE_ELEM(2, 0, elem)>::value(val);

//#define QI_SIGNATURE_REFLECT(TYPE, MEMBERS)                                           \
//  namespace qi { namespace detail {                                                   \
//    template <>                                                                       \
//      struct signature<TYPE>  {                                                       \
//      static std::string &value(std::string &val) {                                   \
//        val += "(";                                                                   \
//        BOOST_PP_SEQ_FOR_EACH(_QI_SIGNATURE_REFLECT_ELEM, none, MEMBERS)              \
//        val += ")";                                                                   \
//        return val;                                                                   \
//      }                                                                               \
//    };                                                                                \
//  }}                                                                                  \

namespace qi {

  //this is the entry point of all the signature machinery

  /// Return a signature based on the templated type T, provided in parameter to the struct.
  /// Ref and const are not computed, those qualifiers are compiler details.
  /// \ingroup Signature
  /// \include example_qi_signature_type.cpp
  template <typename T>
  struct QIMESSAGING_API signatureFromType {
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


  class SignaturePrivate;

  class QIMESSAGING_API Signature {
  public:

  public:
    Signature(const char *fullSignature = 0);
    Signature(const std::string &fullSignature);

    bool isValid() const;

    class iterator;
    ::qi::Signature::iterator begin() const;
    ::qi::Signature::iterator end() const;

    //TODO use the type than "network type"
    enum Type {
      None     = 0,
      Bool     = 'b',
      Char     = 'c',
      Void     = 'v',
      Int      = 'i',
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
      iterator(const char *begin = 0, const char *end = 0) : _current(begin), _end(end) {}
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
      const char *_current;
      const char *_end;
    };

    std::string toSTLSignature(bool constify = false);
    std::string toQtSignature(bool constify = false);

  protected:
    boost::shared_ptr<SignaturePrivate> _p;
  };


}

#endif  // _QIMESSAGING_SIGNATURE_HPP_
