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

#include <string>
#include <vector>
#include <qimessaging/api.hpp>
#include <boost/shared_ptr.hpp>


namespace qi {

  QIMESSAGING_API std::vector<std::string> signatureSplit(const std::string &fullSignature);

  class SignaturePrivate;

  class QIMESSAGING_API Signature {
  public:

  public:
    Signature(const char *signature = 0);
    Signature(const std::string &signature);

    bool isValid() const;

    // Number of elements in the signature.
    unsigned int size() const;

    class iterator;
    iterator begin() const;
    iterator end() const;

    //TODO use the type than "network type"
    enum Type {
      Type_None     = 0,
      Type_Bool     = 'b',

      Type_Int8     = 'c',
      Type_UInt8    = 'C',

      Type_Void     = 'v',

      Type_Int16    = 'w',
      Type_UInt16   = 'W',

      Type_Int32    = 'i',
      Type_UInt32   = 'I',

      Type_Int64    = 'l',
      Type_UInt64   = 'L',

      Type_Float    = 'f',
      Type_Double   = 'd',

      Type_String   = 's',
      Type_List     = '[',
      Type_List_End = ']',

      Type_Map      = '{',
      Type_Map_End  = '}',

      Type_Tuple    = '(',
      Type_Tuple_End= ')',

      Type_Dynamic  = 'm',

      Type_Raw      = 'r',

      Type_Unknown  = 'X'
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

namespace qi {

  //defined here before signatureFromType
  namespace detail {
    template <typename T, class Enable = void>
    struct signature {
      static std::string &value(std::string &val) {
        //should match Type_Unknown (which is not defined at that moment)
        val += (char)qi::Signature::Type_Unknown;
        return val;
      }
    };
  }

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

}

#include <qimessaging/details/type_signature.hpp>
#include <qimessaging/details/function_signature.hpp>
#include <qimessaging/details/stl_signature.hpp>



/// Create signature information for given struct.
#define QI_SIGNATURE_STRUCT(Cname, ...) \
  QI_SIGNATURE_STRUCT_DECLARE(Cname) \
  __QI_SIGNATURE_STRUCT_IMPLEMENT_(inline, Cname, __VA_ARGS__)

/// Only declare the signature creation operator.
#define QI_SIGNATURE_STRUCT_DECLARE(Cname) \
namespace qi { namespace detail {  \
  template<> struct signature<Cname> { static std::string& value( std::string& val); };}}

/// Implement the signature creation operator
#define QI_SIGNATURE_STRUCT_IMPLEMENT(Cname, ...) \
  __QI_SIGNATURE_STRUCT_IMPLEMENT_(/**/, Cname, __VA_ARGS__)

#define __QI_ADD_SIG(_, Cname, Field) \
  val += ::qi::signatureFromObject::value(dummy.Field);

#define __QI_SIGNATURE_STRUCT_IMPLEMENT_(inl, Cname, ...) \
namespace qi {                                                       \
   namespace detail {                                                 \
     inl std::string& signature<Cname>::value( std::string& val)          \
       {                                                              \
         Cname dummy;                                                 \
         QI_VAARGS_APPLY(__QI_ADD_SIG, _, __VA_ARGS__);                \
         return val;                                                  \
       }                                                              \
   }                                                                  \
 }


#endif  // _QIMESSAGING_SIGNATURE_HPP_
