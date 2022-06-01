#include <tuple>
#include <gtest/gtest.h>
#include <ka/opt.hpp>
#include <ka/flatten.hpp>
#include <ka/empty.hpp>
#include <ka/utility.hpp>
#include <ka/testutils.hpp>

namespace test_flatten {
  using ka::test::A;

  // Monoidal operation on `A`.
  auto op(A const& a0, A const& a1) -> A {
    return A{a0.value * a1.value};
  }

#define KA_TEST_PAIR_IMPL(TYPE, MEMBER)       \
  MEMBER a, b;                                \
  KA_GENERATE_FRIEND_REGULAR_OPS_2(TYPE, a, b)

  // Type with a member `flatten` (nested version only).
  template<typename M>
  struct Pair {
    KA_TEST_PAIR_IMPL(Pair, M)
  };

  template<typename M>
  struct Pair<Pair<M>> {
    KA_TEST_PAIR_IMPL(Pair, Pair<M>)

    // Reduces values through the monoidal operation.
    auto flatten() const -> Pair<M> {
      return {op(a.a, b.a), op(a.b, b.b)};
    }
  };

  // Type with a free `flatten`.
  template<typename M>
  struct Pair2 {
    KA_TEST_PAIR_IMPL(Pair2, M)
  };

  template<typename M>
  auto flatten(Pair2<Pair2<M>> const& x) -> Pair2<M> {
    return {op(x.a.a, x.b.a), op(x.a.b, x.b.b)};
  }


#undef KA_TEST_PAIR_IMPL

  using pair_types = testing::Types<
    std::tuple<Pair<A>, Pair<Pair<A>>>,
    std::tuple<Pair2<A>, Pair2<Pair2<A>>>
  >;
} // namespace test_flatten

template<typename Traits>
struct Flatten : testing::Test {
};

TYPED_TEST_SUITE(Flatten, test_flatten::pair_types);

// `ka::flatten` calls the `flatten` member or free function, if any.
TYPED_TEST(Flatten, Basic) {
  using ka::test::A;
  int i = 3, j = 2, r = -1, s = 5;
  using PA = typename std::tuple_element<0, TypeParam>::type;
  using PPA = typename std::tuple_element<1, TypeParam>::type;
  auto x0 = PA{A{i}, A{r}};
  auto x1 = PA{A{j}, A{s}};
  EXPECT_EQ((PA{A{i * j}, A{r * s}}), (ka::flatten(PPA{x0, x1})));
}
