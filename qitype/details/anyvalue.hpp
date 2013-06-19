#pragma once
/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/

#ifndef _QITYPE_DETAILS_ANYVALUE_HPP_
#define _QITYPE_DETAILS_ANYVALUE_HPP_

#include <qitype/api.hpp>
#include <qitype/fwd.hpp>
#include <qitype/details/anyreference.hpp>

namespace qi {

  /** AnyReference with copy semantics
   */
  class QITYPE_API AnyValue: public AnyReference
  {
  public:

    AnyValue();
    /// Share ownership of value with b.
    AnyValue(const AnyValue& b);
    explicit AnyValue(const AnyReference& b, bool copy, bool free);
    explicit AnyValue(const AutoAnyReference& b);
    explicit AnyValue(qi::TypeInterface *type);
    /// Create and return a AnyValue of type T
    template<typename T> static AnyValue make();

    /// @{
    /** The following functions construct a AnyValue from containers of
     * AnyReference.
     */
    static AnyValue makeTuple(const std::vector<AnyReference>& values);
    template<typename T>
    static AnyValue makeList(const std::vector<AnyReference>& values);
    static AnyValue makeGenericList(const std::vector<AnyReference>& values);
    template<typename K, typename V>
    static AnyValue makeMap(const std::map<AnyReference, AnyReference>& values);
    static AnyValue makeGenericMap(const std::map<AnyReference, AnyReference>& values);
    /// @}

    ~AnyValue();
    void operator = (const AnyReference& b);
    void operator = (const AnyValue& b);
    void reset();
    void reset(qi::TypeInterface *type);
    template <typename T>
    void set(const T& t) { AnyReference::set<T>(t); }
    void reset(const AnyReference& src);
    void reset(const AnyReference& src, bool copy, bool free);
    void swap(AnyValue& b);

    template<typename T>
    static AnyValue from(const T& r) { return AnyValue(r);}

  private:
    //we dont accept GVP here.  (block set<T> with T=GVP)
    void set(const AnyReference& t);
    bool _allocated;
  };

  /// Less than operator. Will compare the values within the AnyValue.
  QITYPE_API bool operator<(const AnyValue& a, const AnyValue& b);

  /// Value equality operator. Will compare the values within.
  QITYPE_API bool operator==(const AnyValue& a, const AnyValue& b);
  QITYPE_API bool operator!=(const AnyValue& a, const AnyValue& b);

}

#include <qitype/details/anyvalue.hxx>

#endif  // _QITYPE_DETAILS_ANYVALUE_HPP_
