#ifndef KA_TESTUTILS_HPP
#define KA_TESTUTILS_HPP
#pragma once
#include <cstddef>
#include <functional>
#include <iostream>
#include <stdexcept>
#include <tuple>
#include <utility>
#include "empty.hpp"
#include "functional.hpp"
#include "macro.hpp"
#include "macroregular.hpp"

namespace ka {
  namespace test {
    // These types can be used to test correct typing of generic code.
    KA_DERIVE_REGULAR_TEST_TYPE(A);
    KA_DERIVE_REGULAR_TEST_TYPE(B);
    KA_DERIVE_REGULAR_TEST_TYPE(C);
    KA_DERIVE_REGULAR_TEST_TYPE(D);
    KA_DERIVE_REGULAR_TEST_TYPE(E);
    KA_DERIVE_REGULAR_TEST_TYPE(F);
    KA_DERIVE_REGULAR_TEST_TYPE(G);
    KA_DERIVE_REGULAR_TEST_TYPE(H);
    KA_DERIVE_REGULAR_TEST_TYPE(I);
    KA_DERIVE_REGULAR_TEST_TYPE(J);
  } // namespace test

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

  /// Operations for the `Regular` concept.
  ///
  /// Note: This goes a bit beyond the definition in `concept.hpp`, by including
  /// move operations.
  ///
  /// This is not an enum class to facilitate usage as an index.
  enum regular_op_t {
    regular_op_construct = 0u,
    regular_op_default_construct,
    regular_op_copy,
    regular_op_move,
    regular_op_assign,
    regular_op_move_assign,
    regular_op_destroy,
    regular_op_equality,
    regular_op_ordering,

    regular_op_end
  };

  /// Counters for regular operations.
  ///
  /// Indexing is done with `regular_op_t` values.
  using regular_counters_t = std::array<int, regular_op_end>;

  /// Sets all regular operations counters to 0.
  inline void reset(regular_counters_t& c) KA_NOEXCEPT(true) {
    c = regular_counters_t{};
  }

  /// Regular type that calls a callback for each performed regular operation
  /// (copy, assignment, etc.).
  ///
  /// This allows for instance to check how many times each operation has been
  /// done.
  ///
  /// For default construction to be notified, the procedure type default
  /// constructed value must be non-empty (i.e. valid).
  ///
  /// Example: Counting regular operations with global counters.
  /// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  /// auto& counters = regular_op_global_counters();
  /// reset(counters);
  /// using T = instrumented_regular_t<int, on_regular_op_global_t>;
  /// T a, b, c;
  /// T d = my_algo(a, b, c);
  /// ...
  /// ASSERT_EQ(n, counters[regular_op_copy]); // `n` and `m` have been defined.
  /// ASSERT_EQ(m, counters[regular_op_equality]);
  /// ...
  /// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  ///
  /// Example: Counting regular operations with local counters.
  /// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  /// regular_counters_t counters{};
  /// on_regular_op_local_t on_op{&counters};
  /// using T = instrumented_regular_t<int, on_regular_op_local_t>;
  /// T a{5, on_op}, b{17, on_op}, c{26435, on_op};
  /// T d = my_algo(a, b, c);
  /// ...
  /// ASSERT_EQ(n, counters[regular_op_copy]); // `n` and `m` have been defined.
  /// ASSERT_EQ(m, counters[regular_op_equality]);
  /// ...
  /// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  ///
  /// Regular T, EmptyProcedure<_ (regular_op_t)> Proc
  template<typename T, typename Proc>
  struct instrumented_regular_t {
    T value = T{};
    Proc on_op = Proc{};
  // Regular:
    explicit instrumented_regular_t() {
      if (!ka::empty(on_op)) {
        on_op(regular_op_default_construct);
      }
    }

    explicit instrumented_regular_t(T value, Proc p = {})
        : value(std::move(value)), on_op(std::move(p)) {
      on_op(regular_op_construct);
    }

    instrumented_regular_t(instrumented_regular_t const& x)
        : value(x.value), on_op(x.on_op) {
      on_op(regular_op_copy);
    }

    instrumented_regular_t(instrumented_regular_t&& x)
        : value(std::move(x.value)), on_op(std::move(x.on_op)) {
      on_op(regular_op_move);
    }

    instrumented_regular_t& operator=(instrumented_regular_t const& x) {
      value = x.value;
      on_op = x.on_op;
      on_op(regular_op_assign);
      return *this;
    }

    instrumented_regular_t& operator=(instrumented_regular_t&& x) {
      value = std::move(x.value);
      on_op = std::move(x.on_op);
      on_op(regular_op_move_assign);
      return *this;
    }

    ~instrumented_regular_t() {
      on_op(regular_op_destroy);
    }

    // Callbacks are not parts of the object regarding regularity.
    bool operator==(instrumented_regular_t const& x) const {
      bool const b = (value == x.value);
      on_op(regular_op_equality);
      return b;
    }

    bool operator<(instrumented_regular_t const& x) const {
      bool const b = (value < x.value);
      on_op(regular_op_ordering);
      return b;
    }

    // Generates `!=`, `>`, `<=`, `>=` based on `==` and `<`.
    KA_GENERATE_FRIEND_REGULAR_DERIVED_OPS(instrumented_regular_t)
  };

  /// Initializes an `instrumented_regular_t` with the given value and makes it
  /// use the given counters (no copy is done, meaning the counters must outlive
  /// the `instrumented_regular_t`).
  ///
  /// EmptyProcedure<_ (regular_op_t)> Proc
  template<typename T, typename Proc>
  void init_with_counters(instrumented_regular_t<T, Proc>* x, T value,
      regular_counters_t* counts) {
    x->value = std::move(value);
    x->on_op = [=](regular_op_t i) {
      ++((*counts)[i]);
    };
  }

  inline regular_counters_t& regular_op_global_counters() KA_NOEXCEPT(true) {
    static regular_counters_t c{};
    return c;
  }

  struct on_regular_op_global_t {
  // Regular:
    KA_GENERATE_FRIEND_REGULAR_OPS_0(on_regular_op_global_t)
  // EmptyProcedure<void (ka::regular_op_t)>:
    bool empty() const KA_NOEXCEPT(true) {
      return false;
    }
    void operator()(regular_op_t x) const KA_NOEXCEPT(true) {
      ++regular_op_global_counters()[x];
    }
  };

  struct on_regular_op_local_t {
    regular_counters_t* counters;

  // Regular:
    explicit on_regular_op_local_t(regular_counters_t* c = {}) KA_NOEXCEPT(true)
      : counters(c) {
    }
    KA_GENERATE_FRIEND_REGULAR_OPS_1(on_regular_op_local_t, counters)

  // EmptyProcedure<void (ka::regular_op_t)>:
    bool empty() const KA_NOEXCEPT(true) {
      return counters == nullptr;
    }
    void operator()(regular_op_t x) const KA_NOEXCEPT(true) {
      BOOST_ASSERT(!empty());
      ++(*counters)[x];
    }
  };
} // namespace ka

#endif // KA_TESTUTILS_HPP
