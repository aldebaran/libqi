#include <tuple>
#include <ka/memory.hpp>
#include <ka/functorcontainer.hpp>
#include <ka/functor.hpp>
#include <ka/functional.hpp>
#include <ka/utility.hpp>
#include <ka/typetraits.hpp>
#include <ka/testutils.hpp>
#include <ka/opt.hpp>
#include <boost/optional/optional_io.hpp>
#include <gtest/gtest.h>
#include <numeric>
#include <string>
#include <vector>
#include <list>
#include <deque>
#include <forward_list>
#include <unordered_set>
#include <unordered_map>
#include <map>
#include <sstream>

namespace {
  // No-conversion test types.
  using ka::test::A;
  using ka::test::B;
  using ka::test::C;
  using ka::test::D;

  // Polymorphic function that converts to string.
  struct str_t {
    template<typename T>
    std::string operator()(T const& x) const {
      std::ostringstream oss;
      oss << x;
      return oss.str();
    }
    template<typename T, typename U>
    std::pair<std::string, std::string> operator()(std::pair<T, U> const& x) const {
      return {(*this)(x.first), (*this)(x.second)};
    }
    inline
    std::string operator()(A const& x) const {
      return (*this)(x.value);
    }
    inline
    std::string operator()(D const& x) const {
      return (*this)(x.value);
    }
  } str;

  // Converts a test value (of type `A`, `B`, ...) to a `D`-value.
  struct to_d_t {
    template<typename T>
    auto operator()(T const& x) const -> D {
      return D{x.value};
    }
  } to_d;

  // Used instead of `operator==` because it would cause an ambiguity error on
  // clang for `std::function`.
  template<typename T>
  bool test_eq(T const& x, T const& y) {
    return x == y;
  }

  template<typename T, typename U>
  bool test_eq(std::function<U (T)> const& f,
               std::function<U (T)> const& g) {
    // Approximation.
    for (int i = 0; i != 100; ++i) {
      if (f(T{i}) != g(T{i})) return false;
    }
    return true;
  }

  // Function whose equality with another one is decided by their output for a
  // given input.
  template<typename F, typename Tup>
  struct test_eq_fn_t {
    F f; // Function.
    Tup args; // Test input.
    template<typename... T>
    auto operator()(T&&... t) const -> decltype(f(ka::fwd<T>(t)...)) {
      return f(ka::fwd<T>(t)...);
    }
    template<typename G>
    auto operator==(G g) const -> testing::AssertionResult {
      if (!test_eq(ka::apply(f, args), ka::apply(g, args))) return testing::AssertionFailure();
      return testing::AssertionSuccess();
    }
  };

  // Functor whose lifted functions can be compared through a test input.
  template<typename Tup>
  struct test_functor_t {
    Tup args; // Input of lifted functions.
    template<typename F>
    auto operator()(F f) const -> test_eq_fn_t<decltype(ka::fmap(f)), Tup const&> {
      return {ka::fmap(f), args};
    }
  };

  template<typename... A>
  auto test_functor(A&&... args) -> test_functor_t<std::tuple<ka::Decay<A>...>> {
    return {std::make_tuple(ka::fwd<A>(args)...)};
  }

  struct boost_opt_fn_t {
    template<typename T>
    auto operator()(T&& t) const -> boost::optional<ka::Decay<T>> {
      return {ka::fwd<T>(t)};
    }
  };

  template<typename T>
  auto fill_with(boost_opt_fn_t f) -> decltype(f(T{0})) {
    return f(T{0});
  }

  struct ka_opt_fn_t {
    template<typename T>
    auto operator()(T&& t) const -> ka::opt_t<ka::Decay<T>> {
      using namespace ka;
      return opt(fwd<T>(t));
    }
  };

  template<typename T>
  auto fill_with(ka_opt_fn_t f) -> decltype(f(T{0})) {
    return f(T{0});
  }

  template<typename X>
  struct std_function_fn_t {
    template<typename T>
    auto operator()(T&& t) const -> std::function<ka::Decay<T> (X)> {
      return {[t](X const& x) {
        return ka::Decay<T>{t.value + x.value};
      }};
    }
  };

  template<typename T, typename X>
  auto fill_with(std_function_fn_t<X> f) -> decltype(f(T{0})) {
    return f(T{0});
  }

} // namespace

template<typename T>
struct FunctorTest : testing::Test {
};

namespace ka { namespace test {
  using functor_types = testing::Types<
    array_fn_t,
    vector_fn_t, list_fn_t, deque_fn_t, forward_list_fn_t,
    set_fn_t, multiset_fn_t, unordered_set_fn_t, unordered_multiset_fn_t,
    map_fn_t, multimap_fn_t, unordered_map_fn_t, unordered_multimap_fn_t,
    boost_opt_fn_t, ka_opt_fn_t,
    std_function_fn_t<B>
  >;
}} // namespace ka::test

TYPED_TEST_SUITE(FunctorTest, ka::test::functor_types);

