#ifndef QI_TAG_HPP
#define QI_TAG_HPP
#pragma once
#include <ka/macroregular.hpp>

namespace qi
{
  /// Tag type to represent infinity.
  ///
  /// Example: Representing a timeout that can be infinite
  /// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  /// struct VisitTimeout : boost::static_visitor<int>
  /// {
  ///   int operator()(MilliSeconds x) const {
  ///     return x.count();
  ///   }
  ///   int operator()(Infinity) const {
  ///     return int(FutureTimeout_Infinite);
  ///   }
  /// };
  ///
  /// // In some class:
  /// ValueType valueCopy(Either<MilliSeconds, Infinity> timeout = Infinity{}) const
  /// {
  ///   // `value()` is aware of `FutureTimeout_Infinite`.
  ///   return value(timeout.apply_visitor(VisitTimeout{}));
  /// }
  /// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  struct Infinity
  {
    KA_GENERATE_FRIEND_REGULAR_OPS_0(Infinity)
  };
} // namespace qi

#endif // QI_TAG_HPP
