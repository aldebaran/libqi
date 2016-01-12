#pragma once
/*
**  Copyright (C) 2013 Aldebaran Robotics
**  See COPYING for the license
*/

#ifndef _QI_TYPE_DETAIL_ANYITERATOR_HPP_
#define _QI_TYPE_DETAIL_ANYITERATOR_HPP_

#include <qi/type/detail/anyvalue.hpp>

namespace qi {

  /** AnyValue with Iterator kind, behaving as a STL-compatible iterator
   */
  class QI_API AnyIterator: public AnyValue
  {
  public:
    using value_type = AnyReference;
    using pointer = AnyReference*;
    using reference = AnyReference&;
    using difference_type = ptrdiff_t;
    using iterator_category = std::forward_iterator_tag;

    AnyIterator();
    AnyIterator(const AnyReference& p);
    AnyIterator(const AnyValue& v);

    template<typename T>
    explicit AnyIterator(const T& ref);

    /// Iterator pre-increment
    AnyIterator& operator++();
    /// Iterator post-increment
    AnyIterator operator++(int);
    /// Dereference
    AnyReference operator*();
  };

  QI_API bool operator==(const AnyIterator& a, const AnyIterator& b);
  QI_API bool operator!=(const AnyIterator& a, const AnyIterator& b);
}

#include <qi/type/detail/anyiterator.hxx>

#endif  // _QI_TYPE_DETAIL_ANYITERATOR_HPP_
