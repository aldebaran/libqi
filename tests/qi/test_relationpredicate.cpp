/*
**
** Copyright (C) 2010-2016, Aldebaran Robotics
*/

#include <qi/detail/relationpredicate.hpp>
#include "test_relations.hpp"
#include <qi/application.hpp>
#include <gtest/gtest.h>
#include <cstdint>
#include <qi/range.hpp>

/// @file
/// This file tests predicates on relations (isTransitive, isTotalOrdering, etc.).
/// See relationpredicate.hpp

TEST(RelationPredicate, isTransitive)
{
  using namespace qi;
  using namespace qi::detail;
  using namespace test;
  using N = int;
  auto ints0To9 = incrRange(N{0}, N{10}); // 10 is excluded
  {
    auto eq = [](N a, N b) {return a == b;};
    EXPECT_TRUE(isTransitive(eq, ints0To9));
  }
  {
    auto lt = [](N a, N b) {return a < b;};
    EXPECT_TRUE(isTransitive(lt, ints0To9));
  }
  {
    auto ne = [](N a, N b) {return a != b;};
    EXPECT_FALSE(isTransitive(ne, ints0To9));
    EXPECT_TRUE(isTransitive(ne, repeatRange(5, 10)));
  }
  {
    auto true_ = [](N, N) {return true;};
    EXPECT_TRUE(isTransitive(true_, ints0To9));
  }
  {
    auto false_ = [](N, N) {return false;};
    EXPECT_TRUE(isTransitive(false_, ints0To9));
  }
  {
    int wolf = 0, deer = 1, grass = 2, end = 3;
    auto eat = [=](int a, int b) {
      return (a == wolf && b == deer) || (a == deer && b == grass);
    };
    EXPECT_FALSE(isTransitive(eat, incrRange(wolf, end)));
  }
  {
    auto persons = incrRange(personBegin, personEnd);
    // Because transitivity implies that since bob is the brother of joe,
    // and joe is the brother of bob, bob is its own brother (which is false).
    EXPECT_FALSE(isTransitive(IsBrother{}, persons));
    EXPECT_TRUE(isTransitive(ShareAParent{}, persons));
  }
  {
    auto entities = incrRange(entityBegin, entityEnd);
    EXPECT_FALSE(isTransitive(NotReflexiveNotSymmetricNotTransitive{}, entities));
    EXPECT_FALSE(isTransitive(NotReflexiveSymmetricNotTransitive{}, entities));
    EXPECT_TRUE(isTransitive(ReflexiveSymmetricTransitive{}, entities));
    EXPECT_FALSE(isTransitive(ReflexiveNotSymmetricNotTransitive{}, entities));
    EXPECT_FALSE(isTransitive(ReflexiveSymmetricNotTransitive{}, entities));
    EXPECT_TRUE(isTransitive(ReflexiveNotSymmetricTransitive{}, entities));
    EXPECT_TRUE(isTransitive(NotReflexiveSymmetricTransitive{}, entities));
    EXPECT_TRUE(isTransitive(NotReflexiveNotSymmetricTransitive{}, entities));

    EXPECT_FALSE(isTransitive(NotTrichotomicNotTransitive{}, entities));
    EXPECT_FALSE(isTransitive(TrichotomicNotTransitive{}, entities));
    EXPECT_TRUE(isTransitive(NotTrichotomicTransitive{}, entities));
    EXPECT_TRUE(isTransitive(TrichotomicTransitive{}, entities));
  }
}

