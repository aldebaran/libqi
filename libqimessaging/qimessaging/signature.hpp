/*
*  Author(s):
*  - Chris  Kilner <ckilner@aldebaran-robotics.com>
*  - Cedric Gestes <gestes@aldebaran-robotics.com>
*
*  Copyright (C) 2010, 2012 Aldebaran Robotics
*/


#pragma once
#ifndef _QIMESSAGING_SIGNATURE_HPP_
#define _QIMESSAGING_SIGNATURE_HPP_

#include <qimessaging/api.hpp>
#include <qi/types.hpp>
#include <string>
#include <vector>
#include <list>
#include <map>
#include <sstream>
#include <boost/shared_ptr.hpp>
#include <boost/type_traits/is_function.hpp>
#include <boost/type_traits/is_member_function_pointer.hpp>
#include <boost/type_traits/remove_const.hpp>
#include <boost/type_traits/remove_reference.hpp>
#include <boost/static_assert.hpp>

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

      //This type should not be used, it's will be removed when we get ride of legacy void *.
      Type_Pointer  = '*',

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


  class QIMESSAGING_API SignatureStream {
  public:
    inline void write(qi::Signature::Type type) {
      ss << (char) type;
    }

    inline std::string str() {
      return ss.str();
    }

  private:
    std::stringstream ss;
  };

  QIMESSAGING_API qi::SignatureStream &operator&(qi::SignatureStream &os, const bool &t);
  QIMESSAGING_API qi::SignatureStream &operator&(qi::SignatureStream &os, const char&);
  QIMESSAGING_API qi::SignatureStream &operator&(qi::SignatureStream &os, const qi::int8_t&);
  QIMESSAGING_API qi::SignatureStream &operator&(qi::SignatureStream &os, const qi::uint8_t&);
  QIMESSAGING_API qi::SignatureStream &operator&(qi::SignatureStream &os, const qi::int16_t&);
  QIMESSAGING_API qi::SignatureStream &operator&(qi::SignatureStream &os, const qi::uint16_t&);
  QIMESSAGING_API qi::SignatureStream &operator&(qi::SignatureStream &os, const qi::int32_t&);
  QIMESSAGING_API qi::SignatureStream &operator&(qi::SignatureStream &os, const qi::uint32_t&);
  QIMESSAGING_API qi::SignatureStream &operator&(qi::SignatureStream &os, const qi::int64_t&);
  QIMESSAGING_API qi::SignatureStream &operator&(qi::SignatureStream &os, const qi::uint64_t&);
  QIMESSAGING_API qi::SignatureStream &operator&(qi::SignatureStream &os, const float&);
  QIMESSAGING_API qi::SignatureStream &operator&(qi::SignatureStream &os, const double&);
  QIMESSAGING_API qi::SignatureStream &operator&(qi::SignatureStream &os, const std::string&);
  QIMESSAGING_API qi::SignatureStream &operator&(qi::SignatureStream &os, const char*);
  QIMESSAGING_API qi::SignatureStream &operator&(qi::SignatureStream &os, char*);

  //Catch all to prevent the compiler to cast everything he can to bool.
  // eg: (pointers, functions pointers, shared_ptr,...)
  template <typename T>
  qi::SignatureStream &operator&(qi::SignatureStream &os, T) {
   #ifdef __QI_SIGNATURE_UNKNOWN_INSTEAD_OF_ASSERT
    os.write(qi::Signature::Type_Unknown);
    return os;
   #else
    static T t;
    //if you have a compilation error here, there are two possibilities:
    // - you are taking the signature of an unsupported type (pointer, function pointer, method pointer)
    // - you forgot to add an operator& for your struct/class.
    BOOST_STATIC_ASSERT(sizeof(t) == 0);
   #endif
  }


  template <typename T>
  qi::SignatureStream &operator&(qi::SignatureStream &os, const std::vector<T> &v) {
    os.write(qi::Signature::Type_List);
    static T t;
    os & t;
    os.write(qi::Signature::Type_List_End);
    return os;
  }

  template <typename T>
  qi::SignatureStream &operator&(qi::SignatureStream &os, const std::list<T> &v) {
    os.write(qi::Signature::Type_List);
    static T t;
    os & t;
    os.write(qi::Signature::Type_List_End);
    return os;
  }

  template <typename K, typename V>
  qi::SignatureStream &operator&(qi::SignatureStream &os, const std::map<K, V> &map) {
    os.write(qi::Signature::Type_Map);
    static K k;
    static V v;
    os & k & v;
    os.write(qi::Signature::Type_Map_End);
    return os;
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
      qi::SignatureStream ss;
      typedef typename boost::remove_const<typename boost::remove_reference<T>::type>::type clean_T;
      //static to only allocate once
      static clean_T reft;
      ss & reft;
      valueRef += ss.str();
      return valueRef;
    }

    static std::string value() {
      std::string valueRef;
      return value(valueRef);
    }
  };

  template <>
  class signatureFromType<void> {
  public:
    static std::string &value(std::string &valueRef) {
      qi::SignatureStream ss;
      ss.write(qi::Signature::Type_Void);
      valueRef += ss.str();
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
    template<typename T>
    static std::string &value(const T &t, std::string &valueRef) {
      qi::SignatureStream ss;
      ss & t;
      valueRef += ss.str();
      return valueRef;
    }

    template<typename T>
    static std::string value(const T &t) {
      std::string valueRef;
      return signatureFromObject::value(t, valueRef);
    }
  };

}

/// Create signature information for given struct.
#define QI_SIGNATURE_STRUCT(Cname, ...) \
  QI_SIGNATURE_STRUCT_DECLARE(Cname) \
  __QI_SIGNATURE_STRUCT_IMPLEMENT_(inline, Cname, __VA_ARGS__)

/// Only declare the signature creation operator.
#define QI_SIGNATURE_STRUCT_DECLARE(Cname) \
  QIMESSAGING_API qi::SignatureStream &operator&(qi::SignatureStream &, const Cname&);

/// Implement the signature creation operator
#define QI_SIGNATURE_STRUCT_IMPLEMENT(Cname, ...) \
  __QI_SIGNATURE_STRUCT_IMPLEMENT_(/**/, Cname, __VA_ARGS__)

#define __QI_ADD_SIG(_, Cname, Field) \
  stream & t.Field;

#define __QI_SIGNATURE_STRUCT_IMPLEMENT_(inl, Cname, ...)                         \
  inl qi::SignatureStream &operator&(qi::SignatureStream &stream, const Cname& t) \
  {                                                                               \
    QI_VAARGS_APPLY(__QI_ADD_SIG, _, __VA_ARGS__);                                \
    return stream;                                                                \
  }


#endif  // _QIMESSAGING_SIGNATURE_HPP_
