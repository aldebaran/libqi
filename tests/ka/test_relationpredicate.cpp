#include <ka/relationpredicate.hpp>
#include "test_relations.hpp"
#include <gtest/gtest.h>
#include <cmath>
#include <cstdint>
#include <ka/range.hpp>

/// @file
/// This file tests predicates on relations (is_transitive, is_total_ordering, etc.).
/// See relationpredicate.hpp

TEST(RelationPredicate, IsTransitive) {
  using namespace ka;
  using namespace test;
  using N = int;
  auto ints_0_to_9 = bounded_range(N{0}, N{10}); // 10 is excluded
  {
    auto eq = [](N a, N b) {return a == b;};
    EXPECT_TRUE(is_transitive(eq, ints_0_to_9));
  } {
    auto lt = [](N a, N b) {return a < b;};
    EXPECT_TRUE(is_transitive(lt, ints_0_to_9));
  } {
    auto ne = [](N a, N b) {return a != b;};
    EXPECT_FALSE(is_transitive(ne, ints_0_to_9));
    EXPECT_TRUE(is_transitive(ne, repeat_range(5, 10)));
  } {
    auto true_ = [](N, N) {return true;};
    EXPECT_TRUE(is_transitive(true_, ints_0_to_9));
  } {
    auto false_ = [](N, N) {return false;};
    EXPECT_TRUE(is_transitive(false_, ints_0_to_9));
  } {
    int wolf = 0, deer = 1, grass = 2, end = 3;
    auto eat = [=](int a, int b) {
      return (a == wolf && b == deer) || (a == deer && b == grass);
    };
    EXPECT_FALSE(is_transitive(eat, bounded_range(wolf, end)));
  } {
    auto persons = bounded_range(person_begin, person_end);
    // Because transitivity implies that since bob is the brother of joe,
    // and joe is the brother of bob, bob is its own brother (which is false).
    EXPECT_FALSE(is_transitive(is_brother_t{}, persons));
    EXPECT_TRUE(is_transitive(share_a_parent_t{}, persons));
  } {
    auto entities = bounded_range(entity_begin, entity_end);
    EXPECT_FALSE(is_transitive(not_reflexive_not_symmetric_not_transitive_t{}, entities));
    EXPECT_FALSE(is_transitive(not_reflexive_symmetric_not_transitive_t{}, entities));
    EXPECT_TRUE(is_transitive(reflexive_symmetric_transitive_t{}, entities));
    EXPECT_FALSE(is_transitive(reflexive_not_symmetric_not_transitive_t{}, entities));
    EXPECT_FALSE(is_transitive(reflexive_symmetric_not_transitive_t{}, entities));
    EXPECT_TRUE(is_transitive(reflexive_not_symmetric_transitive_t{}, entities));
    EXPECT_TRUE(is_transitive(not_reflexive_symmetric_transitive_t{}, entities));
    EXPECT_TRUE(is_transitive(not_reflexive_not_symmetric_transitive_t{}, entities));

    EXPECT_FALSE(is_transitive(not_trichotomic_not_transitive_t{}, entities));
    EXPECT_FALSE(is_transitive(trichotomic_not_transitive_t{}, entities));
    EXPECT_TRUE(is_transitive(not_trichotomic_transitive_t{}, entities));
    EXPECT_TRUE(is_transitive(trichotomic_transitive_t{}, entities));
  }
}

