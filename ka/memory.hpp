#ifndef KA_MEMORY_HPP
#define KA_MEMORY_HPP
#pragma once
#include <memory>
#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>
#include "typetraits.hpp"

/// @file Contains helper functions related to components of the standard
///   header `<memory>` (helpers to construct `std::shared_ptr`, etc.).

namespace ka {
  /// Constructs a std::shared_ptr<T> with T deduced from the parameter.
  template<typename T>
  std::shared_ptr<Decay<T>> shared_ptr(T&& t) {
    return std::make_shared<Decay<T>>(fwd<T>(t));
  }

  /// Creates a weak_ptr<T> with T deduced from a shared_ptr<T>
  template<typename T>
  std::weak_ptr<T> weak_ptr(std::shared_ptr<T> const& p) {
    return {p};
  }

  template<typename T>
  boost::weak_ptr<T> weak_ptr(boost::shared_ptr<T> const& p) {
    return {p};
  }

  // model Scopelockable std::weak_ptr<T> const:
  template<typename T>
  std::shared_ptr<T> scopelock(std::weak_ptr<T> const& p) {
    return p.lock();
  }

  // model Scopelockable boost::weak_ptr<T> const:
  template<typename T>
  boost::shared_ptr<T> scopelock(boost::weak_ptr<T> const& p) {
    return p.lock();
  }

  namespace detail {
  // Almost models `EmptyMutable` (but `empty()` doesn't affect all
  // regular operations, since `std::unique_ptr` is not `Regular`).
    template<typename T>
    bool empty(std::unique_ptr<T> const& x) KA_NOEXCEPT(true) {
      return !static_cast<bool>(x);
    }

  // model EmptyMutable std::shared_ptr<T>:
    template<typename T>
    bool empty(std::shared_ptr<T> const& x) KA_NOEXCEPT(true) {
      return !static_cast<bool>(x);
    }

  // model EmptyMutable boost::shared_ptr<T>:
    template<typename T>
    bool empty(boost::shared_ptr<T> const& x) KA_NOEXCEPT(true) {
      return !static_cast<bool>(x);
    }
  } // namespace detail
} // namespace ka

#endif // KA_MEMORY_HPP
