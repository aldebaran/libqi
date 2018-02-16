#pragma once
#include <tuple>
#include <type_traits>
#include <boost/config.hpp>

namespace test
{
  template<std::size_t N>
  using index = std::integral_constant<std::size_t, N>;

  template<typename A, typename B, typename C>
  struct X
  {
    A a;
    B b;
    C c;

    bool operator==(const X& y) const
    {
      return a == y.a && b == y.b && c == y.c;
    }

    A& get(index<0>) {return a;}
    B& get(index<1>) {return b;}
    C& get(index<2>) {return c;}
    const A& get(index<0>) const {return a;}
    const B& get(index<1>) const {return b;}
    const C& get(index<2>) const {return c;}
  };

} // namespace test

namespace std
{
  template<typename A, typename B, typename C>
  struct tuple_size<test::X<A, B, C>> : integral_constant<size_t, 3>
  {
  };

  template<size_t I, typename A, typename B, typename C>
  BOOST_CONSTEXPR auto get(test::X<A, B, C>& x)
    -> decltype(x.get(integral_constant<size_t, I>{}))
  {
    return x.get(integral_constant<size_t, I>{});
  }

  template<size_t I, typename A, typename B, typename C>
  BOOST_CONSTEXPR auto get(const test::X<A, B, C>& x)
    -> decltype(x.get(integral_constant<size_t, I>{}))
  {
    return x.get(integral_constant<size_t, I>{});
  }
} // namespace std