TEST(RelationPredicate, IsReflexive) {
  using namespace ka;
  using namespace test;
  using N = int;
  auto ints_0_to_9 = bounded_range(N{0}, N{10}); // 10 is excluded
  {
    auto eq = [](N a, N b) {return a == b;};
    auto ne = [](N a, N b) {return a != b;};
    auto lt = [](N a, N b) {return a < b;};
    EXPECT_TRUE(is_reflexive(eq, ints_0_to_9));
    EXPECT_FALSE(is_reflexive(ne, ints_0_to_9));
    EXPECT_FALSE(is_reflexive(lt, ints_0_to_9));
    EXPECT_TRUE(is_reflexive(eq, repeat_range(5, 10)));
  } {
    auto broken_eq = [](N a, N b) {
      if (a == N{3} && b == N{3}) return false;
      return a == b;
    };
    EXPECT_FALSE(is_reflexive(broken_eq, ints_0_to_9));
  } {
    auto true_ = [](N, N) {return true;};
    EXPECT_TRUE(is_reflexive(true_, ints_0_to_9));
  } {
    auto false_ = [](N, N) {return false;};
    EXPECT_FALSE(is_reflexive(false_, ints_0_to_9));
  } {
    auto persons = bounded_range(person_begin, person_end);
    EXPECT_FALSE(is_reflexive(is_brother_t{}, persons));
    EXPECT_TRUE(is_reflexive(share_a_parent_t{}, persons));
    EXPECT_FALSE(is_reflexive(is_brother_t{}, persons));
  } {
    auto entities = bounded_range(entity_begin, entity_end);
    EXPECT_FALSE(is_reflexive(not_reflexive_not_symmetric_not_transitive_t{}, entities));
    EXPECT_FALSE(is_reflexive(not_reflexive_symmetric_not_transitive_t{}, entities));
    EXPECT_TRUE(is_reflexive(reflexive_symmetric_transitive_t{}, entities));
    EXPECT_TRUE(is_reflexive(reflexive_not_symmetric_not_transitive_t{}, entities));
    EXPECT_TRUE(is_reflexive(reflexive_symmetric_not_transitive_t{}, entities));
    EXPECT_TRUE(is_reflexive(reflexive_not_symmetric_transitive_t{}, entities));
    EXPECT_FALSE(is_reflexive(not_reflexive_symmetric_transitive_t{}, entities));
    EXPECT_FALSE(is_reflexive(not_reflexive_not_symmetric_transitive_t{}, entities));
  }
}

namespace {
  // Tag-identified int.
  // Used below to define a broken equality.
  static int next_int_tag = 0;
  struct int_t {
    int i;
    int tag;

    explicit int_t(int i) : i(i), tag(next_int_tag++) {
    }
    int_t() : i{0}, tag(next_int_tag++) {}
    int_t(int_t const& x) : i(x.i), tag(next_int_tag++) {
    }
    void operator=(int_t const& x) {
      i = x.i;
      tag = next_int_tag++;
    }
    friend bool operator==(int_t x, int_t y) {
      return x.i == y.i;
    }
    friend bool operator<(int_t x, int_t y) {
      return x.i < y.i;
    }
    friend void operator++(int_t& x) {
      ++x.i;
    }
    KA_GENERATE_FRIEND_REGULAR_DERIVED_OPS(int_t)
  };
} // namespace

