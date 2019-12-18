#include <type_traits>
#include <gtest/gtest.h>
#include <ka/testutils.hpp>
#include <ka/typetraits.hpp>
#include <ka/utility.hpp>

TEST(UtilityFwd, Basic) {
  using namespace ka;
  int i = 5;
  // In a template context, a "universal reference" `T&&` is deduced as:
  // - `U&` if a lvalue of type U if passed
  // - `U` otherwise (a rvalue of type U is passed)
  static_assert(std::is_lvalue_reference<decltype(fwd<int&>(i))>::value, "");
  static_assert(std::is_rvalue_reference<decltype(fwd<int>(5))>::value, "");
}

namespace ka {
  enum class ref_kind_t {
    l_value,
    r_value
  };

  template<typename T>
  ref_kind_t g(move_only_t<T>&&) {
    return ref_kind_t::r_value;
  }

  template<typename T>
  ref_kind_t g(move_only_t<T>&) {
    return ref_kind_t::l_value;
  }

  template<typename T>
  ref_kind_t f(T&& x) {
    using namespace ka;
    return g(fwd<T>(x));
  }
}

TEST(UtilityFwd, RealWord) {
  using namespace ka;
  move_only_t<int> m{4};
  ASSERT_EQ(ref_kind_t::l_value, f(m));
  ASSERT_EQ(ref_kind_t::r_value, f(move_only_t<int>{4}));
}

TEST(UtilityDeclref, Basic) {
  using namespace ka;
  static_assert(Equal<decltype(declref<int>()), int&>::value, "");
  static_assert(Equal<decltype(declref<int&>()), int&>::value, ""); // Ref collapse
  static_assert(Equal<decltype(declref<int&&>()), int&>::value, ""); // Ref collapse
  static_assert(Equal<decltype(declref<const int>()), const int&>::value, "");
}

namespace test_utility {
  template<typename T>
  T& k(T& t);

  template<typename T, std::size_t N>
  T* k(T (&)[N]);

  template<typename T>
  struct x_t {
    using type = decltype(k(ka::declref<T>()));
  };
} // namespace test_utility

TEST(UtilityDeclref, RealWorld) {
  using namespace ka;
  using namespace test_utility;
  static_assert(Equal<x_t<int>::type, int&>::value, "");
  static_assert(Equal<x_t<int [2]>::type, int*>::value, "");
}

TEST(UtilityDeclcref, Basic) {
  using namespace ka;
  static_assert(Equal<decltype(declcref<int>()), const int&>::value, "");

  // A ref itself can't be const and you can't have a ref on a ref.
  static_assert(Equal<decltype(declcref<int&>()), int&>::value, "");

  // Idem.
  static_assert(Equal<decltype(declcref<int&&>()), int&>::value, "");

  static_assert(Equal<decltype(declcref<const int>()), const int&>::value, "");
}

TEST(UtilityExchange, Basic) {
  using namespace ka;

  int a = 42;
  const auto b = exchange(a, 33);
  static_assert(Equal<decltype(b), const int>::value, "");

  EXPECT_EQ(33, a);
  EXPECT_EQ(42, b);
}

TEST(UtilityExchange, MoveOnly) {
  using namespace ka;

  move_only_t<int> a{ 42 };
  const auto b = exchange(a, move_only_t<int>{ 33 });
  static_assert(Equal<decltype(b), const move_only_t<int>>::value, "");

  EXPECT_EQ(33, *a);
  EXPECT_EQ(42, *b);
}

namespace test_utility {
  int op(ka::type_t<ka::test::A>, int i) {
    return i + 1;
  }
  int op(ka::type_t<ka::test::B>, int i) {
    return i * 2;
  }
}

// Example of `type_t` used as dispatch facility.
TEST(Utility, TypeTDispatch) {
  using namespace ka;
  using namespace ka::test;
  using test_utility::op;
  auto const i = 32;
  EXPECT_EQ(i + 1, op(type_t<A>{}, i));
  EXPECT_EQ(i * 2, op(type_t<B>{}, i));
}

TEST(Utility, TypeTAsProduct) {
  using namespace ka;
  using namespace ka::test;
  using testing::StaticAssertTypeEq;
  StaticAssertTypeEq<A, std::tuple_element<0, type_t<A>>::type>();
  StaticAssertTypeEq<B, std::tuple_element<1, type_t<A, B>>::type>();
  StaticAssertTypeEq<C, std::tuple_element<2, type_t<A, B, C>>::type>();
  StaticAssertTypeEq<D, std::tuple_element<3, type_t<A, B, C, D>>::type>();
  StaticAssertTypeEq<E, std::tuple_element<4, type_t<A, B, C, D, E>>::type>();
  StaticAssertTypeEq<F, std::tuple_element<5, type_t<A, B, C, D, E, F>>::type>();

  StaticAssertTypeEq<A, std::tuple_element<0, type_t<A, B, C, D, E, F>>::type>();
  StaticAssertTypeEq<B, std::tuple_element<1, type_t<A, B, C, D, E, F>>::type>();
  StaticAssertTypeEq<C, std::tuple_element<2, type_t<A, B, C, D, E, F>>::type>();
  StaticAssertTypeEq<D, std::tuple_element<3, type_t<A, B, C, D, E, F>>::type>();
  StaticAssertTypeEq<E, std::tuple_element<4, type_t<A, B, C, D, E, F>>::type>();
  StaticAssertTypeEq<F, std::tuple_element<5, type_t<A, B, C, D, E, F>>::type>();
}
