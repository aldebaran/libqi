#include <ka/conceptpredicate.hpp>
#include <ka/relationpredicate.hpp>
#include "test_relations.hpp"
#include <ka/functional.hpp>
#include <ka/functor.hpp>
#include <ka/range.hpp>
#include <ka/typetraits.hpp>
#include <ka/macroregular.hpp>
#include <ka/testutils.hpp>
#include <gtest/gtest.h>
#include <cstdint>

#include <iterator>
#include <vector>
#include <functional>
#include <iostream>
#include <cassert>

// This file contains tests on concept predicates, for example `is_regular`.

namespace test_conceptpredicate {
  template<typename T>
  bool operator!=(T const& a, T const& b) {
    return !(a == b);
  }

  struct regular_t {
    using self = regular_t;
    int x = 0;
    regular_t(int x) : x(x) {}
  // Regular:
    // Copy, assignment and destruction by default.
    regular_t() = default;
    KA_GENERATE_FRIEND_REGULAR_OPS_1(self, x)
  };

  struct regular_stateless_t {
  // Regular:
    KA_GENERATE_FRIEND_REGULAR_OPS_0(regular_stateless_t)
  };

  struct equality_of_address_t {
    using self = equality_of_address_t;
    int x = 0;
    equality_of_address_t(int x) : x(x) {}
  // Regular:
    // Copy, assignment and destruction by default.
    equality_of_address_t() = default;
    friend bool operator==(self const& a, self const& b) {
      return &a == &b;
    }
    friend bool operator<(self a, self b) {
      return a.x < b.x;
    }
  };

  struct less_is_less_equal_t {
    using self = less_is_less_equal_t;
    int x = 0;
    less_is_less_equal_t(int x) : x(x) {}
  // Regular:
    // Copy, assignment and destruction by default.
    less_is_less_equal_t() = default;
    friend bool operator==(self a, self b) {
      return a.x == b.x;
    }
    friend bool operator<(self a, self b) {
      return a.x <= b.x;
    }
  };

  struct bad_assignment_t {
    using self = bad_assignment_t;
    int x = 0;
    bad_assignment_t(int x) : x(x) {}
  // Regular:
    // Destruction by default.
    bad_assignment_t() = default;
    bad_assignment_t(self const&) = default;
    void operator=(self const& a) {
      x = a.x + 1;
    }
    friend bool operator==(self a, self b) {
      return a.x == b.x;
    }
    friend bool operator<(self a, self b) {
      return a.x < b.x;
    }
  };

  struct bad_copy_t {
    using self = bad_copy_t;
    int x = {};
  // Regular:
    // Default constructor, destruction by default.
    bad_copy_t() = default;
    bad_copy_t(int x) : x{x} {
    }
    bad_copy_t(self const& a) : x{a.x + 1} {
    }
    bad_copy_t& operator=(self const& a) = default;
    friend bool operator==(self a, self b) {
      return a.x == b.x;
    }
    friend bool operator<(self a, self b) {
      return a.x < b.x;
    }
  };

  // We don't want to add operator++ to the (pseudo) regular types above,
  // to ensure they are only Regular (minimal interface), but we want
  // for test purpose to easily create incremental ranges.
  struct incr_member_x_t {
    template<typename T>
    void operator()(T& t) const {
      ++t.x;
    }
  } incr_member_x;

} // namespace test_conceptpredicate

// Spaces cause problems in the macros (e.g EXPECT_TRUE).
using longlong = long long;
using longdouble = long double;
using unsignedchar = unsigned char;
using unsignedshort = unsigned short;
using unsignedint = unsigned int;
using unsignedlong = unsigned long;
using unsignedlonglong = unsigned long long;

TEST(ConceptPredicate, IsRegularBuiltinTypes) {
  using namespace ka;
  EXPECT_TRUE(is_regular({true, false}));
  EXPECT_TRUE(is_regular(bounded_range(char(0), char(10))));
  EXPECT_TRUE(is_regular(bounded_range(short(0), short(10))));
  EXPECT_TRUE(is_regular(bounded_range(int(0), int(10))));
  EXPECT_TRUE(is_regular(bounded_range(long(0), long(10))));
  EXPECT_TRUE(is_regular(bounded_range(longlong(0), longlong(10))));
  EXPECT_TRUE(is_regular(bounded_range(float(0), float(10))));
  EXPECT_TRUE(is_regular(bounded_range(double(0), double(10))));
  EXPECT_TRUE(is_regular(bounded_range(longdouble(0), longdouble(10))));

  EXPECT_TRUE(is_regular(bounded_range(unsignedchar(0), unsignedchar(10))));
  EXPECT_TRUE(is_regular(bounded_range(unsignedshort(0), unsignedshort(10))));
  EXPECT_TRUE(is_regular(bounded_range(unsignedint(0), unsignedint(10))));
  EXPECT_TRUE(is_regular(bounded_range(unsignedlong(0), unsignedlong(10))));
  EXPECT_TRUE(is_regular(bounded_range(unsignedlonglong(0), unsignedlonglong(10))));
}