TEST(RelationPredicate, IsSymmetric) {
  using namespace ka;
  using namespace test;
  using N = int_t;
  auto ints_0_to_9 = bounded_range(N{0}, N{10}); // 10 is excluded
  {
    auto eq = [](N a, N b) {return a == b;};
    auto ne = [](N a, N b) {return a != b;};
    auto lt = [](N a, N b) {return a < b;};
    EXPECT_TRUE(is_symmetric(eq, ints_0_to_9));
    EXPECT_FALSE(is_symmetric(lt, ints_0_to_9));
    EXPECT_TRUE(is_symmetric(ne, ints_0_to_9));
    EXPECT_TRUE(is_symmetric(eq, repeat_range(N{5}, N{10})));
  } {
    // We test here with a relation that is not symmetric : we use a tag to identify instances.
    // The tag is unique to the instance.
    auto broken_eq = [=](N const& a, N const& b) mutable {
      // Make it fail for a specific value : it will be true in one direction, but not in the other.
      if (a == N{3} && b == N{3}) {
        return a.tag < b.tag;
      }
      return a == b;
    };
    EXPECT_FALSE(is_symmetric(broken_eq, ints_0_to_9));
  } {
    auto true_ = [](N, N) {return true;};
    EXPECT_TRUE(is_symmetric(true_, ints_0_to_9));
  } {
    auto false_ = [](N, N) {return false;};
    EXPECT_TRUE(is_symmetric(false_, ints_0_to_9));
  } {
    auto persons = bounded_range(person_begin, person_end);
    EXPECT_TRUE(is_symmetric(is_brother_t{}, persons));
    EXPECT_TRUE(is_symmetric(share_a_parent_t{}, persons));
  } {
    auto entities = bounded_range(entity_begin, entity_end);
    EXPECT_FALSE(is_symmetric(not_reflexive_not_symmetric_not_transitive_t{}, entities));
    EXPECT_TRUE(is_symmetric(not_reflexive_symmetric_not_transitive_t{}, entities));
    EXPECT_TRUE(is_symmetric(reflexive_symmetric_transitive_t{}, entities));
    EXPECT_FALSE(is_symmetric(reflexive_not_symmetric_not_transitive_t{}, entities));
    EXPECT_TRUE(is_symmetric(reflexive_symmetric_not_transitive_t{}, entities));
    EXPECT_FALSE(is_symmetric(reflexive_not_symmetric_transitive_t{}, entities));
    EXPECT_TRUE(is_symmetric(not_reflexive_symmetric_transitive_t{}, entities));
    EXPECT_FALSE(is_symmetric(not_reflexive_not_symmetric_transitive_t{}, entities));
  }
}

TEST(RelationPredicate, IsEquivalence) {
  using namespace ka;
  using namespace test;
  using N = int;
  auto ints_0_to_9 = bounded_range(N{0}, N{10}); // 10 is excluded
  {
    auto eq = [](N a, N b) {return a == b;};
    auto ne = [](N a, N b) {return a != b;};
    auto lt = [](N a, N b) {return a < b;};
    EXPECT_TRUE(is_equivalence(eq, ints_0_to_9));
    EXPECT_FALSE(is_equivalence(lt, ints_0_to_9));
    EXPECT_FALSE(is_equivalence(ne, ints_0_to_9));
    EXPECT_TRUE(is_equivalence(eq, repeat_range(5, 10)));
  } {
    auto broken_eq = [=](N const& a, N const& b) mutable {
      if (a == N{3} && b == N{3}) return &a < &b;
      return a == b;
    };
    EXPECT_FALSE(is_equivalence(broken_eq, ints_0_to_9));
  } {
    auto persons = bounded_range(person_begin, person_end);
    // isBrother is symmetric, but not reflexive nor transitive.
    EXPECT_FALSE(is_equivalence(is_brother_t{}, persons));
    EXPECT_TRUE(is_equivalence(share_a_parent_t{}, persons));
  } {
    auto entities = bounded_range(entity_begin, entity_end);
    EXPECT_FALSE(is_equivalence(not_reflexive_not_symmetric_not_transitive_t{}, entities));
    EXPECT_FALSE(is_equivalence(not_reflexive_symmetric_not_transitive_t{}, entities));
    EXPECT_TRUE(is_equivalence(reflexive_symmetric_transitive_t{}, entities));
    EXPECT_FALSE(is_equivalence(reflexive_not_symmetric_not_transitive_t{}, entities));
    EXPECT_FALSE(is_equivalence(reflexive_symmetric_not_transitive_t{}, entities));
    EXPECT_FALSE(is_equivalence(reflexive_not_symmetric_transitive_t{}, entities));
    EXPECT_FALSE(is_equivalence(not_reflexive_symmetric_transitive_t{}, entities));
    EXPECT_FALSE(is_equivalence(not_reflexive_not_symmetric_transitive_t{}, entities));
  }
}

