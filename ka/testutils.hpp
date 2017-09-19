#ifndef KA_TESTUTILS_HPP
#define KA_TESTUTILS_HPP
#pragma once
#include <iostream>
#include <stdexcept>
#include "macro.hpp"

namespace ka {

  /// Useful to test support for move-only types.
  template<typename T>
  class move_only_t {
    mutable T value;
    bool moved = false;
    void check_not_moved() const {
      if (moved) throw std::runtime_error("operating on moved instance");
    }
  public:
    explicit move_only_t(T const& value = T())
      : value(value) {
    }
    move_only_t(move_only_t const&) = delete;
    move_only_t& operator=(move_only_t const&) = delete;
    move_only_t(move_only_t&& x)
        : value((x.check_not_moved(), std::move(x.value))) {
      x.moved = true;
    }
    move_only_t& operator=(move_only_t&& x) {
      x.check_not_moved();
      value = std::move(x.value);
      moved = false;
      x.moved = true;
      return *this;
    }
    bool operator==(move_only_t const& x) const {
      check_not_moved();
      x.check_not_moved();
      return value == x.value;
    }
  // Mutable<T>:
    T& operator*() const {
      check_not_moved();
      return value;
    }
  };

  /// Allows to know if an instance has been moved.
  struct move_aware_t {
    int i;
    bool moved = false;
    inline explicit move_aware_t(int i) KA_NOEXCEPT(true)
      : i(i) {
    }
    move_aware_t() = default;
    inline move_aware_t(move_aware_t const& x) KA_NOEXCEPT(true)
      : i(x.i) {
    }
    inline move_aware_t& operator=(move_aware_t const& x) KA_NOEXCEPT(true) {
      i = x.i;
      moved = false;
      return *this;
    }
    inline move_aware_t(move_aware_t&& x) KA_NOEXCEPT(true)
        : i(x.i) {
      x.moved = true;
    }
    inline move_aware_t& operator=(move_aware_t&& x) KA_NOEXCEPT(true) {
      i = x.i;
      moved = false;
      x.moved = true;
      return *this;
    }
    inline bool operator==(move_aware_t const& x) const KA_NOEXCEPT(true) {
      return i == x.i; // ignore the `moved` flag.
    }
    inline friend std::ostream& operator<<(std::ostream& o, move_aware_t const& x) {
      return o << x.i;
    }
  };
} // namespace ka

#endif // KA_TESTUTILS_HPP
