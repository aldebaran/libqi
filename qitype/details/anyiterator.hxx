#pragma once
/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/

#ifndef _QITYPE_DETAILS_ANYITERATOR_HXX_
#define _QITYPE_DETAILS_ANYITERATOR_HXX_

#include <qitype/typeinterface.hpp>

namespace qi {

  inline AnyReference AnyIterator::operator*()
  {
    if (kind() == TypeKind_Iterator)
      return static_cast<IteratorTypeInterface*>(type)->dereference(value);
    else
      throw std::runtime_error("Expected iterator");
  }

  template<typename T>
  AnyIterator::AnyIterator(const T& ref)
    : AnyValue(AnyReference(ref))
  {}

  inline AnyIterator::AnyIterator()
  {}

  inline AnyIterator::AnyIterator(const AnyReference& p)
    : AnyValue(p)
  {}

  inline AnyIterator::AnyIterator(const AnyValue& v)
    : AnyValue(v)
  {}

  inline AnyIterator AnyIterator::operator++()
  {
    if (kind() != TypeKind_Iterator)
      throw std::runtime_error("Expected an iterator");
    static_cast<IteratorTypeInterface*>(type)->next(&value);
    return *this;
  }

  inline bool operator!=(const AnyIterator& a, const AnyIterator& b)
  {
    return !(a==b);
  }

}


#endif  // _QITYPE_DETAILS_ANYITERATOR_HXX_
