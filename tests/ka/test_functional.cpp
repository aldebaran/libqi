#include <atomic>
#include <cmath>
#include <cstdlib>
#include <future>
#include <limits>
#include <list>
#include <memory>
#include <sstream>
#include <string>
#include <vector>
#include <boost/optional.hpp>
#include <boost/thread/shared_mutex.hpp>
#include <gtest/gtest.h>
#include "test_functional_common.hpp"
#include <ka/algorithm.hpp>
#include <ka/conceptpredicate.hpp>
#include <ka/functional.hpp>
#include <ka/memory.hpp>
#include <ka/mutablestore.hpp>
#include <ka/mutex.hpp>
#include <ka/range.hpp>
#include <ka/testutils.hpp>
#include <ka/typetraits.hpp>
#include <ka/utility.hpp>

TEST(FunctionalPolymorphicConstantFunction, RegularNonVoid) {
  using namespace ka;
  using F = constant_function_t<int>;
  auto const incr = [](F& f) {
    ++f.ret;
  };
  // F is regular because int is.
  ASSERT_TRUE(is_regular(bounded_range(F{0}, F{100}, incr)));
}

TEST(FunctionalPolymorphicConstantFunction, RegularVoid) {
  using namespace ka;
  using F = constant_function_t<void>;
  ASSERT_TRUE(is_regular({F{}}));
}

struct non_regular_t {
  int i;
  friend bool operator==(non_regular_t a, non_regular_t b) {
    return &a == &b;
  }
  friend bool operator<(non_regular_t a, non_regular_t b) {
    return &a < &b;
  }
};

TEST(FunctionalPolymorphicConstantFunction, NonRegularNonVoid) {
  using namespace ka;
  using F = constant_function_t<non_regular_t>;
  auto incr = [](F& f) {
    ++f.ret.i;
  };
  // F is not regular because non_regular_t isn't.
  ASSERT_FALSE(is_regular(bounded_range(F{{0}}, F{{100}}, incr)));
}

TEST(FunctionalPolymorphicConstantFunction, BasicNonVoid) {
  using namespace ka;
  char const c = 'z';
  constant_function_t<unsigned char> f{c};
  ASSERT_EQ(c, f());
  ASSERT_EQ(c, f(1));
  ASSERT_EQ(c, f(2.345));
  ASSERT_EQ(c, f("abcd"));
  ASSERT_EQ(c, f(true));
  ASSERT_EQ(c, f(std::vector<int>{5, 7, 2, 1}));
  ASSERT_EQ(c, f(1, 2.345, "abcd", true));
}

TEST(FunctionalPolymorphicConstantFunction, BasicVoid) {
  using namespace ka;
  constant_function_t<void> f;
  ASSERT_NO_THROW(f());
  ASSERT_NO_THROW(f(1));
  ASSERT_NO_THROW(f(2.345));
  ASSERT_NO_THROW(f("abcd"));
  ASSERT_NO_THROW(f(true));
  ASSERT_NO_THROW(f(std::vector<int>{5, 7, 2, 1}));
  ASSERT_NO_THROW(f(1, 2.345, "abcd", true));
}

TEST(FunctionalPolymorphicConstantFunction, Retraction) {
  using namespace ka;
  constant_function_t<void> f;
  auto g = retract(f);
  ASSERT_EQ(f, g);
  ASSERT_NO_THROW(f());
  ASSERT_NO_THROW(g());
}

namespace {
  template<typename F>
  void testIdTransfo() {
    F id{};
    ASSERT_EQ(id(1), 1);
    ASSERT_EQ(id(true), true);
    ASSERT_EQ(id('a'), 'a');
    ASSERT_EQ(id(nullptr), nullptr);
    ASSERT_EQ(id(std::vector<int>(10, 'z')), std::vector<int>(10, 'z'));
  }
} // namespace

TEST(FunctionalIdTransfo, Basic) {
  testIdTransfo<ka::id_transfo_t>();
  testIdTransfo<ka::id_transfo_t const>();
}

namespace {
  template<typename F>
  void testIdAction() {
    F id{};
    {
      auto x = 1;
      id(x);
      ASSERT_EQ(x, 1);
    }
    {
      auto x = true;
      id(x);
      ASSERT_EQ(x, true);
    }
    {
      auto x = 'a';
      id(x);
      ASSERT_EQ(x, 'a');
    }
    {
      auto x = nullptr;
      id(x);
      ASSERT_EQ(x, nullptr);
    }
    {
      auto x = std::vector<int>(10, 'z');
      id(x);
      ASSERT_EQ(x, std::vector<int>(10, 'z'));
    }
  }
} // namespace

TEST(FunctionalIdAction, Basic) {
  testIdAction<ka::id_action_t>();
  testIdAction<ka::id_action_t const>();
}

namespace {
  // For use to test `is_regular` only.
  // The returned strings are irrelevant, the only point is that these functions
  // are regular.
  std::string strbool0(bool x) {
    return x ? "test test" : "1, 2, 1, 2";
  }

  std::string strbool1(bool x) {
    return x ? "mic mic" : "Vous etes chauds ce soir?!";
  }
} // namespace

TEST(FunctionalCompose, Regular) {
  using namespace ka;
  using namespace std;
  using C = composition_t<string (*)(bool), bool (*)(float)>;
  ASSERT_TRUE(is_regular({
    C{strbool0, isnan}, C{strbool0, isfinite}, C{strbool1, isinf}
  }));
}

