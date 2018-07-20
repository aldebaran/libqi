/*
**  Copyright (C) 2018 SoftBank Robotics Europe
**  See COPYING for the license
*/

#ifndef QI_NUMERIC_HPP
#define QI_NUMERIC_HPP

#pragma once

#include <qi/assert.hpp>
#include <boost/numeric/conversion/converter.hpp>
#include <cstdint>
#include <limits>
#include <stdexcept>

namespace qi
{
  using NumericPositiveOverflow = boost::numeric::positive_overflow;
  using NumericNegativeOverflow = boost::numeric::negative_overflow;

  /// Converts a value of an arithmetic source type into the equivalent value of
  /// the arithmetic destination type. If the result would overflow or
  /// underflow, throws respectively a qi::NumericPositiveOverflow or a
  /// qi::NumericNegativeOverflow exception.
  ///
  /// Note: It is undefined behavior to convert a floating number that is NaN.
  ///
  /// Example: Converting from std::uint8_t to std::int8_t
  /// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  /// std::uint8_t a = 78u;
  /// auto b = qi::numericConvert<std::int8_t>(a); // b == 78
  /// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  ///
  /// Example: Overflowing
  /// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  /// std::uint8_t a = 145u;
  /// auto b = qi::numericConvert<std::int8_t>(a); // throws a NumericPositiveOverflow exception.
  /// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  ///
  /// Example: Underflowing
  /// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  /// std::int8_t a = -37;
  /// auto b = qi::numericConvert<std::uint8_t>(a); // throws a NumericNegativeOverflow exception.
  /// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  ///
  /// Arithmetic Dst
  /// Arithmetic Src
  template<typename Dst, typename Src>
  Dst numericConvert(Src v)
  {
    return boost::numeric::converter<Dst, Src>::convert(v);
  }

  /// Converts a value of an arithmetic source type into the equivalent value of
  /// the arithmetic destination type. If the result would overflow or
  /// underflow, returns respectively the max or the min of the destination
  /// type.
  ///
  /// Note: It is undefined behavior to convert a floating number that is NaN.
  ///
  /// Example: Converting from std::uint8_t to std::int8_t
  /// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  /// std::uint8_t a = 78u;
  /// auto b = qi::numericConvertBound<std::int8_t>(a); // b == 78
  /// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  ///
  /// Example: Overflowing
  /// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  /// std::uint8_t a = 145u;
  /// auto b = qi::numericConvertBound<std::int8_t>(a); // b == INT8_MAX
  /// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  ///
  /// Example: Underflowing
  /// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  /// std::int8_t a = -37;
  /// auto b = qi::numericConvertBound<std::uint8_t>(a); // b == UINT8_MIN
  /// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  ///
  /// Arithmetic Dst
  /// Arithmetic Src
  template<typename Dst, typename Src>
  Dst numericConvertBound(Src v)
  {
    using Converter = boost::numeric::converter<Dst, Src>;

    switch (Converter::out_of_range(v))
    {
      case boost::numeric::cNegOverflow: return std::numeric_limits<Dst>::min();
      case boost::numeric::cPosOverflow: return std::numeric_limits<Dst>::max();
      case boost::numeric::cInRange:     return Converter::convert(v);
    }
    QI_ASSERT_UNREACHABLE();
    return Dst{};
  }

  /// Returns true if a value of an arithmetic source type is in the range of
  /// an arithmetic destination type.
  ///
  /// Arithmetic Dst
  /// Arithmetic Src
  template<typename Dst, typename Src>
  bool numericIsInRange(Src v)
  {
    using Converter = boost::numeric::converter<Dst, Src>;
    return Converter::out_of_range(v) == boost::numeric::cInRange;
  }
}

#endif // QI_NUMERIC_HPP
