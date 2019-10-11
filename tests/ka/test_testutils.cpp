#include <gtest/gtest.h>
#include <boost/function.hpp>
#include <ka/conceptpredicate.hpp>
#include <ka/macro.hpp>
#include <ka/range.hpp>
#include <ka/testutils.hpp>

namespace {
  template<typename T>
  bool test_type_is_regular() {
    using namespace ka;
    auto incr = [](T& x) {++x.value;};
    return is_regular(bounded_range(T{0}, T{10}, incr));
  }
} // namespace

TEST(TestType, Regular) {
  using namespace ka::test;
  ASSERT_TRUE(test_type_is_regular<A>());
  ASSERT_TRUE(test_type_is_regular<B>());
  ASSERT_TRUE(test_type_is_regular<C>());
  ASSERT_TRUE(test_type_is_regular<D>());
  ASSERT_TRUE(test_type_is_regular<E>());
  ASSERT_TRUE(test_type_is_regular<F>());
  ASSERT_TRUE(test_type_is_regular<G>());
  ASSERT_TRUE(test_type_is_regular<H>());
  ASSERT_TRUE(test_type_is_regular<I>());
  ASSERT_TRUE(test_type_is_regular<J>());
}

namespace {
  template<typename G, typename F>
  struct my_compo_t {
    G g;
    F f;
    template<typename T>
    auto operator()(T const& t) const -> decltype(g(f(t))) {
      return g(f(t));
    }
  };

  KA_DERIVE_CTOR_FUNCTION_TEMPLATE(my_compo)
} // namespace

// Function composition is an example of generic code with strong typing.
// Testing with strong types validates the type-safety of the code.
TEST(TestType, FunctionComposition) {
  using namespace ka;
  using namespace ka::test;
  // Replacing `A`, `B`, `C` by builtin types (`int`, `bool`, `char`) would
  // lessen the value of the test, since they are implicitly convertible.
  auto f = [](A a) {return B{a.value + 1};}; // A -> B
  auto g = [](B b) {return C{b.value * 2};}; // B -> C
  ASSERT_EQ(C{10}, my_compo(g, f)(A{4})); // (4 + 1) * 2
}

TEST(InstrumentedRegular, IsRegular) {
  using namespace ka;
  using T = instrumented_regular_t<int, on_regular_op_global_t>;
  auto incr = [](T& x) {++x.value;};
  ASSERT_TRUE(is_regular(bounded_range(T{0}, T{10}, incr)));
}

// Counting regular operations via global counters.
// All regular operations are handled.
TEST(InstrumentedRegular, GlobalCounters) {
  using namespace ka;
  auto& counters = regular_op_global_counters();
  reset(counters);
  for (auto x: counters) {
    ASSERT_EQ(0, x);
  }
  using T = instrumented_regular_t<int, on_regular_op_global_t>;
  {
    T x0;
    ASSERT_EQ(1, counters[regular_op_default_construct]);
    {
      T x1{5};
      ASSERT_EQ(1, counters[regular_op_construct]);

      x0 = x1;
      ASSERT_EQ(1, counters[regular_op_assign]);

      bool equal0 = (x0 == x1);
      ASSERT_TRUE(equal0);
      ASSERT_EQ(1, counters[regular_op_equality]);

      bool less0 = (x0 < x1);
      ASSERT_FALSE(less0);
      ASSERT_EQ(1, counters[regular_op_ordering]);
    }
    ASSERT_EQ(1, counters[regular_op_destroy]); // because of x1
  }
  ASSERT_EQ(2, counters[regular_op_destroy]); // because of x0

  T x2{6};
  ASSERT_EQ(2, counters[regular_op_construct]);

  T x3;
  ASSERT_EQ(2, counters[regular_op_default_construct]);

  x3 = std::move(x2);
  ASSERT_EQ(1, counters[regular_op_move_assign]);

  T x4{std::move(x3)};
  ASSERT_EQ(1, counters[regular_op_move]);

  T x5{std::move(x4)};
  ASSERT_EQ(2, counters[regular_op_move]);
}