namespace {
  template<typename F>
  void testComposeNonVoid(F& half_greater_1) {
    static_assert(ka::Equal<bool, decltype(half_greater_1(3))>::value, "");
    ASSERT_TRUE(half_greater_1(3));
    ASSERT_FALSE(half_greater_1(1));
  }
} // namespace

TEST(FunctionalCompose, NonVoid) {
  using namespace ka;

  auto half = [](int x) {
    return x / 2.f;
  };
  auto greater_1 = [](float x) {
    return x > 1.f;
  };
  {
    auto half_greater_1 = compose(greater_1, half);
    testComposeNonVoid(half_greater_1);
  }
  {
    auto const half_greater_1 = compose(greater_1, half);
    testComposeNonVoid(half_greater_1);
  }
}

namespace {
  template<typename F>
  void testComposeVoid(F& k, int uninitialized, int& fOrder, int& gOrder) {
    static_assert(ka::Equal<void, decltype(k(3))>::value, "");

    ASSERT_EQ(uninitialized, fOrder);
    ASSERT_EQ(uninitialized, gOrder);
    k(3);
    ASSERT_EQ(0, fOrder);
    ASSERT_EQ(1, gOrder);
  }
} // namespace

TEST(FunctionalCompose, Void) {
  using namespace ka;
  int const uninitialized = std::numeric_limits<int>::max();
  {
    int order = 0;
    int fOrder = uninitialized;
    int gOrder = uninitialized;
    auto f = [&](int) {
      fOrder = order++;
    };
    auto g = [&] {
      gOrder = order++;
    };
    auto k = compose(g, f);
    testComposeVoid(k, uninitialized, fOrder, gOrder);
  }
  {
    int order = 0;
    int fOrder = uninitialized;
    int gOrder = uninitialized;
    auto f = [&](int) {
      fOrder = order++;
    };
    auto g = [&] {
      gOrder = order++;
    };
    auto const k = compose(g, f);
    testComposeVoid(k, uninitialized, fOrder, gOrder);
  }
}

namespace {
  template<typename F>
  void testComposeMulti(F& f) {
    static_assert(ka::Equal<std::string, decltype(f(3))>::value, "");

    ASSERT_EQ("true", f(3));
    ASSERT_EQ("false", f(1));
  }
} // namespace

TEST(FunctionalCompose, Multi) {
  using namespace ka;
  using std::string;

  auto half = [](int x) {
    return x / 2.f;
  };
  auto greater_1 = [](float x) {
    return x > 1.f;
  };
  auto str = [](bool x) -> string {
    return x ? "true" : "false";
  };

  {
    auto f = compose(str, compose(greater_1, half));
    testComposeMulti(f);
  }
  {
    auto const f = compose(str, compose(greater_1, half));
    testComposeMulti(f);
  }
}

TEST(FunctionalCompose, Retraction) {
  using namespace ka;
  using namespace test_functional;
  // We compose a function and its retraction and expect to get the identity
  // function.
  f_t f;
  auto g = retract(f);
  auto gf = compose(g, f);
  ASSERT_EQ(e0_t::a, gf(e0_t::a));
  ASSERT_EQ(e0_t::b, gf(e0_t::b));

// TODO: Remove this define (but keep the content) when get rid of VS2013.
#if !KA_COMPILER_VS2013_OR_BELOW
  static_assert(Equal<decltype(gf), id_transfo_t>::value, "");
#endif
}

TEST(FunctionalCompose, SeemsRetractionButNotQuite) {
  using namespace ka;
  using namespace test_functional;
  // Even if the two functions have retractions (even, are isomorphisms), and
  // can be composed (right domain and codomain), `g_inv_t` is _not_ a retraction
  // for `F`.
  // We expect to _not_ get the identity function.
  auto ginv_f = compose(g_inv_t{}, f_t{});
  ASSERT_EQ(e0_t::b, ginv_f(e0_t::a));
  ASSERT_EQ(e0_t::a, ginv_f(e0_t::b));
  static_assert(!Equal<decltype(ginv_f), id_transfo_t>::value, "");
}

namespace {
  struct plus_t {
    float a;
    KA_GENERATE_FRIEND_REGULAR_OPS_1(plus_t, a)
    float operator()(float b) const {
      return a + b;
    }
    friend plus_t retract(plus_t const& x) {
      return {-x.a};
    }
  };
  struct times_t {
    float a;
    KA_GENERATE_FRIEND_REGULAR_OPS_1(times_t, a)
    float operator()(int b) const {
      return a * b;
    }
    friend times_t retract(times_t const& x) {
      return {1.f / x.a};
    }
  };
}

TEST(FunctionalCompose, RetractionOfComposition) {
  using namespace ka;

  auto incr = plus_t{1};
  auto twice = times_t{2};

  ASSERT_EQ(3, compose(incr, twice)(1)); // (1*2)+1
  ASSERT_EQ(11, compose(incr, twice)(5)); // (5*2)+1

  ASSERT_EQ(1, retract(compose(incr, twice))(3)); // (3-1)/2
  ASSERT_EQ(5, retract(compose(incr, twice))(11)); // (11-1)/2
}

