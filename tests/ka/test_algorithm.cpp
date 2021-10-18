#include <gtest/gtest.h>
#include <ka/algorithm.hpp>
#include <string>
#include <vector>
#include <deque>
#include <list>
#include <forward_list>
#include <map>
#include <unordered_map>
#include <set>
#include <unordered_set>
#include <boost/container/static_vector.hpp>
#include <boost/container/small_vector.hpp>
#include <boost/container/slist.hpp>
#include <boost/container/flat_set.hpp>
#include <boost/container/flat_map.hpp>

template<typename T>
struct EraseIfSequence : testing::Test {
};

using sequences = testing::Types<
  std::string, std::vector<int>, std::deque<int>, std::list<int>,
  std::forward_list<int>, boost::container::static_vector<int, 9>,
  boost::container::small_vector<int, 9>, boost::container::slist<int>>;

TYPED_TEST_SUITE(EraseIfSequence, sequences);

struct even_t {
  /// Arithmetic N
  template<typename N>
  bool operator()(N n) const {
    return n % N{2} == N{0};
  }
};

TYPED_TEST(EraseIfSequence, SequencesEmpty) {
  using S = TypeParam;
  S s;
  ka::erase_if(s, even_t{});
  EXPECT_EQ(S{}, s);
}

TYPED_TEST(EraseIfSequence, SequencesPredicateAlwaysFalse) {
  using S = TypeParam;
  S s{1, 3, 5};
  ka::erase_if(s, even_t{});
  EXPECT_EQ((S{1, 3, 5}), s);
}

TYPED_TEST(EraseIfSequence, SequencesPredicateAlwaysTrue) {
  using S = TypeParam;
  S s{4, 6, 8, 10};
  ka::erase_if(s, even_t{});
  EXPECT_EQ(S{}, s);
}

TYPED_TEST(EraseIfSequence, SequencesStandardCase) {
  using S = TypeParam;
  S s{1, 4, 6, 8, 3};
  ka::erase_if(s, even_t{});
  EXPECT_EQ((S{1, 3}), s);
}

template<typename T>
struct EraseIfAssociativeSequence : testing::Test {};

using associative_sequences = testing::Types<
  std::map<int, char>, std::multimap<int, char>, std::unordered_map<int, char>,
  boost::container::flat_map<int, char>, boost::container::flat_multimap<int, char>>;

TYPED_TEST_SUITE(EraseIfAssociativeSequence, associative_sequences);

struct even_key_t {
  /// Pair<Arithmetic, _> T
  template<typename T>
  bool operator()(T const& pair) const {
    using N = decltype(pair.first);
    return pair.first % N{2} == N{0};
  }
};

TYPED_TEST(EraseIfAssociativeSequence, AssociativeSequencesEmpty) {
  using S = TypeParam;
  S s;
  ka::erase_if(s, even_key_t{});
  EXPECT_EQ(S{}, s);
}

TYPED_TEST(EraseIfAssociativeSequence, AssociativeSequencesPredicateAlwaysFalse) {
  using S = TypeParam;
  S s{{1, 'a'}, {3, 'c'}, {5, 'e'}};
  ka::erase_if(s, even_key_t{});
  EXPECT_EQ((S{{1, 'a'}, {3, 'c'}, {5, 'e'}}), s);
}

TYPED_TEST(EraseIfAssociativeSequence, AssociativeSequencesPredicateAlwaysTrue) {
  using S = TypeParam;
  S s{{4, 'd'}, {6, 'f'}, {8, 'h'}, {10, 'j'}};
  ka::erase_if(s, even_key_t{});
  EXPECT_EQ(S{}, s);
}

TYPED_TEST(EraseIfAssociativeSequence, AssociativeSequencesStandardCase) {
  using S = TypeParam;
  S s{{1, 'a'}, {4, 'd'}, {6, 'f'}, {8, 'h'}, {3, 'c'}};
  ka::erase_if(s, even_key_t{});
  EXPECT_EQ((S{{1, 'a'}, {3, 'c'}}), s);
}

template<typename T>
struct EraseIfSet : testing::Test {};

using sets = testing::Types<
  std::set<int>, std::multiset<int>, std::unordered_set<int>,
  boost::container::flat_set<int>, boost::container::flat_multiset<int>>;

TYPED_TEST_SUITE(EraseIfSet, sets);

TYPED_TEST(EraseIfSet, SetsEmpty) {
  using S = TypeParam;
  S s;
  ka::erase_if(s, even_t{});
  EXPECT_EQ(S{}, s);
}

TYPED_TEST(EraseIfSet, SetsPredicateAlwaysFalse) {
  using S = TypeParam;
  S s{1, 3, 5, 3};
  ka::erase_if(s, even_t{});
  EXPECT_EQ((S{1, 3, 5, 3}), s);
}

TYPED_TEST(EraseIfSet, SetsPredicateAlwaysTrue) {
  using S = TypeParam;
  S s{4, 6, 8, 4, 10, 6};
  ka::erase_if(s, even_t{});
  EXPECT_EQ(S{}, s);
}

TYPED_TEST(EraseIfSet, SetsPredicateStandardCase) {
  using S = TypeParam;
  S s{1, 4, 1, 6, 6, 8, 3, 8, 3};
  ka::erase_if(s, even_t{});
  EXPECT_EQ((S{1, 1, 3, 3}), s);
}