TEST(RelationPredicate, AreComplement) {
  using namespace ka;
  using namespace test;
  using N = int;
  auto ints_0_to_9 = bounded_range(N{0}, N{10}); // 10 is excluded
  {
    auto eq = [](N a, N b) {return a == b;};
    auto ne = [](N a, N b) {return a != b;};
    auto lt = [](N a, N b) {return a < b;};
    auto ge = [](N a, N b) {return a >= b;};
    auto gt = [](N a, N b) {return a > b;};
    EXPECT_TRUE(are_complement(eq, ne, ints_0_to_9));
    EXPECT_TRUE(are_complement(ne, eq, ints_0_to_9));
    EXPECT_FALSE(are_complement(eq, eq, ints_0_to_9));
    EXPECT_FALSE(are_complement(ne, ne, ints_0_to_9));
    EXPECT_TRUE(are_complement(lt, ge, ints_0_to_9));
    EXPECT_TRUE(are_complement(ge, lt, ints_0_to_9));
    EXPECT_FALSE(are_complement(lt, lt, ints_0_to_9));
    EXPECT_FALSE(are_complement(ge, ge, ints_0_to_9));
    EXPECT_FALSE(are_complement(lt, gt, ints_0_to_9));
  }
  auto dist = [](N a, N b) -> N {return std::abs(b - a);};
  auto dist_odd = [=](N a, N b) -> bool {return dist(a, b) % 2 == 1;};
  {
    auto dist_even = [=](N a, N b) -> bool {return dist(a, b) % 2 == 0;};
    EXPECT_TRUE(are_complement(dist_even, dist_odd, ints_0_to_9));
  }
  {
    auto pseudo_dist_even = [=](N a, N b) {
      return dist(a, b) % 2 == 0 || (a == 2 && b == 9);
    };
    // Relations both hold for `(2, 9)`.
    EXPECT_FALSE(are_complement(pseudo_dist_even, dist_odd, ints_0_to_9));
  }
}

TEST(RelationPredicate, IsTrichotomic) {
  using namespace ka;
  using namespace test;
  using N = int;
  {
    auto eq = [](N a, N b) {return a == b;};
    auto ne = [](N a, N b) {return a != b;};
    auto lt = [](N a, N b) {return a < b;};
    auto le = [](N a, N b) {return a <= b;};
    auto ints_0_to_9 = bounded_range(N{0}, N{10}); // 10 is excluded
    EXPECT_FALSE(is_trichotomic(eq, ints_0_to_9));
    EXPECT_TRUE(is_trichotomic(lt, ints_0_to_9));
    EXPECT_FALSE(is_trichotomic(le, ints_0_to_9));
    EXPECT_FALSE(is_trichotomic(ne, ints_0_to_9));
    EXPECT_FALSE(is_trichotomic(eq, repeat_range(5, 10)));
  } {
    auto entities = bounded_range(entity_begin, entity_end);
    EXPECT_FALSE(is_trichotomic(not_trichotomic_not_transitive_t{}, entities));
    EXPECT_TRUE(is_trichotomic(trichotomic_not_transitive_t{}, entities));
    EXPECT_FALSE(is_trichotomic(not_trichotomic_transitive_t{}, entities));
    EXPECT_TRUE(is_trichotomic(trichotomic_transitive_t{}, entities));
  }
}

