#pragma once
/*
**  Copyright (C) 2013 Aldebaran Robotics
**  See COPYING for the license
*/

#ifndef _QI_SIGNATURE_HPP_
#define _QI_SIGNATURE_HPP_

#include <qi/api.hpp>
#include <string>
#include <vector>
#include <list>
#include <map>
#include <sstream>
#include <boost/shared_ptr.hpp>

#ifdef _MSC_VER
#  pragma warning( push )
#  pragma warning( disable: 4251 )
#endif

namespace qi {

  QI_API std::vector<std::string> signatureSplit(const std::string &fullSignature);

  class SignaturePrivate;

  class AnyReference;
  class AnyValue;
  class TypeInterface;
  class Signature;
  QI_API qi::Signature makeTupleSignature(const std::vector<qi::AnyReference>& vgv,
                                              bool resolveDynamic = false,
                                              const std::string &name = std::string(),
                                              const std::vector<std::string>& names = std::vector<std::string>());
  QI_API qi::Signature makeTupleSignature(const std::vector<TypeInterface*>& vgv,
                                              const std::string &name = std::string(),
                                              const std::vector<std::string>& names = std::vector<std::string>());

  QI_API qi::Signature makeTupleSignature(const qi::Signature &element);
  QI_API qi::Signature makeListSignature(const qi::Signature &element);
  QI_API qi::Signature makeVarArgsSignature(const qi::Signature &element);
  QI_API qi::Signature makeKwArgsSignature(const qi::Signature &element);
  QI_API qi::Signature makeMapSignature(const qi::Signature &key, const qi::Signature &value);
  QI_API qi::Signature makeOptionalSignature(const qi::Signature& value);

  /* Represent the serialisation signature of a Type.
  * pseudo-grammar:
  * root: element
  * element: signature annotation.opt
  * sinature:
  *   | primitive  // in bcCwWiIlLfdsmro, see Type
  *   | [element]
  *   | {elementelement}
  *   | (elementsequence)
  * elementsequence: a list of 1 or more elements
  * annotation.opt: empty or <annotation>
  * annotation: may contain arbitrary content except \0
      and must balance all (), {}, [] and <> within
  * for tuple annotation has the following form: "<TupleName,elementName0,...,elementName1>"
  */
  class Signature;
  using SignatureVector = std::vector<Signature>;

  class QI_API Signature {
  protected:
    Signature(const std::string &signature, size_t begin, size_t end);
    friend class SignaturePrivate;

  public:
    Signature();
    Signature(const char *signature);
    Signature(const std::string &signature);

    bool isValid() const;

    bool hasChildren() const;

    const SignatureVector& children() const;

    //TODO use the type than "network type"
    enum Type {
      // Used only for empty containers when Dynamic resolution is used.
      Type_None     = '_',
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

      Type_Object   = 'o',
      Type_VarArgs  = '#',
      Type_KwArgs   = '~',

      Type_Optional = '+',

      Type_Unknown  = 'X',
    };

    Type type() const;
    std::string annotation() const;

    /** Encode the signature in a plain struct,
    * suitable for further serialization.
    * [typeString, childrenList, annotationString ]
    */
    AnyValue toData() const;
    std::string toPrettySignature() const;
    const std::string& toString() const;

    /** Tell if arguments with this signature can be converted to \p b.
     * @return 0 if conversion is impossible, or a score in ]0,1] indicating
     * the amount of type mismatch (the closer signatures are the bigger)
     */
    float isConvertibleTo(const Signature& b) const;

    static Signature fromType(Type t);
  protected:
    // C4251
    boost::shared_ptr<SignaturePrivate> _p;

    friend QI_API bool operator==(const Signature &lhs, const Signature &rhs);
  };

  inline std::ostream& operator<<(std::ostream& os, const Signature& s)
  {
    return os << s.toString();
  }

  inline bool operator!=(const Signature &lhs, const Signature &rhs)
  { return !(lhs == rhs); }
  QI_API bool operator==(const Signature &lhs, const Signature &rhs);

}

extern "C" QI_API char* signature_to_json(const char* sig);

#ifdef _MSC_VER
#  pragma warning( pop )
#endif

#endif  // _QITYPE_SIGNATURE_HPP_
