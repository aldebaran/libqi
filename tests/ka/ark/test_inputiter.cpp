#include <algorithm>
#include <gtest/gtest.h>
#include <ka/ark/inputiter.hpp>
#include <ka/testutils.hpp>

namespace {
  template<typename I, typename Val, typename Diff, typename Ref, typename Ptr>
  void test_traits() {
    using namespace ka;
    using J = ark::input_iter_t<I>;
    static_assert(Equal<std::input_iterator_tag, typename std::iterator_traits<J>::iterator_category>::value, "");
    static_assert(Equal<Val, typename std::iterator_traits<J>::value_type>::value, "");
    static_assert(Equal<Diff, typename std::iterator_traits<J>::difference_type>::value, "");
    static_assert(Equal<Ref, typename std::iterator_traits<J>::reference>::value, "");
    static_assert(Equal<Ptr, typename std::iterator_traits<J>::pointer>::value, "");
  }
} // namespace

// Test traits for various underlying iterators.
TEST(ArkInputIter, Traits) {
  test_traits<int*, int, std::ptrdiff_t, int&, int*>();
  test_traits<int const*, int, std::ptrdiff_t, int const&, int const*>();
  test_traits<std::vector<int>::iterator, int, std::ptrdiff_t, int&, int*>();
  test_traits<std::vector<int>::const_iterator, int, std::ptrdiff_t, int const&, int const*>();
  test_traits<std::list<int>::iterator, int, std::ptrdiff_t, int&, int*>();
  test_traits<std::list<int>::const_iterator, int, std::ptrdiff_t, int const&, int const*>();
}

template<typename T>
struct ArkInputIterTemplate : testing::Test {
};

// Containers in a broad sense (a native array is not a standard container).
using container_types = testing::Types<
  ka::test::A[3],
  std::array<ka::test::A, 3>,
  std::vector<ka::test::A>,
  std::list<ka::test::A>
>;

TYPED_TEST_SUITE(ArkInputIterTemplate, container_types);

// Basic manipulations (an input iterator is not a regular value so `is_regular`
// is not used).
TYPED_TEST(ArkInputIterTemplate, Basic) {
  using namespace ka;
  using test::A;
  TypeParam a{A{0}, A{1}, A{2}};
  auto i = ark::input_iter(std::begin(a));
  decltype(i) j;         // default-construction
  j = i;                 // assignment
  ASSERT_EQ(i, j);       // equality
  ++i;                   // increment
  ASSERT_NE(i, j);       // inequality
  ASSERT_EQ(A{1}, *i);   // dereferencement
  ASSERT_EQ(A{1}, *i++); // post-increment
  ASSERT_EQ(A{2}, *i);
  *i = A{3};             // assignment to dereference
  ASSERT_EQ(A{3}, *i);
  i->value = 4;
  ASSERT_EQ(A{4}, *i);
}

// Test with a standard algorithm.
TYPED_TEST(ArkInputIterTemplate, Find) {
  using namespace ka;
  using test::A;
  TypeParam a{A{0}, A{1}, A{2}};
  auto i = std::find(ark::input_iter(std::begin(a)), ark::input_iter(std::end(a)), A{2});
  ASSERT_EQ(ark::input_iter(std::next(std::begin(a), 2)), i);
}