TEST(FunctionalCompose, Identity) {
  using namespace ka;
  using namespace ka::functional_ops;
  using namespace test_functional;
  f_t f;
  id_transfo_t _1;
  static_assert(Equal<Decay<decltype(_1 * _1)>, decltype(_1)>::value, "");
  static_assert(Equal<Decay<decltype(f * _1)>, decltype(f)>::value, "");
  static_assert(Equal<Decay<decltype(f * _1 * _1)>, decltype(f)>::value, "");
  static_assert(Equal<Decay<decltype(_1 * f)>, decltype(f)>::value, "");
  static_assert(Equal<Decay<decltype(_1 * f * _1)>, decltype(f)>::value, "");

// TODO: Remove this define (but keep the content) when get rid of VS2013.
#if KA_COMPILER_VS2013_OR_BELOW
  // To avoid "unreferenced local variable" warnings.
  f(e0_t::a);
  _1(e0_t::a);
#endif
}

TEST(FunctionalCompose, Simplification) {
// TODO: Remove this define (but keep the content) when get rid of VS2013.
// VS2013 compiler does not allow to do composition simplifications.
#if !KA_COMPILER_VS2013_OR_BELOW
  using namespace ka;
  using namespace ka::functional_ops;
  using namespace test_functional;
  // We expect chains of composition to be simplified in the right way.
  f_t f;
  auto g = retract(f);
  auto z = g * f * g * f * g * f * g * f;
  static_assert(Equal<decltype(z), id_transfo_t>::value, "");
  static_assert(Equal<Decay<decltype(z * g)>, decltype(g)>::value, "");
#endif
}

TEST(FunctionalCompose, Associative) {
  using namespace ka::functional_ops;
  using std::string;

  auto f = [](int x) {
    return x / 2.f;
  };
  auto g = [](float x) {
    return x > 1.f;
  };
  auto h = [](bool x) -> string {
    return x ? "true" : "false";
  };
  auto i = [](string const& x) -> std::size_t {
    return x.size();
  };

  auto a = ((i * h) * g) * f;
  auto b = (i * (h * g)) * f;
  auto c = i * (h * (g * f));
  auto d = (i * h) * (g * f);
  auto e = i * ((h * g) * f);

  ASSERT_EQ(a(3), b(3));
  ASSERT_EQ(b(3), c(3));
  ASSERT_EQ(c(3), d(3));
  ASSERT_EQ(d(3), e(3));

  ASSERT_EQ(a(0), b(0));
  ASSERT_EQ(b(0), c(0));
  ASSERT_EQ(c(0), d(0));
  ASSERT_EQ(d(0), e(0));
}

TEST(FunctionalCompose, Id) {
  using namespace ka;
  using namespace ka::functional_ops;
  using std::string;

  auto f = [](int x) {
    return x / 2.f;
  };
  auto g = [](float x) {
    return x > 1.f;
  };
  id_transfo_t _1;

  auto f0 = f * _1;
  auto f1 = _1 * f;
  auto gf0 = (g * f) * _1;
  auto gf1 = _1 * (g * f);

  ASSERT_EQ(f0(3), f1(3));
  ASSERT_EQ(gf0(3), gf1(3));

  ASSERT_EQ(f0(0), f1(0));
  ASSERT_EQ(gf0(0), gf1(0));
}

TEST(FunctionalCompose, OperatorPipe) {
  using namespace ka;
  using ka::functional_ops::operator|;
  using std::string;

  auto half = [](int x) {
    return x / 2.f;
  };
  auto greater_1 = [](float x) {
    return x > 1.f;
  };
  auto str = [](bool x) -> string {
    return x ? "true" : "false";
  };

  {
    auto f = half | greater_1 | str;
    testComposeMulti(f);
  }
  {
    auto const f = half | greater_1 | str;
    testComposeMulti(f);
  }
}

namespace {
  void remove_n(std::string& s, char c, int n) {
    ka::erase_if(s, [&](char x) {return x == c && --n >= 0;});
  }

  void concat(std::string& s, char c, int n) {
    // Precondition: n >= 0
    s.insert(s.end(), n, c);
  }

  void noop(std::string&, char, int) {
  }
}

TEST(FunctionalComposeAccu, Regular) {
  using namespace ka;
  using A = void (*)(std::string&, char, int);
  using C = composition_accu_t<A, A>;
  ASSERT_TRUE(is_regular({
    C{remove_n, concat}, C{concat, remove_n}, C{remove_n, remove_n}, C{concat, noop}
  }));
}

namespace {
  template<typename F>
  void testComposeAccuMulti(F& f) {
    {
      float i = -3.f;
      f(i);
      ASSERT_EQ(1.f, i);
    } {
      float i = 1.f;
      f(i);
      ASSERT_EQ(0.5f, i);
    }
  }
} // namespace

TEST(FunctionalComposeAccu, Multi) {
  using namespace ka;

  auto half = [](float& x) {
    x /= 2.f;
  };
  auto clamp = [](float& x) {
    if (x > 1.f) x = 1.f;
    if (x < -1.f) x = -1.f;
  };
  auto abs = [](float& x) {
    if (x < 0.f) x = -x;
  };

  {
    auto f = compose_accu(abs, compose_accu(clamp, half));
    testComposeAccuMulti(f);
  }
  {
    auto const f = compose_accu(abs, compose_accu(clamp, half));
    testComposeAccuMulti(f);
  }
}

TEST(FunctionalComposeAccu, Retraction) {
  using namespace ka;
  using namespace test_functional;
  // We compose an action and its retraction and expect to get the identity
  // action.
  a_t f;
  auto g = retract(f);
  auto gf = compose_accu(g, f);
  {
    e0_t e = e0_t::a;
    gf(e);
    ASSERT_EQ(e0_t::a, e);
  } {
    e0_t e = e0_t::b;
    gf(e);
    ASSERT_EQ(e0_t::b, e);
  }
  static_assert(Equal<decltype(gf), id_action_t>::value, "");
}

