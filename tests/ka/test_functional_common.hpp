#pragma once
#ifndef TESTS_KA_FUNCTIONALCOMMON_HPP
#define TESTS_KA_FUNCTIONALCOMMON_HPP
#include <tuple>
#include <type_traits>
#include <boost/config.hpp>
#include <ka/macroregular.hpp>
#include <ka/macro.hpp>

namespace test_functional {
  template<std::size_t N>
  using index = std::integral_constant<std::size_t, N>;

  template<typename A, typename B, typename C>
  struct x_t {
    A a;
    B b;
    C c;

    bool operator==(x_t const& y) const {
      return a == y.a && b == y.b && c == y.c;
    }

    A& get(index<0>) {return a;}
    B& get(index<1>) {return b;}
    C& get(index<2>) {return c;}
    A const& get(index<0>) const {return a;}
    B const& get(index<1>) const {return b;}
    C const& get(index<2>) const {return c;}
  };

} // namespace test_functional

namespace std {

KA_WARNING_PUSH()
KA_WARNING_DISABLE(, pragmas)
KA_WARNING_DISABLE(, mismatched-tags)
  template<typename A, typename B, typename C>
  struct tuple_size<test_functional::x_t<A, B, C>> : integral_constant<size_t, 3> {
  };
KA_WARNING_POP()

  template<size_t I, typename A, typename B, typename C>
  BOOST_CONSTEXPR auto get(test_functional::x_t<A, B, C>& x)
      -> decltype(x.get(integral_constant<size_t, I>{})) {
    return x.get(integral_constant<size_t, I>{});
  }

  template<size_t I, typename A, typename B, typename C>
  BOOST_CONSTEXPR auto get(test_functional::x_t<A, B, C> const& x)
      -> decltype(x.get(integral_constant<size_t, I>{})) {
    return x.get(integral_constant<size_t, I>{});
  }
} // namespace std

namespace test_functional {
  enum class e0_t {
    a = 345,
    b = 432
  };

  enum class e1_1 {
    x = 0,
    y = 1
  };

  struct f_inv_t;

  // a -> x
  // b -> y
  struct f_t {
  // Regular:
    KA_GENERATE_FRIEND_REGULAR_OPS_0(f_t)
  // Function<e1_t (e0_t)>:
    inline e1_1 operator()(e0_t e) const {
      return e == e0_t::a
          ? e1_1::x
          : e1_1::y;
    }
  // Isomorphism<e1_t, e0_t>:
    using retract_type  = f_inv_t;
  };

  // x -> a
  // y -> b
  struct f_inv_t {
  // Regular:
    KA_GENERATE_FRIEND_REGULAR_OPS_0(f_inv_t)
  // Function<e0_t (e1_t)>:
    inline e0_t operator()(e1_1 e) const {
      return e == e1_1::x
          ? e0_t::a
          : e0_t::b;
    }
  // Isomorphism<e1_t, e0_t>:
    using retract_type  = f_t;
    inline friend f_t retract(f_inv_t) {
      return {};
    }
  };

  // Isomorphism<e0_t, e1_t> f_inv_t:
  inline f_inv_t retract(f_t) {
    return {};
  }

  struct a_inv_t;

  // a -> b
  // b -> a
  struct a_t {
    inline void operator()(e0_t& e) const {
      e = (e == e0_t::a) ? e0_t::b : e0_t::a;
    }
    using retract_type = a_inv_t;
  };

  // `a_t` is in fact its own retraction, but we still use a separate type for tests.
  // a -> b
  // b -> a
  struct a_inv_t {
    inline void operator()(e0_t& e) const {
      e = (e == e0_t::a) ? e0_t::b : e0_t::a;
    }
    inline friend a_t retract(a_inv_t) {
      return {};
    }
    using retract_type = a_t;
  };

  inline a_inv_t retract(a_t) {
    return {};
  }

  enum class e2_t {
    a = 345,
    b = 432
  };

  enum class e3_t {
    x = 0,
    y = 1
  };

  struct g_inv_t;

  // a -> y
  // b -> x
  struct g_t {
    inline e1_1 operator()(e0_t e) const {
      return e == e0_t::a
          ? e1_1::y
          : e1_1::x;
    }
    using retract_type = g_inv_t;
  };

  // x -> b
  // y -> a
  struct g_inv_t {
    inline e0_t operator()(e1_1 e) const {
      return e == e1_1::x
          ? e0_t::b
          : e0_t::a;
    }
    inline friend g_t retract(g_inv_t) {
      return {};
    }
    using retract_type = g_t;
  };

  inline g_inv_t retract(g_t) {
    return {};
  }

  struct one_t {
  };

  struct two_t {
    bool b;
  };

  // one_t     two_t
  //  {} <---- true
  //   ^------ false
  struct one_ {
    inline one_t operator()(two_t) const {
      return {};
    }
  };

  // one_t     two_t
  //  {} ----> true
  //           false
  struct true_ {
    inline two_t operator()(one_t) const {
      return {true};
    }
    inline friend one_ retract(true_) {
      return {};
    }
    using retract_type = one_;
  };
} // namespace test_functional

#endif // TESTS_KA_FUNCTIONALCOMMON_HPP
