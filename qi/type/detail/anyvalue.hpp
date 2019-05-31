#pragma once
/*
**  Copyright (C) 2013 Aldebaran Robotics
**  See COPYING for the license
*/

#ifndef _QITYPE_DETAIL_ANYVALUE_HPP_
#define _QITYPE_DETAIL_ANYVALUE_HPP_

#include <ka/macro.hpp>

namespace qi {

  /** Represent any value supported by the typesystem.
   *  when constructed or set the value is copied.
   *  as a pointer to the real value.
   *  to convert the value if needed and copy to the required type.
   *
   *  \includename{qi/anyvalue.hpp}
   */
  class QI_API AnyValue: public detail::AnyReferenceBase
  {
  public:

    /**
     * A default-constructed AnyValue is invalid, and equivalent to an unset optional value.
     * It cannot be converted to any other type, and will yield exceptions instead.
     */
    AnyValue();
    AnyValue(const AnyValue& b);
    AnyValue(AnyValue&& b) KA_NOEXCEPT(true);
    explicit AnyValue(const AnyReference& b, bool copy, bool free);
    explicit AnyValue(const AutoAnyReference& b);
    explicit AnyValue(qi::TypeInterface *type);
    /// Create and return a AnyValue of type T
    template<typename T> static AnyValue make();

    /// @return the contained value, and reset the AnyValue.
    /// @warning you should destroy the returned value or no, depending on how the AnyValue was initialized.
    AnyReference release() {
      AnyReference ref = AnyReference(_type, _value);
      _allocated = false;
      _value = 0;
      _type = 0;
      return ref;
    }

    /// @{
    /** The following functions construct a AnyValue from containers of
     * AnyReference.
     */
    static AnyValue makeTuple(const AnyReferenceVector& values);
    static AnyValue makeTupleFromValue(const AutoAnyReference& v0,
                                       const AutoAnyReference& v1,
                                       const AutoAnyReference& v2,
                                       const AutoAnyReference& v3,
                                       const AutoAnyReference& v4,
                                       const AutoAnyReference& v5,
                                       const AutoAnyReference& v6,
                                       const AutoAnyReference& v7,
                                       const AutoAnyReference& v8,
                                       const AutoAnyReference& v9);

    template<typename T>
    static AnyValue makeList(const AnyReferenceVector& values);
    static AnyValue makeGenericList(const AnyReferenceVector& values);
    template<typename K, typename V>
    static AnyValue makeMap(const std::map<AnyReference, AnyReference>& values);
    static AnyValue makeGenericMap(const std::map<AnyReference, AnyReference>& values);
    /// @}

    /// Construct a void AnyValue: defined, but with no data.
    static AnyValue makeVoid();

    ~AnyValue();
    AnyValue& operator=(const AnyReference& b);
    AnyValue& operator=(const AnyValue& b);
    AnyValue& operator=(AnyValue&& b);

    void reset();
    void reset(qi::TypeInterface *type);

    template <typename T>
    void set(const T& t) { detail::AnyReferenceBase::set<T>(t); }

    void reset(const AnyReference& src);
    void reset(const AnyReference& src, bool copy, bool free);

    void swap(AnyValue& b);

    AnyReference asReference() const {
      //AnyRef == AnyRefBase
      return *reinterpret_cast<const AnyReference*>(
          static_cast<const detail::AnyReferenceBase*>(this));
    }

    template<typename T>
    static AnyValue from(const T& r) {
      //explicit AutoAnyReference to avoid ambiguous call for object implementing cast to AnyValue
      return AnyValue(AutoAnyReference(r));
    }

  private:
    // It is "unsafe" because it invalidates internal pointers without
    // nullifying them. Caller is expected to set them afterwards to valid
    // values.
    void resetUnsafe();

    //hide AnyReference::destroy
    //simply assign an empty AnyValue.
    void destroy() { return detail::AnyReferenceBase::destroy(); }

    //we dont accept GVP here.  (block set<T> with T=GVP)
    void set(const AnyReference& t);
    bool _allocated;
  };

  /// Less than operator. Will compare the values within the AnyValue.
  QI_API bool operator<(const AnyValue& a, const AnyValue& b);

  /// Value equality operator. Will compare the values within.
  QI_API bool operator==(const AnyValue& a, const AnyValue& b);
  QI_API bool operator!=(const AnyValue& a, const AnyValue& b);

  using AnyValueVector = std::vector<AnyValue>;

  inline AnyReferenceVector asAnyReferenceVector(const AnyValueVector& vect);

}

#include <qi/type/detail/anyvalue.hxx>

#endif  // _QITYPE_DETAIL_ANYVALUE_HPP_