TEST(FunctionalComposeAccu, ComposeAccu) {
  using namespace ka;
  {
    auto a = compose_accu(remove_n, concat);
    std::string s{"youpi les amis"};
    a(s, ' ', 2);
    ASSERT_EQ(std::string{"youpilesamis  "}, s);
  } {
    auto const a = compose_accu(concat, remove_n);
    std::string s{"youpi les amis"};
    a(s, ' ', 4);
    ASSERT_EQ(std::string{"youpilesamis    "}, s);
  } {
    auto a = compose_accu(concat, noop);
    std::string s{"youpi les amis"};
    a(s, '!', 3);
    ASSERT_EQ(std::string{"youpi les amis!!!"}, s);
  }
}

namespace {
  void drop3(std::string& s) {
    s.erase(0, 3);
  }

  void twice(std::string& s) {
    s += s;
  }
}

TEST(FunctionalComposeAccu, ComposeAction) {
  using namespace ka;
  {
    auto a = compose_accu(drop3, twice);
    std::string s{"youpi"};
    a(s);
    ASSERT_EQ(std::string{"piyoupi"}, s);
  } {
    auto const a = compose_accu(twice, drop3);
    std::string s{"youpi"};
    a(s);
    ASSERT_EQ(std::string{"pipi"}, s);
  }
}

namespace {
  struct plus_accu_t {
    float a;
    KA_GENERATE_FRIEND_REGULAR_OPS_1(plus_accu_t, a)
    void operator()(float& b) const {
      b += a;
    }
    friend plus_accu_t retract(plus_accu_t const& x) {
      return {-x.a};
    }
  };
  struct times_accu_t {
    float a;
    KA_GENERATE_FRIEND_REGULAR_OPS_1(times_accu_t, a)
    void operator()(float& b) const {
      b *= a;
    }
    friend times_accu_t retract(times_accu_t const& x) {
      return {1.f / x.a};
    }
  };
}

TEST(FunctionalComposeAccu, RetractionOfComposition) {
  using namespace ka;

  auto incr = plus_accu_t{1.f};
  auto twice = times_accu_t{2.f};
  auto f = compose_accu(incr, twice);

  float a = 0.f;
  a = 1.f;
  f(a); // (1*2)+1
  ASSERT_EQ(3, a);

  a = 5.f;
  f(a); // (5*2)+1
  ASSERT_EQ(11, a);

  a = 3.f;
  retract(f)(a); // (3-1)/2
  ASSERT_EQ(1, a);

  a = 11.f;
  retract(f)(a); // (11-1)/2
  ASSERT_EQ(5, a);
}

TEST(FunctionalComposeAccu, Identity) {
  using namespace ka;
  using ka::functional_ops_accu::operator*;
  using namespace test_functional;
  a_t f;
  id_action_t _1;
  static_assert(Equal<Decay<decltype(_1 * _1)>, decltype(_1)>::value, "");
  static_assert(Equal<Decay<decltype(f * _1)>, decltype(f)>::value, "");
  static_assert(Equal<Decay<decltype(f * _1 * _1)>, decltype(f)>::value, "");
  static_assert(Equal<Decay<decltype(_1 * f)>, decltype(f)>::value, "");
  static_assert(Equal<Decay<decltype(_1 * f * _1)>, decltype(f)>::value, "");

// TODO: Remove this define (but keep the content) when get rid of VS2013.
#if KA_COMPILER_VS2013_OR_BELOW
  // To avoid "unreferenced local variable" warnings.
  e0_t e = e0_t::a;
  f(e);
  _1(e);
#endif
}

TEST(FunctionalComposeAccu, OperatorPipe) {
  using namespace ka;
  using ka::functional_ops_accu::operator|;

  auto half = [](float& x) {
    x /= 2.f;
  };
  auto clamp = [](float& x) {
    if (x > 1.f) x = 1.f;
    if (x < -1.f) x = -1.f;
  };
  auto abs = [](float& x) {
    if (x < 0.f) x = -x;
  };

  {
    auto f = half | clamp | abs;
    testComposeAccuMulti(f);
  }
  {
    auto const f = half | clamp | abs;
    testComposeAccuMulti(f);
  }
}

TEST(FunctionalComposeAccu, Simplification) {
  using namespace ka;
  using ka::functional_ops_accu::operator*;
  using namespace test_functional;
  // We expect chains of composition to be simplified in the right way.
  a_t f;
  auto g = retract(f);
  auto z = g * f * g * f * g * f * g * f;
  static_assert(Equal<decltype(z), id_action_t>::value, "");
  static_assert(Equal<Decay<decltype(z * g)>, decltype(g)>::value, "");
}

