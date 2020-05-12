#ifndef KA_ZERO_HPP
#define KA_ZERO_HPP
#pragma once

namespace ka {
  namespace details {
    /// Empty base class to disable aggregate construction.
    struct zero_base_t {};
  }

  /// Type with no instance.
  ///
  /// Theoretically, it corresponds to the initial object in the category of
  /// sets and functions, which is the empty set (i.e. the set with no element).
  /// It is therefore the dual of `unit_t`.
  /// It also happens to be (isomorphic to) the sum of no set, hence the
  /// definition should be as an empty sum (but `boost::variant<>` and
  /// `std::variant<>` are incomplete types, see TODO).
  ///
  /// Warning: Abusing C++ type system is easy, and it is possible to form
  ///   references or pointers to this uninstantiable type (e.g. with
  ///   `(zero_t*)nullptr`). A good usage of this type of course excludes this
  ///   kind of practices.
  ///
  /// Warning: Copy constructor is not deactivated as a lot of existing code
  /// (`boost` libraries...) tends to instantiate types too greedily, and
  /// deactivation leads to a lot of artificial compilation problems. In a
  /// correct usage of this type, this lack should not be a problem since there
  /// is no value to copy in the first place.
  ///
  /// meaning(zero_t) = 0
  ///
  /// TODO: Make an alias of `ka::sum_t<>` when available.
  struct zero_t final : private details::zero_base_t {
    zero_t() = delete;

    // Implemented to calm down compiler excessive template instantiations.
    // These methods can never be called though, since no instance can be
    // constructed.
    inline
    auto operator==(zero_t) const -> bool {
      return false;
    }
    inline
    auto operator!=(zero_t) const -> bool {
      return false;
    }
  };
} // namespace ka

#endif // KA_ZERO_HPP