TEST(RelationPredicate, isReflexive)
{
  using namespace qi;
  using namespace qi::detail;
  using namespace test;
  using N = int;
  auto ints0To9 = incrRange(N{0}, N{10}); // 10 is excluded
  {
    auto eq = [](N a, N b) {return a == b;};
    auto ne = [](N a, N b) {return a != b;};
    auto lt = [](N a, N b) {return a < b;};
    EXPECT_TRUE(isReflexive(eq, ints0To9));
    EXPECT_FALSE(isReflexive(ne, ints0To9));
    EXPECT_FALSE(isReflexive(lt, ints0To9));
    EXPECT_TRUE(isReflexive(eq, repeatRange(5, 10)));
  }
  {
    auto brokenEq = [](N a, N b) {
      if (a == N{3} && b == N{3}) return false;
      return a == b;
    };
    EXPECT_FALSE(isReflexive(brokenEq, ints0To9));
  }
  {
    auto true_ = [](N, N) {return true;};
    EXPECT_TRUE(isReflexive(true_, ints0To9));
  }
  {
    auto false_ = [](N, N) {return false;};
    EXPECT_FALSE(isReflexive(false_, ints0To9));
  }
  {
    auto persons = incrRange(personBegin, personEnd);
    EXPECT_FALSE(isReflexive(IsBrother{}, persons));
    EXPECT_TRUE(isReflexive(ShareAParent{}, persons));
    EXPECT_FALSE(isReflexive(IsBrother{}, persons));
  }
  {
    auto entities = incrRange(entityBegin, entityEnd);
    EXPECT_FALSE(isReflexive(NotReflexiveNotSymmetricNotTransitive{}, entities));
    EXPECT_FALSE(isReflexive(NotReflexiveSymmetricNotTransitive{}, entities));
    EXPECT_TRUE(isReflexive(ReflexiveSymmetricTransitive{}, entities));
    EXPECT_TRUE(isReflexive(ReflexiveNotSymmetricNotTransitive{}, entities));
    EXPECT_TRUE(isReflexive(ReflexiveSymmetricNotTransitive{}, entities));
    EXPECT_TRUE(isReflexive(ReflexiveNotSymmetricTransitive{}, entities));
    EXPECT_FALSE(isReflexive(NotReflexiveSymmetricTransitive{}, entities));
    EXPECT_FALSE(isReflexive(NotReflexiveNotSymmetricTransitive{}, entities));
  }
}

TEST(RelationPredicate, isSymmetric)
{
  using namespace qi;
  using namespace qi::detail;
  using namespace test;
  using N = int;
  auto ints0To9 = incrRange(N{0}, N{10}); // 10 is excluded
  {
    auto eq = [](N a, N b) {return a == b;};
    auto ne = [](N a, N b) {return a != b;};
    auto lt = [](N a, N b) {return a < b;};
    EXPECT_TRUE(isSymmetric(eq, ints0To9));
    EXPECT_FALSE(isSymmetric(lt, ints0To9));
    EXPECT_TRUE(isSymmetric(ne, ints0To9));
    EXPECT_TRUE(isSymmetric(eq, repeatRange(5, 10)));
  }
  {
    auto brokenEq = [=](const N& a, const N& b) mutable {
      if (a == N{3} && b == N{3}) return &a < &b;
      return a == b;
    };
    EXPECT_FALSE(isSymmetric(brokenEq, ints0To9));
  }
  {
    auto true_ = [](N, N) {return true;};
    EXPECT_TRUE(isSymmetric(true_, ints0To9));
  }
  {
    auto false_ = [](N, N) {return false;};
    EXPECT_TRUE(isSymmetric(false_, ints0To9));
  }
  {
    auto persons = incrRange(personBegin, personEnd);
    EXPECT_TRUE(isSymmetric(IsBrother{}, persons));
    EXPECT_TRUE(isSymmetric(ShareAParent{}, persons));
  }
  {
    auto entities = incrRange(entityBegin, entityEnd);
    EXPECT_FALSE(isSymmetric(NotReflexiveNotSymmetricNotTransitive{}, entities));
    EXPECT_TRUE(isSymmetric(NotReflexiveSymmetricNotTransitive{}, entities));
    EXPECT_TRUE(isSymmetric(ReflexiveSymmetricTransitive{}, entities));
    EXPECT_FALSE(isSymmetric(ReflexiveNotSymmetricNotTransitive{}, entities));
    EXPECT_TRUE(isSymmetric(ReflexiveSymmetricNotTransitive{}, entities));
    EXPECT_FALSE(isSymmetric(ReflexiveNotSymmetricTransitive{}, entities));
    EXPECT_TRUE(isSymmetric(NotReflexiveSymmetricTransitive{}, entities));
    EXPECT_FALSE(isSymmetric(NotReflexiveNotSymmetricTransitive{}, entities));
  }
}