TEST(FunctionalComposeAccu, Associative) {
  using ka::functional_ops_accu::operator*;
  using std::string;

  auto f = [](float& x) {
    x /= 2.f;
  };
  auto g = [](float& x) {
    x = -x;
  };
  auto h = [](float& x) {
    x += x;
  };
  auto i = [](float& x) {
    x -= 1;
  };

  auto a = (((i * h) * g) * f);
  auto b = ((i * (h * g)) * f);
  auto c = (i * (h * (g * f)));
  auto d = ((i * h) * (g * f));
  auto e = (i * ((h * g) * f));

  {
    float i = 3;
    float j = 3;
    a(i);
    b(j);
    ASSERT_EQ(i, j);
  } {
    float i = 3;
    float j = 3;
    b(i);
    c(j);
    ASSERT_EQ(i, j);
  } {
    float i = 3;
    float j = 3;
    c(i);
    d(j);
    ASSERT_EQ(i, j);
  } {
    float i = 3;
    float j = 3;
    d(i);
    e(j);
    ASSERT_EQ(i, j);
  } {
    float i = 0;
    float j = 0;
    a(i);
    b(j);
    ASSERT_EQ(i, j);
  } {
    float i = 0;
    float j = 0;
    b(i);
    c(j);
    ASSERT_EQ(i, j);
  } {
    float i = 0;
    float j = 0;
    c(i);
    d(j);
    ASSERT_EQ(i, j);
  } {
    float i = 0;
    float j = 0;
    d(i);
    e(j);
    ASSERT_EQ(i, j);
  }
}

TEST(FunctionalComposeAccu, Id) {
  using namespace ka;
  using ka::functional_ops_accu::operator*;
  using std::string;

  auto f = [](float& x) {
    x /= 2.f;
  };
  auto g = [](float& x) {
    x = -x;
  };
  id_action_t _1;

  auto f0 = (f * _1);
  auto f1 = (_1 * f);
  auto gf0 = ((g * f) * _1);
  auto gf1 = (_1 * (g * f));

  {
    float i = 3;
    float j = 3;
    f0(i);
    f1(j);
    ASSERT_EQ(i, j);
  } {
    float i = 3;
    float j = 3;
    gf0(i);
    gf1(j);
    ASSERT_EQ(i, j);
  } {
    float i = 0;
    float j = 0;
    f0(i);
    f1(j);
    ASSERT_EQ(i, j);
  } {
    float i = 0;
    float j = 0;
    gf0(i);
    gf1(j);
    ASSERT_EQ(i, j);
  }
}

namespace {
  struct x_t {
    x_t() = default;
    explicit x_t(bool b) : b{ b } {}
    bool b{ false };
    bool operator==(x_t x) const {return b == x.b;}
  };

  template<typename T>
  struct constant_unit_t {
    T operator()() const {
      return T{};
    }
  };

  template<typename T>
  bool equal(T const& a, T const& b) {
    return a == b;
  }
}

using types = testing::Types<
  x_t, boost::optional<bool>, std::list<bool>
>;

template<typename T>
struct FunctionalSemiLift0 : testing::Test
{
};

TYPED_TEST_CASE(FunctionalSemiLift0, types);

TYPED_TEST(FunctionalSemiLift0, NonVoidCodomain) {
  using namespace ka;
  using T = TypeParam;

  auto positive = [](int i) {
    return i > 0;
  };
  auto unit = [](bool b) {
    return T{b};
  };
  auto f = semilift(positive, unit);

  static_assert(Equal<T, decltype(f(0))>::value, "");
  ASSERT_TRUE(equal(T{true}, f(1)));
  ASSERT_TRUE(equal(T{false}, f(-1)));
}

using void_types = testing::Types<
  x_t, boost::optional<bool>, std::list<bool>
>;

template<typename T>
struct FunctionalSemiLift1 : testing::Test {
};

TYPED_TEST_CASE(FunctionalSemiLift1, void_types);

TYPED_TEST(FunctionalSemiLift1, VoidCodomain) {
  using namespace ka;
  using T = TypeParam;

  auto noop = [](int) {
  };
  constant_unit_t<T> unit;
  auto f = semilift(noop, unit);

  static_assert(Equal<T, decltype(f(0))>::value, "");
  ASSERT_TRUE(equal(unit(), f(0)));
}

TYPED_TEST(FunctionalSemiLift1, VoidCodomainVoidDomain) {
  using namespace ka;
  using T = TypeParam;

  auto noop = [] {
  };
  constant_unit_t<T> unit;
  auto f = semilift(noop, unit);

  static_assert(Equal<T, decltype(f())>::value, "");
  ASSERT_TRUE(equal(unit(), f()));
}

template<typename T>
struct FunctionalDataBoundProc : testing::Test {
};

namespace {
  // The two following types (`bound_proc_t` and `bound_transfo_t`) are
  // semantically equivalent: they are polymorphic functions that bind a data to
  // a procedure. The returned procedure behaves in the same way as the input
  // one.

  // Implemented with `data_bound_proc`.
  struct bound_proc_t {
    template<typename Proc, typename T>
    auto operator()(Proc const& proc, T const& data) const -> decltype(ka::data_bound_proc(proc, data)) {
      return ka::data_bound_proc(proc, data);
    }
  };

  // Implemented with `data_bound_transfo`.
  struct bound_transfo_t {
    template<typename Proc, typename T>
    auto operator()(Proc const& proc, T const& data) const -> decltype(ka::data_bound_transfo(data)(proc)) {
      return ka::data_bound_transfo(data)(proc);
    }
  };
} // namespace

using bound_proc_types = testing::Types<
  bound_proc_t,
  bound_proc_t const,
  bound_transfo_t,
  bound_transfo_t const>;

// We perform all tests with `ka::data_bound_proc` and `ka::data_bound_transfo`.
TYPED_TEST_CASE(FunctionalDataBoundProc, bound_proc_types);

