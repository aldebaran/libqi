#include <string>
#include <string_view>
#include <vector>
#include <gtest/gtest.h>
#include <boost/function.hpp>
#include <boost/optional.hpp>
#include <ka/opt.hpp>
#include <ka/memory.hpp>
#include <ka/empty.hpp>
#include <ka/functional.hpp>

TEST(Empty, Pointer) {
  // A pointer is empty iff it is null.
  ASSERT_TRUE(ka::empty(nullptr));
  int* p0 = nullptr;
  ASSERT_TRUE(ka::empty(p0));
  int i = 5;
  int* p1 = &i;
  ASSERT_FALSE(ka::empty(p1));
}

TEST(Empty, InitializerList) {
  ASSERT_TRUE(ka::empty(std::initializer_list<int>{}));
  ASSERT_FALSE(ka::empty(std::initializer_list<int>{1}));
  ASSERT_FALSE(ka::empty(std::initializer_list<int>{1, 2}));
}

TEST(Empty, StdStringView) {
  auto s = "abcd";
  ASSERT_TRUE(ka::empty(std::string_view()));
  ASSERT_TRUE(ka::empty(std::string_view(s, 0)));
  ASSERT_FALSE(ka::empty(std::string_view(s)));
  ASSERT_FALSE(ka::empty(std::string_view(s, 1)));
}

TEST(Empty, BoostOptional) {
  ASSERT_TRUE(ka::empty(boost::optional<int>{}));
  ASSERT_FALSE(ka::empty(boost::optional<int>{5}));
}

TEST(Empty, KaOpt) {
  ASSERT_TRUE(ka::empty(ka::opt_t<int>{}));
  ASSERT_TRUE(ka::empty(ka::opt_t<void>{}));
  ASSERT_FALSE(ka::empty(ka::opt_t<int>{5}));
  ASSERT_FALSE(ka::empty(ka::opt())); // opt_t<void> that is non-empty
}

TEST(Empty, StdUniquePtr) {
  ASSERT_TRUE(ka::empty(std::unique_ptr<int>{}));
  ASSERT_FALSE(ka::empty(std::unique_ptr<int>{new int(1)}));
}

TEST(Empty, StdSharedPtr) {
  ASSERT_TRUE(ka::empty(std::shared_ptr<int>{}));
  ASSERT_FALSE(ka::empty(std::shared_ptr<int>{new int(1)}));
}

TEST(Empty, StdContainer) {
  // `ka::empty` can also be used with standard containers, since they define an
  // `empty` member function.
  ASSERT_TRUE(ka::empty(std::vector<int>{}));
  ASSERT_FALSE(ka::empty(std::vector<int>{1, 2, 3}));
}

TEST(Empty, StdFunction) {
  ASSERT_TRUE(ka::empty(std::function<bool (int)>{}));
  ASSERT_FALSE(ka::empty(std::function<int ()>{ka::constant_function(3)}));
}

TEST(Empty, BoostFunction) {
  ASSERT_TRUE(ka::empty(boost::function<bool (int)>{}));
  ASSERT_FALSE(ka::empty(boost::function<int ()>{ka::constant_function(3)}));
}

namespace {
  struct my0_t {
    int i;
    bool empty() const {
      return i == 3245;
    }
  };
} // namespace

TEST(Empty, Custom) {
  // `ka::empty` finds `empty` member function for any type.
  ASSERT_FALSE(ka::empty(my0_t{3244}));
  ASSERT_TRUE(ka::empty(my0_t{3245}));
}

namespace {
  // Member function and free function:
  struct my1_t {
    int i;
    bool empty() const {
      return i == 789;
    }
  };

  bool empty(my1_t x) {
    return x.i == 123;
  }
} // namespace

TEST(Empty, MemberFunctionAndFreeFunction) {
  // What happens if there is a member function _and_ a free function?
  // (suspense)
  ASSERT_FALSE(ka::empty(my1_t{789}));
  ASSERT_TRUE(ka::empty(my1_t{123}));
}

namespace ka {
  template<typename T>
  bool my_empty(T&& t)  {
    // Don't have to qualify inside `ka`.
    return empty(fwd<T>(t));
  }
} // namespace ka

TEST(Empty, InsideKa) {
  using ka::my_empty;

  ASSERT_TRUE(my_empty(nullptr));
  int* p0 = nullptr;
  ASSERT_TRUE(my_empty(p0));
  int i = 5;
  int* p1 = &i;
  ASSERT_FALSE(my_empty(p1));

  ASSERT_TRUE(my_empty(boost::optional<int>{}));
  ASSERT_FALSE(my_empty(boost::optional<int>{5}));

  ASSERT_TRUE(my_empty(std::vector<int>{}));
  ASSERT_FALSE(my_empty(std::vector<int>{1, 2, 3}));
}

TEST(Empty, Compose) {
  // `ka::empty` is not a template function but a polymorphic function object,
  // so it is possible to use it directly for function composition.
  auto str = [](bool x) -> std::string {return x ? "true" : "false";};
  auto f = ka::compose(str, ka::empty);
  int i = 45;
  ASSERT_EQ("true", f(nullptr));
  ASSERT_EQ("false", f(&i));
  ASSERT_EQ("true", f(ka::opt_t<int>()));
  ASSERT_EQ("false", f(std::vector<int>{1, 2, 3}));
}
