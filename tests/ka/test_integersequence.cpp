#include <iomanip>
#include <iostream>
#include <sstream>
#include <tuple>
#include <type_traits>
#include <boost/predef.h>
#include <gtest/gtest.h>
#include <ka/integersequence.hpp>

TEST(IntegerSequence, ValueType) {
  using namespace ka;
  static_assert(std::is_same<integer_sequence<int>::value_type, int>::value, "");
  static_assert(std::is_same<integer_sequence<short>::value_type, short>::value, "");
  static_assert(std::is_same<integer_sequence<unsigned long>::value_type, unsigned long>::value, "");
}

#define NOT_VS2013 (BOOST_COMP_MSVC == 0 || BOOST_COMP_MSVC >= BOOST_VERSION_NUMBER(19, 0, 0))

#if NOT_VS2013
TEST(IntegerSequence, Size) {
  using namespace ka;
  static_assert(integer_sequence<int>{}.size() == 0, "");
  static_assert(integer_sequence<int, 18>{}.size() == 1, "");
  static_assert(integer_sequence<int, 18, 9, 43>{}.size() == 3, "");
}
#endif

TEST(IntegerSequence, MakeIntegerSequence) {
  using namespace ka;
  static_assert(std::is_same<make_integer_sequence<int, 0>, integer_sequence<int>>::value, "");
  static_assert(std::is_same<make_integer_sequence<int, 1>, integer_sequence<int, 0>>::value, "");
  static_assert(std::is_same<make_integer_sequence<int, 2>, integer_sequence<int, 0, 1>>::value, "");
  static_assert(std::is_same<make_integer_sequence<int, 3>, integer_sequence<int, 0, 1, 2>>::value, "");
}

TEST(IntegerSequence, IndexSequence) {
  using namespace ka;
  static_assert(std::is_same<index_sequence<>, integer_sequence<std::size_t>>::value, "");
  static_assert(std::is_same<index_sequence<18>, integer_sequence<std::size_t, 18>>::value, "");
  static_assert(std::is_same<index_sequence<18, 9>, integer_sequence<std::size_t, 18, 9>>::value, "");
  static_assert(std::is_same<index_sequence<18, 9, 43>, integer_sequence<std::size_t, 18, 9, 43>>::value, "");
}

TEST(IntegerSequence, MakeIndexSequence) {
  using namespace ka;
  static_assert(std::is_same<make_index_sequence<0>, integer_sequence<std::size_t>>::value, "");
  static_assert(std::is_same<make_index_sequence<1>, integer_sequence<std::size_t, 0>>::value, "");
  static_assert(std::is_same<make_index_sequence<2>, integer_sequence<std::size_t, 0, 1>>::value, "");
  static_assert(std::is_same<make_index_sequence<3>, integer_sequence<std::size_t, 0, 1, 2>>::value, "");
}

TEST(IntegerSequence, IndexSequenceFor) {
  using namespace ka;
  static_assert(std::is_same<index_sequence_for<>, integer_sequence<std::size_t>>::value, "");
  static_assert(std::is_same<index_sequence_for<bool>, integer_sequence<std::size_t, 0>>::value, "");
  static_assert(std::is_same<index_sequence_for<bool, char>, integer_sequence<std::size_t, 0, 1>>::value, "");
  static_assert(std::is_same<index_sequence_for<bool, char, float>, integer_sequence<std::size_t, 0, 1, 2>>::value, "");
}

namespace test {
  template<typename Tuple, std::size_t... I>
  std::ostream& print(std::ostream& o, const Tuple& x, ka::index_sequence<I...>) {
    using swallow = std::initializer_list<int>;
    (void)swallow{0, (void(o << std::get<I>(x) << ' '), 0)...};
    return o;
  }

  template<typename... T>
  std::ostream& operator<<(std::ostream& o, const std::tuple<T...>& x) {
    return print(o, x, ka::index_sequence_for<T...>{});
  }
} // namespace test

TEST(IntegerSequence, RealWorld) {
  using namespace ka;
  using test::operator<<;
  std::tuple<bool, char, float> x{true, 'a', 3.23f};
  std::ostringstream ss;
  ss << std::boolalpha << x;
  ASSERT_EQ("true a 3.23 ", ss.str());
}