namespace {
  // Returns the next element.
  struct succ_t {
    template<typename T>
    T operator()(T t) const {
      ++t;
      return t;
    }
  };
}

// Tests that a data-bound procedure acts in the same way as the original
// procedure.
TYPED_TEST(FunctionalDataBoundProc, Basic) {
  TypeParam data_bound{};
  succ_t f0;
  auto f1 = data_bound(f0, 'a');
  for (int i = 0; i != 100; ++i) {
    ASSERT_EQ(f0(i), f1(i));
  }
}

namespace {
  // Has a lifetime dependency on another data.
  struct add_t {
    int const* i; //< here

    template<typename T>
    auto operator()(T const& t) const -> decltype(t + (*i)) {
      return t + (*i);
    }
  };

  // Produces a task that has a lifetime dependency on the worker.
  template<typename F>
  struct worker_t : std::enable_shared_from_this<worker_t<F>> {
    int i;
    F data_bound;

    explicit worker_t(int i, F f) : i(i), data_bound(f) {
    }

    ~worker_t() {
      i = 0xDEADBEEF;
    }

    auto make_task() const -> decltype(data_bound(add_t{&i}, std::shared_ptr<worker_t const>{})) {
      return data_bound(add_t{&i}, this->shared_from_this()); //< here

      // We bind a shared pointer to the returned task to extend the worker's
      // lifetime, and ensure task correctness.
      // This can also be done by capturing the shared pointer in a lambda, but
      // only if the task is not polymorphic (this is a C++11 limitation).
      // We use a `data_bound_proc` to overcome this issue.
    }
  };
} // namespace

// Uses a data-bound procedure to extend the lifetime of an object.
TYPED_TEST(FunctionalDataBoundProc, ExtendLifetime) {
  using namespace ka;
  using F = TypeParam;
  F data_bound{};
  int const i = 5;
  data_bound_proc_t<add_t, std::shared_ptr<worker_t<F> const>> add0;
  {
    auto w = std::make_shared<worker_t<F>>(i, data_bound);
    add0 = w->make_task();
  }
  // Here, the worker is out of scoped but we nonetheless want to use the
  // produced task.

  add_t add1{&i};
  {
    int const j = 3;
    ASSERT_EQ(add0(j), i + j);
    ASSERT_EQ(add1(j), i + j);
  }
  {
    double const j = 3.12;
    ASSERT_EQ(add0(j), i + j);
    ASSERT_EQ(add1(j), i + j);
  }
}

TEST(FunctionalMoveAssign, Basic) {
  using namespace ka;
  using M = move_only_t<int>;
  int const i = 3;
  M original{i};
  move_assign_t<M, M> move_assign{std::move(original)};
  M x{i + 1};
  move_assign(x); // x = std::move(original);
  ASSERT_EQ(i, *x);
}

TEST(FunctionalIncr, Regular) {
  using namespace ka;
  incr_t incr;
  ASSERT_TRUE(is_regular({incr})); // only one possible value because no state
}

TEST(FunctionalIncr, Arithmetic) {
  using namespace ka;
  {
    incr_t incr;
    int x = 0;
    incr(x);
    ASSERT_EQ(1, x);
  } {
    incr_t incr;
    double x = 0.0;
    incr(x);
    ASSERT_EQ(1.0, x);
  }
}

TEST(FunctionalIncr, InputIterator) {
  using namespace ka;
  using namespace std;
  stringstream ss{"youpi les amis"};
  using I = istream_iterator<string>;
  I b(ss);
  incr_t incr;
  ASSERT_EQ("youpi", *b);
  incr(b);
  ASSERT_EQ("les", *b);
  incr(b);
  ASSERT_EQ("amis", *b);
}

TEST(FunctionalDecr, Regular) {
  using namespace ka;
  decr_t decr;
  ASSERT_TRUE(is_regular({decr})); // only one possible value because no state
}

TEST(FunctionalDecr, Arithmetic) {
  using namespace ka;
  {
    decr_t decr;
    int x = 1;
    decr(x);
    ASSERT_EQ(0, x);
  } {
    decr_t decr;
    double x = 1.0;
    decr(x);
    ASSERT_EQ(0.0, x);
  }
}

TEST(FunctionalDecr, BidirectionalIterator) {
  using namespace ka;
  using namespace std;
  decr_t decr;
  list<string> l{"youpi", "les", "amis"};
  auto b = end(l);
  decr(b);
  ASSERT_EQ("amis", *b);
  decr(b);
  ASSERT_EQ("les", *b);
  decr(b);
  ASSERT_EQ("youpi", *b);
}

TEST(FunctionalIncr, IsomorphicIntegral) {
  using namespace ka;
  {
    incr_t incr;
    auto inv = retract(incr);
    int i = 0;
    incr(i);
    inv(i);
    ASSERT_EQ(0, i);
  } {
    incr_t incr;
    auto inv = retract(incr);
    int i = 0;
    inv(i);
    incr(i);
    ASSERT_EQ(0, i);
  }
}

TEST(FunctionalIncr, IsomorphicBidirectionalIterator) {
  using namespace ka;
  using namespace std;
  incr_t incr;
  auto inv = retract(incr);
  list<string> l{"youpi", "les", "amis"};
  auto b = begin(l);
  ++b;
  incr(b);
  inv(b);
  ASSERT_EQ("les", *b);
  inv(b);
  incr(b);
  ASSERT_EQ("les", *b);
}

