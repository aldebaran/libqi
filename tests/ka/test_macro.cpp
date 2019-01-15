#include <gtest/gtest.h>
#include <ka/macro.hpp>
#include <ka/macroregular.hpp>
#include <ka/typetraits.hpp>
#include <ka/utility.hpp>

namespace {

// non-template, 0 member
struct stuff0_t {
  KA_GENERATE_FRIEND_REGULAR_OPS_0(stuff0_t)
};

// non-template, 1 member
struct stuff1_t {
  int a;
  KA_GENERATE_FRIEND_REGULAR_OPS_1(stuff1_t, a)
};

// non-template, 1 member, explicit
struct stuff1_explicit_t {
  int a;
  explicit stuff1_explicit_t(int a) : a(a) {
  }
  KA_GENERATE_FRIEND_REGULAR_OPS_1(stuff1_explicit_t, a)
};

// non-template, 2 members
struct stuff2_t {
  int a;
  bool b;
  KA_GENERATE_FRIEND_REGULAR_OPS_2(stuff2_t, a, b)
};

KA_DERIVE_CTOR_FUNCTION(stuff0)
KA_DERIVE_CTOR_FUNCTION(stuff1)
KA_DERIVE_CTOR_FUNCTION(stuff1_explicit)
KA_DERIVE_CTOR_FUNCTION(stuff2)

} // namespace

TEST(Macro, KaDeriveCtorFunction) {
  int const i = 5;
  bool const b = true;

  ASSERT_EQ(stuff0_t{}, stuff0());
  ASSERT_EQ(stuff1_t{i}, stuff1(i));
  ASSERT_EQ(stuff1_explicit_t{i}, stuff1_explicit(i));
  ASSERT_EQ((stuff2_t{i, b}), stuff2(i, b));
}

namespace {

// template, 1 member
template<typename A>
struct stuff3_t {
  A a;
  KA_GENERATE_FRIEND_REGULAR_OPS_1(stuff3_t, a)
};

// template, 1 member, explicit
template<typename A>
struct stuff3_explicit_t {
  A a;
  explicit stuff3_explicit_t(A a) : a(a) {
  }
  KA_GENERATE_FRIEND_REGULAR_OPS_1(stuff3_explicit_t, a)
};

// template, 2 members
template<typename A, typename B>
struct stuff4_t {
  A a;
  B b;
  KA_GENERATE_FRIEND_REGULAR_OPS_2(stuff4_t, a, b)
};

KA_DERIVE_CTOR_FUNCTION_TEMPLATE(stuff3)
KA_DERIVE_CTOR_FUNCTION_TEMPLATE(stuff3_explicit)
KA_DERIVE_CTOR_FUNCTION_TEMPLATE(stuff4)

} // namespace

TEST(Macro, KaDeriveCtorFunctionTemplate) {
  int const a = 5;
  bool const b = true;

  ASSERT_EQ(stuff3_t<int>{a}, stuff3(a));
  ASSERT_EQ(stuff3_explicit_t<int>{a}, stuff3_explicit(a));
  ASSERT_EQ((stuff4_t<int, bool>{a, b}), stuff4(a, b));
}

namespace {

// template, void specialization
template<typename A>
struct stuff5_t {
  A a;
  KA_GENERATE_FRIEND_REGULAR_OPS_1(stuff5_t, a)
};

template<>
struct stuff5_t<void> {
  KA_GENERATE_FRIEND_REGULAR_OPS_0(stuff5_t)
};

// template, void specialization, explicit
template<typename A>
struct stuff5_explicit_t {
  A a;
  explicit stuff5_explicit_t(A a) : a(a) {
  }
  KA_GENERATE_FRIEND_REGULAR_OPS_1(stuff5_explicit_t, a)
};

template<>
struct stuff5_explicit_t<void> {
  KA_GENERATE_FRIEND_REGULAR_OPS_0(stuff5_explicit_t)
};

KA_DERIVE_CTOR_FUNCTION_TEMPLATE(stuff5)
KA_DERIVE_CTOR_FUNCTION_TEMPLATE_VOID(stuff5)
KA_DERIVE_CTOR_FUNCTION_TEMPLATE(stuff5_explicit)
KA_DERIVE_CTOR_FUNCTION_TEMPLATE_VOID(stuff5_explicit)

} // namespace

TEST(Macro, KaDeriveCtorFunctionTemplateVoid) {
  int const a = 5;

  ASSERT_EQ(stuff5_t<int>{a}, stuff5(a));
  ASSERT_EQ(stuff5_t<void>{}, stuff5());
  ASSERT_EQ(stuff5_explicit_t<int>{a}, stuff5_explicit(a));
  ASSERT_EQ(stuff5_explicit_t<void>{}, stuff5_explicit());
}

namespace {
  struct X {
    KA_CONSTEXPR
    int f(int i) const {
      return i;
    }
  };
}

TEST(Macro, KaConstexpr) {
  ASSERT_EQ(X{}.f(5), 5);
#ifndef BOOST_NO_CXX11_CONSTEXPR
  static_assert(X{}.f(5) == 5, "");
#endif
}