// First, we test functor laws for all functor types. To test equality of
// functions, we specify an input for which they must agree, i.e. give same
// output. This is an approximation of extensional equality.
TYPED_TEST(FunctorTest, Laws) {
  using namespace ka;
  using ka::functional_ops::operator*; // Mathematical function composition.
  TypeParam ctor_fn;
  auto const test_input = fill_with<A>(ctor_fn);
  auto F = test_functor(test_input);
  id_transfo_t _1;
  auto f = str;
  auto g = to_d;

  // Lifting of identity is the same as identity.
  EXPECT_TRUE(F(_1) == _1);

  // Lifting of composition is the same as composition of lifting.
  EXPECT_TRUE(F(f * g) == F(f) * F(g));
}

template<typename T>
struct FunctorAppTest : testing::Test {
};

namespace ka { namespace test {
  using functor_app_types = testing::Types<
    array_fn_t,
    vector_fn_t, list_fn_t, deque_fn_t, forward_list_fn_t,
    set_fn_t, multiset_fn_t, unordered_set_fn_t, unordered_multiset_fn_t,
    boost_opt_fn_t, ka_opt_fn_t
  >;
}} // namespace ka::test

TYPED_TEST_SUITE(FunctorAppTest, ka::test::functor_app_types);

namespace {
  auto const sum_abc = [](A a, B b, C c) -> D {
    return D{a.value + b.value + c.value};
  };
} // namespace

TYPED_TEST(FunctorAppTest, Laws) {
  using namespace ka;
  using ka::functional_ops::operator*; // Mathematical function composition.
  TypeParam ctor_fn;
  auto const as = fill_with<A>(ctor_fn);
  auto const bs = fill_with<B>(ctor_fn);
  auto const cs = fill_with<C>(ctor_fn);
  auto F = test_functor(as, bs, cs);
  auto f = str;
  auto g = sum_abc;

  EXPECT_TRUE(F(f * g) == F(f) * F(g));
}

TEST(FunctorApp, LawsDifferentSizes) {
  using namespace ka;
  using ka::functional_ops::operator*; // Mathematical function composition.
  auto as = fill_with<A>(test::vector_fn);
  auto bs = fill_with<B>(test::list_fn);
  auto cs = fill_with<C>(test::set_fn);
  auto f = str;
  auto g = sum_abc;
  {
    as.erase(begin(as), std::next(begin(as), 2));
    auto F = test_functor(as, bs, cs);
    EXPECT_TRUE(F(f * g) == F(f) * F(g));
  }
  as = fill_with<A>(test::vector_fn);
  {
    bs.erase(begin(bs), std::next(begin(bs), 3));
    auto F = test_functor(as, bs, cs);
    EXPECT_TRUE(F(f * g) == F(f) * F(g));
  }
  bs = fill_with<B>(test::list_fn);
  {
    cs.erase(begin(cs), std::next(begin(cs), 1));
    auto F = test_functor(as, bs, cs);
    EXPECT_TRUE(F(f * g) == F(f) * F(g));
  }
}

// Example of empty case (Functor).
TEST(Functor, VectorEmpty) {
  using namespace std;
  EXPECT_EQ(vector<std::string>{}, ka::fmap(str, vector<A>{}));
}

namespace {
  template<typename T>
  struct X {
    T value;
  // Regular:
    KA_GENERATE_FRIEND_REGULAR_OPS_1(X, value);
  // Functor:
    template<typename F>
    auto fmap(F f) const -> X<decltype(f(value))> {
      return {f(value)};
    }
  };
} // namespace

TEST(Functor, MemberFmap) {
  using namespace ka;
  EXPECT_EQ(X<std::string>{"34"}, fmap(str, X<int>{34}));
}

// Separate namespace to test ADL.
namespace test_functor_ns {
  template<typename T>
  struct Y {
    T value;
  // Regular:
    KA_GENERATE_FRIEND_REGULAR_OPS_1(Y, value)
  };
  template<typename F, typename T>
  auto fmap(F f, Y<T> y) -> Y<decltype(f(y.value))> {
    return {f(y.value)};
  }
} // namespace test_functor_ns

TEST(Functor, AdlFmap) {
  using test_functor_ns::Y;
  int const i = 34;
  EXPECT_EQ(Y<D>{D{i}}, ka::fmap(to_d, Y<A>{A{i}}));
}

TEST(Functor, PolymorphicFmap) {
  using boost::optional;
  auto to_d_lift = ka::fmap(to_d);
  ka::test::list_fn_t list;
  ka::test::map_fn_t map;
  int const i = 5;
  EXPECT_EQ(list(D{i}, D{i+1}), to_d_lift(list(A{i}, A{i+1})));

  EXPECT_EQ(map(kv(B{i}, D{i}), kv(B{i+1}, D{i+1})), to_d_lift(
            map(kv(B{i}, A{i}), kv(B{i+1}, A{i+1}))));

  EXPECT_EQ(optional<D>{i}, to_d_lift(optional<A>{A{i}}));
}