TEST(FunctionalDecr, IsomorphicIntegral) {
  using namespace ka;
  {
    decr_t decr;
    auto inv = retract(decr);
    int i = 0;
    decr(i);
    inv(i);
    ASSERT_EQ(0, i);
  } {
    decr_t decr;
    auto inv = retract(decr);
    int i = 0;
    inv(i);
    decr(i);
    ASSERT_EQ(0, i);
  }
}

TEST(FunctionalDecr, IsomorphicBidirectionalIterator) {
  using namespace ka;
  using namespace std;
  decr_t decr;
  auto inv = retract(decr);
  list<string> l{"youpi", "les", "amis"};
  auto b = begin(l);
  ++b;
  decr(b);
  inv(b);
  ASSERT_EQ("les", *b);
  inv(b);
  decr(b);
  ASSERT_EQ("les", *b);
}

TEST(FunctionalApply, Tuple) {
  using namespace ka;
  auto g = [](int i, char c, float f) {
    return std::make_tuple(i, c, f);
  };
  auto const args = std::make_tuple(5, 'a', 3.14f);
  ASSERT_EQ(args, apply(g, args));
  ASSERT_EQ(args, apply(g)(args));
  auto const h = apply(g);
  ASSERT_EQ(args, h(args));
}

TEST(FunctionalApply, Pair) {
  using namespace ka;
  auto g = [](int i, char c) {
    return std::make_pair(i, c);
  };
  auto const args = std::make_pair(5, 'a');
  ASSERT_EQ(args, apply(g, args));
  ASSERT_EQ(args, apply(g)(args));
  auto const h = apply(g);
  ASSERT_EQ(args, h(args));
}

TEST(FunctionalApply, Array) {
  using namespace ka;
  auto g = [](int i, int j, int k, int l) {
    return std::array<int, 4>{{i, j, k, l}};
  };
  std::array<int, 4> const args {{0, 1, 2, 3}};
  ASSERT_EQ(args, apply(g, args));
  ASSERT_EQ(args, apply(g)(args));
  auto const h = apply(g);
  ASSERT_EQ(args, h(args));
}

TEST(FunctionalApply, Custom) {
  using namespace ka;
  using X = test_functional::x_t<int, char, float>;
  auto g = [](int i, char c, float f) {
    return X{i, c, f};
  };
  X const args{5, 'a', 3.14f};
  ASSERT_EQ(args, apply(g, args));
  ASSERT_EQ(args, apply(g)(args));
  auto const h = apply(g);
  ASSERT_EQ(args, h(args));
}

TEST(FunctionalApply, MoveOnly) {
  using namespace ka;
  auto g = [](move_only_t<int> i, move_only_t<char> c, move_only_t<float> f) {
    return std::make_tuple(*i, *c, *f);
  };
  auto const res = std::make_tuple(5, 'a', 3.14f);
  {
    auto args = std::make_tuple(move_only_t<int>{5}, move_only_t<char>{'a'}, move_only_t<float>{3.14f});
    ASSERT_EQ(res, apply(g, std::move(args)));
  } {
    auto args = std::make_tuple(move_only_t<int>{5}, move_only_t<char>{'a'}, move_only_t<float>{3.14f});
    ASSERT_EQ(res, apply(g)(std::move(args)));
  } {
    auto args = std::make_tuple(move_only_t<int>{5}, move_only_t<char>{'a'}, move_only_t<float>{3.14f});
    auto const h = apply(g);
    ASSERT_EQ(res, h(std::move(args)));
  }
}

TEST(FunctionalPolyIncr, Regular) {
  using namespace ka;
  ASSERT_TRUE(is_regular({incr_t{}})); // only one possible value because no state
}

TEST(FunctionalPolyIncr, Basic) {
  using namespace ka;
  incr_t incr;
  {
    int i = 0;
    incr(i);
    ASSERT_EQ(1, i);
  } {
    std::vector<int> v{1};
    auto b = begin(v);
    incr(b);
    ASSERT_EQ(end(v), b);
  }
}

TEST(FunctionalPolyDecr, Regular) {
  using namespace ka;
  ASSERT_TRUE(is_regular({decr_t{}})); // only one possible value because no state
}

TEST(FunctionalPolyDecr, Basic) {
  using namespace ka;
  decr_t decr;
  {
    int i = 1;
    decr(i);
    ASSERT_EQ(0, i);
  } {
    std::vector<int> v{1, 2};
    auto b = begin(v) + 1;
    decr(b);
    ASSERT_EQ(begin(v), b);
  }
}

TEST(FunctionalPolyIncr, Isomorphic) {
  using namespace ka;
  {
    incr_t incr;
    auto decr = retract(incr);
    int i = 0;
    incr(i);
    decr(i);
    ASSERT_EQ(0, i);
  } {
    decr_t decr;
    auto incr = retract(decr);
    int i = 0;
    decr(i);
    incr(i);
    ASSERT_EQ(0, i);
  }
}

TEST(FunctionalPolyIncr, Composition) {
  using namespace ka;
  using ka::functional_ops_accu::operator*;
  {
    incr_t incr;
    auto incr_twice = (incr * incr);
    int i = 0;
    incr_twice(i);
    ASSERT_EQ(2, i);
  } {
    incr_t incr;
    auto decr = retract(incr);
    auto id = (incr * decr * decr * incr);
    static_assert(Equal<decltype(id), id_action_t>::value, "");
    int i = 0;
    id(i);
    ASSERT_EQ(0, i);
  }
}