TEST(ConceptPredicate, IsRegularUserDefinedTypes) {
  using namespace ka;
  using namespace test_conceptpredicate;

  EXPECT_TRUE(is_regular(bounded_range(regular_t{0}, regular_t{10}, incr_member_x)));
  {
    regular_stateless_t r;
    EXPECT_TRUE(is_regular(bounded_range(&r, &r + 1)));
  }
  EXPECT_FALSE(is_regular(bounded_range(equality_of_address_t{0}, equality_of_address_t{10}, incr_member_x)));
  EXPECT_FALSE(is_regular(bounded_range(less_is_less_equal_t{0}, less_is_less_equal_t{10}, incr_member_x)));
  EXPECT_FALSE(is_regular(bounded_range(bad_assignment_t{0}, bad_assignment_t{10}, incr_member_x)));
  EXPECT_FALSE(is_regular(bounded_range(bad_copy_t{0}, bad_copy_t{10}, incr_member_x)));
}

namespace test_conceptpredicate {
  using namespace test_relation;

  // Allows specifying equality and total ordering relations.
  template<typename Eq, typename Lt> // Relation<entity_t> Eq, Lt
  struct pseudo_regular_t {
    using self = pseudo_regular_t;
    entity_t x = entity_begin;
    pseudo_regular_t(entity_t x) : x(x) {
    }
  // Regular:
    pseudo_regular_t() = default;
    // Default constructor, copy, assignment and destruction by default.
    friend bool operator==(self a, self b) {
      if (a.x == entity_end || b.x == entity_end) return a.x == b.x;
      return Eq{}(a.x, b.x);
    }
    friend bool operator<(self a, self b) {
      return Lt{}(a.x, b.x);
    }
    KA_GENERATE_FRIEND_REGULAR_DERIVED_OPS(self);
  };

  // Relation<entity_t> Eq, Relation<entity_t> Lt
  template<typename Eq = std::equal_to<entity_t>, typename Lt = std::less<entity_t>>
  auto pseudo_regular_entity_range()
      -> decltype(ka::bounded_range(pseudo_regular_t<Eq, Lt>{entity_begin},
                                    pseudo_regular_t<Eq, Lt>{entity_end}, incr_member_x)) {
    using T = pseudo_regular_t<Eq, Lt>;
    return ka::bounded_range(T{entity_begin}, T{entity_end}, incr_member_x);
  }
} // namespace test_conceptpredicate

TEST(ConceptPredicate, IsRegularWrongEquality) {
  using namespace ka;
  using namespace test_conceptpredicate;
  EXPECT_FALSE(is_regular(pseudo_regular_entity_range<not_reflexive_not_symmetric_not_transitive_t>()));
  EXPECT_FALSE(is_regular(pseudo_regular_entity_range<not_reflexive_symmetric_not_transitive_t>()));
  EXPECT_FALSE(is_regular(pseudo_regular_entity_range<reflexive_symmetric_not_transitive_t>()));

  // This one must fail because even if the relation used for equality is an
  // equivalence, the trichotomy law is not respected
  // (i.e. we can have !(a < b) && !(b < a) && !(a == b)).
  EXPECT_FALSE(is_regular(pseudo_regular_entity_range<reflexive_symmetric_transitive_t>()));

  EXPECT_FALSE(is_regular(pseudo_regular_entity_range<reflexive_not_symmetric_not_transitive_t>()));
  EXPECT_FALSE(is_regular(pseudo_regular_entity_range<reflexive_not_symmetric_transitive_t>()));
  EXPECT_FALSE(is_regular(pseudo_regular_entity_range<not_reflexive_symmetric_transitive_t>()));
  EXPECT_FALSE(is_regular(pseudo_regular_entity_range<not_reflexive_not_symmetric_transitive_t>()));
}

TEST(ConceptPredicate, IsRegularWrongOrdering) {
  using namespace ka;
  using namespace test_conceptpredicate;
  using Eq = std::equal_to<entity_t>;
  EXPECT_FALSE(is_regular(pseudo_regular_entity_range<Eq, std::less_equal<entity_t>>()));
  EXPECT_FALSE(is_regular(pseudo_regular_entity_range<Eq, not_trichotomic_not_transitive_t>()));
  EXPECT_FALSE(is_regular(pseudo_regular_entity_range<Eq, trichotomic_not_transitive_t>()));
  EXPECT_FALSE(is_regular(pseudo_regular_entity_range<Eq, not_trichotomic_transitive_t>()));
}