TEST(RelationPredicate, isEquivalence)
{
  using namespace qi;
  using namespace qi::detail;
  using namespace test;
  using N = int;
  auto ints0To9 = incrRange(N{0}, N{10}); // 10 is excluded
  {
    auto eq = [](N a, N b) {return a == b;};
    auto ne = [](N a, N b) {return a != b;};
    auto lt = [](N a, N b) {return a < b;};
    EXPECT_TRUE(isEquivalence(eq, ints0To9));
    EXPECT_FALSE(isEquivalence(lt, ints0To9));
    EXPECT_FALSE(isEquivalence(ne, ints0To9));
    EXPECT_TRUE(isEquivalence(eq, repeatRange(5, 10)));
  }
  {
    auto brokenEq = [=](const N& a, const N& b) mutable {
      if (a == N{3} && b == N{3}) return &a < &b;
      return a == b;
    };
    EXPECT_FALSE(isEquivalence(brokenEq, ints0To9));
  }
  {
    auto persons = incrRange(personBegin, personEnd);
    // isBrother is symmetric, but not reflexive nor transitive.
    EXPECT_FALSE(isEquivalence(IsBrother{}, persons));
    EXPECT_TRUE(isEquivalence(ShareAParent{}, persons));
  }
  {
    auto entities = incrRange(entityBegin, entityEnd);
    EXPECT_FALSE(isEquivalence(NotReflexiveNotSymmetricNotTransitive{}, entities));
    EXPECT_FALSE(isEquivalence(NotReflexiveSymmetricNotTransitive{}, entities));
    EXPECT_TRUE(isEquivalence(ReflexiveSymmetricTransitive{}, entities));
    EXPECT_FALSE(isEquivalence(ReflexiveNotSymmetricNotTransitive{}, entities));
    EXPECT_FALSE(isEquivalence(ReflexiveSymmetricNotTransitive{}, entities));
    EXPECT_FALSE(isEquivalence(ReflexiveNotSymmetricTransitive{}, entities));
    EXPECT_FALSE(isEquivalence(NotReflexiveSymmetricTransitive{}, entities));
    EXPECT_FALSE(isEquivalence(NotReflexiveNotSymmetricTransitive{}, entities));
  }
}

TEST(RelationPredicate, isTrichotomic)
{
  using namespace qi;
  using namespace qi::detail;
  using namespace test;
  using N = int;
  {
    auto eq = [](N a, N b) {return a == b;};
    auto ne = [](N a, N b) {return a != b;};
    auto lt = [](N a, N b) {return a < b;};
    auto le = [](N a, N b) {return a <= b;};
    auto ints0To9 = incrRange(N{0}, N{10}); // 10 is excluded
    EXPECT_FALSE(isTrichotomic(eq, ints0To9));
    EXPECT_TRUE(isTrichotomic(lt, ints0To9));
    EXPECT_FALSE(isTrichotomic(le, ints0To9));
    EXPECT_FALSE(isTrichotomic(ne, ints0To9));
    EXPECT_FALSE(isTrichotomic(eq, repeatRange(5, 10)));
  }
  {
    auto entities = incrRange(entityBegin, entityEnd);
    EXPECT_FALSE(isTrichotomic(NotTrichotomicNotTransitive{}, entities));
    EXPECT_TRUE(isTrichotomic(TrichotomicNotTransitive{}, entities));
    EXPECT_FALSE(isTrichotomic(NotTrichotomicTransitive{}, entities));
    EXPECT_TRUE(isTrichotomic(TrichotomicTransitive{}, entities));
  }
}