struct trivial_scope_lockable_t {
  bool success;
  friend bool scopelock(trivial_scope_lockable_t& x) { return x.success; }
};

struct strict_scope_lockable_t {
  struct lock_t {
    ka::move_only_t<bool*> locked;

    lock_t(bool* l) : locked{ l } { **locked = true; }
    ~lock_t() { **locked = false; }

    lock_t(lock_t&& o) : locked{std::move(o.locked)} {}
    lock_t& operator=(lock_t&& o) { locked = std::move(o.locked); return *this; }

    explicit operator bool() const { return true; }
  };

  friend lock_t scopelock(strict_scope_lockable_t& l) {
    return lock_t{ l.locked };
  }

  bool* locked;
};

TEST(FunctionalScopeLock, ReturnsVoidSuccess) {
  using namespace ka;
  using L = trivial_scope_lockable_t;

  // TODO: pass by value instead of mutable store when source is available
  {
    bool called = false;
    auto proc = scope_lock_proc([&]{ called = true; }, mutable_store(L{ true }));
    proc();
    ASSERT_TRUE(called);
  } {
    bool called = false;
    auto const proc = scope_lock_proc([&]{ called = true; }, mutable_store(L{ true }));
    proc();
    ASSERT_TRUE(called);
  }
}

TEST(FunctionalScopeLock, ReturnsVoidFailure) {
  using namespace ka;
  using L = trivial_scope_lockable_t;

  // TODO: pass by value instead of mutable store when source is available
  {
    bool called = false;
    auto proc = scope_lock_proc([&]{ called = true; }, mutable_store(L{ false }));
    proc();
    ASSERT_FALSE(called);
  } {
    bool called = false;
    auto const proc = scope_lock_proc([&]{ called = true; }, mutable_store(L{ false }));
    proc();
    ASSERT_FALSE(called);
  }
}

TEST(FunctionalScopeLock, ReturnsProcResultOnLockSuccess) {
  using namespace ka;
  using L = trivial_scope_lockable_t;

  // TODO: pass by value instead of mutable store when source is available
  auto proc = scope_lock_proc([](int i){ return i + 10; }, mutable_store(L{ true }));
  auto res = proc(5);
  ASSERT_FALSE(res.empty());
  ASSERT_EQ(15, *res);
}

TEST(FunctionalScopeLock, ReturnsEmptyOptionalOnLockFailure) {
  using namespace ka;
  using L = trivial_scope_lockable_t;

  // TODO: pass by value instead of mutable store when source is available
  auto proc = scope_lock_proc([](int i){ return i + 10; }, mutable_store(L{ false }));
  auto res = proc(12);
  ASSERT_TRUE(res.empty());
}

TEST(FunctionalScopeLock, StaysLockedUntilProcIsFinished) {
  using namespace ka;
  using L = strict_scope_lockable_t;

  bool locked = false;
  // TODO: pass by value instead of mutable store when source is available
  auto proc = scope_lock_proc(
      [&] {
        if (!locked)
          throw std::runtime_error("was not locked");
      },
      mutable_store(L{ &locked }));
  ASSERT_NO_THROW(proc());
  ASSERT_FALSE(locked);
}

using SharedPtrTypes = testing::Types<std::shared_ptr<int>,
                                      boost::shared_ptr<int>>;

template<typename T>
struct FunctionalScopeLockWeakPtr : testing::Test {
};

TYPED_TEST_CASE(FunctionalScopeLockWeakPtr, SharedPtrTypes);

TYPED_TEST(FunctionalScopeLockWeakPtr, SuccessfulLock) {
  using namespace ka;
  using ShPtr = TypeParam;

  ShPtr shptr{ new int{ 42 } };
  auto wkptr = weak_ptr(shptr);
  auto l = scopelock(wkptr);
  ASSERT_TRUE(l);
  ASSERT_EQ(2, l.use_count());
  ASSERT_EQ(shptr.get(), l.get());
}

TYPED_TEST(FunctionalScopeLockWeakPtr, FailureExpired) {
  using namespace ka;
  using ShPtr = TypeParam;

  ShPtr shptr;
  auto wkptr = weak_ptr(shptr);
  auto l = scopelock(wkptr);
  ASSERT_FALSE(l);
}

namespace {
  template<typename M>
  bool is_locked(M& m) {
     return std::async(std::launch::async, [&]{
       return !std::unique_lock<M>{ m, std::try_to_lock };
     }).get();
  }
}

using MutexTypes = testing::Types<std::mutex,
                                  std::recursive_mutex,
#if !BOOST_OS_ANDROID
                                  std::timed_mutex,
                                  std::recursive_timed_mutex,
#endif
                                  boost::mutex,
                                  boost::recursive_mutex,
                                  boost::timed_mutex,
                                  boost::recursive_timed_mutex,
                                  boost::shared_mutex>;

template<typename T>
struct FunctionalScopeLockMutexes : testing::Test {
};

TYPED_TEST_CASE(FunctionalScopeLockMutexes, MutexTypes);

TYPED_TEST(FunctionalScopeLockMutexes, Mutexes) {
  using namespace ka;
  using Mutex = TypeParam;

  Mutex m;
  ASSERT_FALSE(is_locked(m));
  {
    auto l = scopelock(m);
    ASSERT_TRUE(l);
    ASSERT_TRUE(is_locked(m));
  }
  ASSERT_FALSE(is_locked(m));
}
