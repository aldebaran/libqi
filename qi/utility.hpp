#pragma once
#ifndef _QI_UTILITY_HPP_
#define _QI_UTILITY_HPP_
#include <type_traits>
#include <qi/macro.hpp>

namespace qi
{
  /// Less noisy equivalent to `std::forward`.
  ///
  /// The only purpose is to reduce the noise in libqi's generic code:
  /// occurrences of `std::forward<T>(t)` can be replaced by `fwd<T>(t)`.
  ///
  /// Note: This code is from the libstdc++ shipped with g++-7.
  template<typename T>
  BOOST_CONSTEXPR T&& fwd(typename std::remove_reference<T>::type& t) QI_NOEXCEPT(true)
  {
    return static_cast<T&&>(t);
  }

  template<typename T>
  BOOST_CONSTEXPR T&& fwd(typename std::remove_reference<T>::type&& t) QI_NOEXCEPT(true)
  {
    static_assert(!std::is_lvalue_reference<T>::value,
      "template argument substituting T is an lvalue reference type");
    return static_cast<T&&>(t);
  }
} // namespace qi

#endif // _QI_UTILITY_HPP_