TEST(ConceptPredicate, IsRegularCorrectOrdering) {
  using namespace ka;
  using namespace test_conceptpredicate;
  using Eq = std::equal_to<entity_t>;
  EXPECT_TRUE(is_regular(pseudo_regular_entity_range<Eq, std::less<entity_t>>()));
  EXPECT_TRUE(is_regular(pseudo_regular_entity_range<Eq, trichotomic_transitive_t>()));
}

namespace test_conceptpredicate {
  // Simple test functor.
  template<typename A>
  struct my_functor_t {
    A value;
  // Regular:
    KA_GENERATE_FRIEND_REGULAR_OPS_1(my_functor_t, value)
  // Functor:
    // Function<B (A)> F
    template<typename F>
    auto fmap(F f) const -> my_functor_t<ka::CodomainFor<F, A>> {
      return {f(value)};
    }
  };

  template<typename A>
  auto my_functor(A a) -> my_functor_t<A> {
    return {a};
  }

  using ka::test::A;
  using ka::test::B;
  using ka::test::C;

  struct a_to_b_fn_t {
    int i;
    auto operator()(A a) const -> B {
      return B{a.value + i};
    }
  };

  struct b_to_c_fn_t {
    int i;
    auto operator()(B b) const -> C {
      return C{b.value * i};
    }
  };

  // Compares functions of `my_functor_t`.
  struct my_functor_fn_equ_t {
    // Function<my_functor_t<C> (my_functor_t<A>)> F, G
    template<typename F, typename G>
    auto operator()(F f, G g) const -> bool {
      for (auto i = -100; i != 100; ++i) {
        auto x = my_functor_t<A>{A{i}};
        if (f(x) != g(x)) return false;
      }
      return true;
    }
  } my_functor_fn_equ;

  // Incorrect lift: identity specialization is incorrect.
  template<typename F>
  struct incorrect_lifted_id_t {
    F f;
    template<typename T>
    auto operator()(my_functor_t<T> x) const -> decltype(my_functor(f(x.value))) {
      return my_functor(f(x.value)); // Ok.
    }
  };

  template<>
  struct incorrect_lifted_id_t<ka::id_transfo_t> {
    ka::id_transfo_t f; // ignored
    auto operator()(my_functor_t<A> x) const -> my_functor_t<A> {
      return my_functor(A{x.value.value + 1}); // Incorrect: not identity.
    }
  };

  // Transforms a function T -> U to a function
  // my_functor_t<T> -> my_functor_t<U>.
  struct incorrect_lift_id_t {
    template<typename F>
    auto operator()(F f) const -> incorrect_lifted_id_t<F> {
      return {f};
    }
  };

  // Incorrect lift: `a_to_b_fn_t` specialization is incorrect.
  template<typename F>
  struct incorrect_lifted_compo_t {
    F f;
    template<typename T>
    auto operator()(my_functor_t<T> x) const -> decltype(my_functor(f(x.value))) {
      return my_functor(f(x.value)); // Ok.
    }
  };

  template<>
  struct incorrect_lifted_compo_t<a_to_b_fn_t> {
    a_to_b_fn_t f; // ignored
    auto operator()(my_functor_t<A> x) const -> my_functor_t<B> {
      return my_functor(f(A{x.value.value * 2})); // Incorrect
    }
  };

  // Transforms a function T -> U to a function
  // my_functor_t<T> -> my_functor_t<U>.
  struct incorrect_lift_compo_t {
    template<typename F>
    auto operator()(F f) const -> incorrect_lifted_compo_t<F> {
      return {f};
    }
  };

  auto is_correct(ka::fmap_fn_t) -> bool {return true;}
  auto is_correct(incorrect_lift_id_t) -> bool {return false;}
  auto is_correct(incorrect_lift_compo_t) -> bool {return false;}

  using lift_types = testing::Types<
    ka::fmap_fn_t, incorrect_lift_id_t, incorrect_lift_compo_t
  >;
} // namespace test_conceptpredicate

template<typename T>
struct ConceptPredicateIsFunctor : testing::Test {};
TYPED_TEST_SUITE(ConceptPredicateIsFunctor, test_conceptpredicate::lift_types);

TYPED_TEST(ConceptPredicateIsFunctor, Basic) {
  using namespace ka;
  using namespace test;
  using namespace test_conceptpredicate;
  using F = a_to_b_fn_t;
  using G = b_to_c_fn_t;
  auto lift = TypeParam{};

  EXPECT_EQ(
    is_correct(lift),
    is_functor(
      lift,
      array_fn(product(G{2}, F{3}), product(G{1}, F{0}), product(G{-1}, F{7})),
      my_functor_fn_equ
    )
  );
}
