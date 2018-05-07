#include <ka/conceptpredicate.hpp>
#include <ka/relationpredicate.hpp>
#include "test_relations.hpp"
#include <ka/range.hpp>
#include <ka/typetraits.hpp>
#include <ka/macroregular.hpp>
#include <gtest/gtest.h>
#include <cstdint>

#include <iterator>
#include <vector>
#include <functional>
#include <iostream>
#include <cassert>

// This file contains tests on concept predicates, for example `is_regular`.

namespace test {
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
    // Copy and destruction by default.
    bad_assignment_t() = default;
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
    // Default constructor, copy and destruction by default.
    bad_copy_t() = default;
    bad_copy_t(int x) : x{x} {
    }
    bad_copy_t(self const& a) : x{a.x + 1} {
    }
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

} // namespace test

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
  using namespace test;

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

namespace test {
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
} // namespace test

TEST(ConceptPredicate, IsRegularWrongEquality) {
  using namespace ka;
  using namespace test;
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
  using namespace test;
  using Eq = std::equal_to<entity_t>;
  EXPECT_FALSE(is_regular(pseudo_regular_entity_range<Eq, std::less_equal<entity_t>>()));
  EXPECT_FALSE(is_regular(pseudo_regular_entity_range<Eq, not_trichotomic_not_transitive_t>()));
  EXPECT_FALSE(is_regular(pseudo_regular_entity_range<Eq, trichotomic_not_transitive_t>()));
  EXPECT_FALSE(is_regular(pseudo_regular_entity_range<Eq, not_trichotomic_transitive_t>()));
}

TEST(ConceptPredicate, IsRegularCorrectOrdering) {
  using namespace ka;
  using namespace test;
  using Eq = std::equal_to<entity_t>;
  EXPECT_TRUE(is_regular(pseudo_regular_entity_range<Eq, std::less<entity_t>>()));
  EXPECT_TRUE(is_regular(pseudo_regular_entity_range<Eq, trichotomic_transitive_t>()));
}