TEST(RelationPredicate, isWeakOrdering)
{
  using namespace qi;
  using namespace qi::detail;
  using namespace test;
  using N = int;
  {
    auto eq = [](N a, N b) {return a == b;};
    auto ne = [](N a, N b) {return a != b;};
    auto lt = [](N a, N b) {return a < b;};
    auto le = [](N a, N b) {return a <= b;};
    auto ints0To9 = incrRange(N{0}, N{10}); // 10 is excluded
    EXPECT_FALSE(isWeakOrdering(eq, eq, ints0To9));
    EXPECT_TRUE(isWeakOrdering(lt, eq, ints0To9));
    EXPECT_FALSE(isWeakOrdering(le, eq, ints0To9));
    EXPECT_FALSE(isWeakOrdering(ne, eq, ints0To9));
    EXPECT_FALSE(isWeakOrdering(eq, eq, repeatRange(5, 10)));
    auto brokenEq = [=](const N& a, const N& b) mutable {
      if (a == N{3} && b == N{3}) return &a < &b;
      return a == b;
    };
    EXPECT_FALSE(isWeakOrdering(brokenEq, eq, ints0To9));
  }
  {
    auto persons = incrRange(personBegin, personEnd);
    auto personEq = [](Person a, Person b) {return a == b;};
    EXPECT_FALSE(isWeakOrdering(IsBrother{}, personEq, persons));
    EXPECT_FALSE(isWeakOrdering(ShareAParent{}, personEq, persons));
  }
  {
    char const* names[] = {"bob", "jim", "joe", "jack", "jay", "ben"};
    auto lt = [](char const* a, char const* b) {return strcmp(a, b) < 0;};
    auto eq = [](char const* a, char const* b) {return strcmp(a, b) == 0;};
    EXPECT_TRUE(isWeakOrdering(lt, eq, boundedRange(names)));
  }
  { // Equivalence that is not equality.
    using T = std::pair<bool, int>;
    T pairs[] = {
      {true, 5}, {true, 5},
      {true, 4}, {false, 4}, // For these 2 elements, we have !lt(a, b) && !lt(b, a) && !equiv(a, b).
      {false, 0}, {true, 3}};
    auto lt = [](const T& a, const T& b) {return a.second < b.second;};
    auto equiv = [](const T& a, const T& b) {return a.second == b.second;};
    EXPECT_TRUE(isWeakOrdering(lt, equiv, boundedRange(pairs)));
  }
  { // Does not respect trichotomy law.
    using T = std::pair<bool, int>;
    T pairs[] = {{true, 5}, {true, 5}, {true, 4}, {false, 4}, {false, 0}, {true, 3}};
    auto lt = [](const T& a, const T& b) {return a.second < b.second;};
    auto equiv = [](const T& a, const T& b) {return a.first == b.first;};
    EXPECT_FALSE(isWeakOrdering(lt, equiv, boundedRange(pairs)));
  }
}

TEST(RelationPredicate, isTotalOrdering)
{
  using namespace qi;
  using namespace qi::detail;
  using namespace test;
  using N = int;
  auto ints0To9 = incrRange(N{0}, N{10}); // 10 is excluded
  {
    auto eq = [](N a, N b) {return a == b;};
    auto ne = [](N a, N b) {return a != b;};
    auto lt = [](N a, N b) {return a < b;};
    auto le = [](N a, N b) {return a <= b;};
    EXPECT_FALSE(isTotalOrdering(eq, ints0To9));
    EXPECT_TRUE(isTotalOrdering(lt, ints0To9));
    EXPECT_FALSE(isTotalOrdering(le, ints0To9));
    EXPECT_FALSE(isTotalOrdering(ne, ints0To9));
    EXPECT_FALSE(isTotalOrdering(eq, repeatRange(5, 10)));
  }
  {
    auto brokenEq = [=](const N& a, const N& b) mutable {
      if (a == N{3} && b == N{3}) return &a < &b;
      return a == b;
    };
    EXPECT_FALSE(isTotalOrdering(brokenEq, ints0To9));
  }
  {
    auto persons = incrRange(personBegin, personEnd);
    EXPECT_FALSE(isTotalOrdering(IsBrother{}, persons));
    EXPECT_FALSE(isTotalOrdering(ShareAParent{}, persons));
  }
  {
    char const* names[] = {"bob", "jim", "joe", "jack", "jay", "ben"};
    auto lt = [](char const* a, char const* b) {return strcmp(a, b) < 0;};
    EXPECT_TRUE(isTotalOrdering(lt, boundedRange(names)));
  }
  { // Does not respect trichotomy law.
    using T = std::pair<bool, int>;
    T pairs[] = {
      {true, 5}, {true, 5},
      {true, 4}, {false, 4}, // For these 2 elements, we have !lt(a, b) && !lt(b, a) && !(a == b).
      {false, 0}, {true, 3}};
    auto lt = [](const T& a, const T& b) {return a.second < b.second;};
    EXPECT_FALSE(isTotalOrdering(lt, boundedRange(pairs)));
  }
}