// Counting regular operations via local counters.
// All regular operations but default construction are handled.
TEST(InstrumentedRegular, LocalCounters) {
  using namespace ka;
  using T = instrumented_regular_t<int, on_regular_op_local_t>;

  regular_counters_t counters {{}};
  for (auto x: counters) {
    ASSERT_EQ(0, x);
  }
  on_regular_op_local_t on_op{&counters};

  {
    // We cannot count default constructions since we don't pass the counters.
    T x0;
    ASSERT_EQ(0, counters[regular_op_default_construct]);
    {
      T x1{5, on_op};
      ASSERT_EQ(1, counters[regular_op_construct]);

      x0 = x1;
      ASSERT_EQ(1, counters[regular_op_assign]);

      bool equal0 = (x0 == x1);
      ASSERT_TRUE(equal0);
      ASSERT_EQ(1, counters[regular_op_equality]);

      bool less0 = (x0 < x1);
      ASSERT_FALSE(less0);
      ASSERT_EQ(1, counters[regular_op_ordering]);
    }
    ASSERT_EQ(1, counters[regular_op_destroy]); // because of x1
  }
  ASSERT_EQ(2, counters[regular_op_destroy]); // because of x0

  T x2{6, on_op};
  ASSERT_EQ(2, counters[regular_op_construct]);

  T x3;
  x3 = std::move(x2);
  ASSERT_EQ(1, counters[regular_op_move_assign]);

  T x4{std::move(x3)};
  ASSERT_EQ(1, counters[regular_op_move]);

  T x5{std::move(x4)};
  ASSERT_EQ(2, counters[regular_op_move]);
}

TEST(ContainerCtorFunction, Basic) {
  using namespace ka::test;

  EXPECT_EQ((std::array<A, 1>{A{1}}), array_fn(A{1}));
  EXPECT_EQ((std::vector<A>{A{1}}), vector_fn(A{1}));
  EXPECT_EQ((std::vector<A>{A{1}, A{2}}), vector_fn(A{1}, A{2}));

  EXPECT_EQ((std::list<B>{B{3}, B{4}}), list_fn(B{3}, B{4}));

  EXPECT_EQ((std::deque<C>{C{3}, C{4}, C{5}}), deque_fn(C{3}, C{4}, C{5}));

  EXPECT_EQ((std::forward_list<D>{D{4}, D{5}, D{6}}),
                     forward_list_fn(D{4}, D{5}, D{6}));

  EXPECT_EQ((std::set<E>{E{4}, E{5}}), set_fn(E{4}, E{5}));
  EXPECT_EQ((std::multiset<F>{F{4}}), multiset_fn(F{4}));

  EXPECT_EQ((std::unordered_set<G>{G{6}, G{7}}),
                  unordered_set_fn(G{6}, G{7}));

  EXPECT_EQ((std::unordered_multiset<A>{A{6}, A{7}}),
                  unordered_multiset_fn(A{6}, A{7}));

  EXPECT_EQ((std::map<A, B>{kv(A{2}, B{4}), kv(A{3}, B{5})}),
                     map_fn(kv(A{2}, B{4}), kv(A{3}, B{5})));

  EXPECT_EQ((std::multimap<C, D>{kv(C{4}, D{5}), kv(C{6}, D{7})}),
                     multimap_fn(kv(C{4}, D{5}), kv(C{6}, D{7})));

  EXPECT_EQ((std::unordered_map<E, F>{kv(E{4}, F{5}), kv(E{6}, F{7})}),
                     unordered_map_fn(kv(E{4}, F{5}), kv(E{6}, F{7})));

  EXPECT_EQ((std::unordered_multimap<G, A>{kv(G{4}, A{5}), kv(G{6}, A{7})}),
                     unordered_multimap_fn(kv(G{4}, A{5}), kv(G{6}, A{7})));
}