TEST(RelationPredicate, IsWeakOrdering) {
  using namespace ka;
  using namespace test;
  using N = int;
  {
    auto eq = [](N a, N b) {return a == b;};
    auto ne = [](N a, N b) {return a != b;};
    auto lt = [](N a, N b) {return a < b;};
    auto le = [](N a, N b) {return a <= b;};
    auto ints_0_to_9 = bounded_range(N{0}, N{10}); // 10 is excluded
    EXPECT_FALSE(is_weak_ordering(eq, eq, ints_0_to_9));
    EXPECT_TRUE(is_weak_ordering(lt, eq, ints_0_to_9));
    EXPECT_FALSE(is_weak_ordering(le, eq, ints_0_to_9));
    EXPECT_FALSE(is_weak_ordering(ne, eq, ints_0_to_9));
    EXPECT_FALSE(is_weak_ordering(eq, eq, repeat_range(5, 10)));
    auto broken_eq = [=](N const& a, N const& b) mutable {
      if (a == N{3} && b == N{3}) return &a < &b;
      return a == b;
    };
    EXPECT_FALSE(is_weak_ordering(broken_eq, eq, ints_0_to_9));
  } {
    auto persons = bounded_range(person_begin, person_end);
    auto personEq = [](person_t a, person_t b) {return a == b;};
    EXPECT_FALSE(is_weak_ordering(is_brother_t{}, personEq, persons));
    EXPECT_FALSE(is_weak_ordering(share_a_parent_t{}, personEq, persons));
  } {
    char const* names[] = {"bob", "jim", "joe", "jack", "jay", "ben"};
    auto lt = [](char const* a, char const* b) {return strcmp(a, b) < 0;};
    auto eq = [](char const* a, char const* b) {return strcmp(a, b) == 0;};
    EXPECT_TRUE(is_weak_ordering(lt, eq, bounded_range(names)));
  } { // Equivalence that is not equality.
    using T = std::pair<bool, int>;
    T pairs[] = {
      {true, 5}, {true, 5},
      {true, 4}, {false, 4}, // For these 2 elements, we have !lt(a, b) && !lt(b, a) && !equiv(a, b).
      {false, 0}, {true, 3}};
    auto lt = [](T const& a, T const& b) {return a.second < b.second;};
    auto equiv = [](T const& a, T const& b) {return a.second == b.second;};
    EXPECT_TRUE(is_weak_ordering(lt, equiv, bounded_range(pairs)));
  } { // Does not respect trichotomy law.
    using T = std::pair<bool, int>;
    T pairs[] = {{true, 5}, {true, 5}, {true, 4}, {false, 4}, {false, 0}, {true, 3}};
    auto lt = [](T const& a, T const& b) {return a.second < b.second;};
    auto equiv = [](T const& a, T const& b) {return a.first == b.first;};
    EXPECT_FALSE(is_weak_ordering(lt, equiv, bounded_range(pairs)));
  }
}

TEST(RelationPredicate, IsTotalOrdering) {
  using namespace ka;
  using namespace test;
  using N = int;
  auto ints_0_to_9 = bounded_range(N{0}, N{10}); // 10 is excluded
  {
    auto eq = [](N a, N b) {return a == b;};
    auto ne = [](N a, N b) {return a != b;};
    auto lt = [](N a, N b) {return a < b;};
    auto le = [](N a, N b) {return a <= b;};
    EXPECT_FALSE(is_total_ordering(eq, ints_0_to_9));
    EXPECT_TRUE(is_total_ordering(lt, ints_0_to_9));
    EXPECT_FALSE(is_total_ordering(le, ints_0_to_9));
    EXPECT_FALSE(is_total_ordering(ne, ints_0_to_9));
    EXPECT_FALSE(is_total_ordering(eq, repeat_range(5, 10)));
  } {
    auto broken_eq = [=](N const& a, N const& b) mutable {
      if (a == N{3} && b == N{3}) return &a < &b;
      return a == b;
    };
    EXPECT_FALSE(is_total_ordering(broken_eq, ints_0_to_9));
  } {
    auto persons = bounded_range(person_begin, person_end);
    EXPECT_FALSE(is_total_ordering(is_brother_t{}, persons));
    EXPECT_FALSE(is_total_ordering(share_a_parent_t{}, persons));
  } {
    char const* names[] = {"bob", "jim", "joe", "jack", "jay", "ben"};
    auto lt = [](char const* a, char const* b) {return strcmp(a, b) < 0;};
    EXPECT_TRUE(is_total_ordering(lt, bounded_range(names)));
  } { // Does not respect trichotomy law.
    using T = std::pair<bool, int>;
    T pairs[] = {
      {true, 5}, {true, 5},
      {true, 4}, {false, 4}, // For these 2 elements, we have !lt(a, b) && !lt(b, a) && !(a == b).
      {false, 0}, {true, 3}};
    auto lt = [](T const& a, T const& b) {return a.second < b.second;};
    EXPECT_FALSE(is_total_ordering(lt, bounded_range(pairs)));
  }
}
