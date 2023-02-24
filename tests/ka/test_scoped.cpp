#include <array>
#include <atomic>
#include <functional>
#include <memory>
#include <type_traits>
#include <gtest/gtest.h>
#include <ka/conceptpredicate.hpp>
#include <ka/range.hpp>
#include <ka/scoped.hpp>
#include <ka/testutils.hpp>

struct resource_t {
  bool open;
};

resource_t open_resource() {
  return {true};
}

void close_resource(resource_t& x) {
  x.open = false;
}

/// This is the canonical example of scoped_t as a RAII resource owner.
TEST(Scoped, FreeResource) {
  using namespace ka;
  resource_t r = open_resource();
  {
    auto s = scoped(std::ref(r), close_resource);
    EXPECT_TRUE(r.open);
  }
  EXPECT_FALSE(r.open);
}

/// Here, scoped_t is used not to free a resource, but to restore an initial state.
/// We want to restore the io flags on scope exit.
TEST(Scoped, RestoreState) {
  using namespace ka;
  using namespace std;
  ostringstream ss;
  ss.setf(ios::dec);
  {
    // setf() returns the old flags and we store them in the scoped_t object.
    // The lambda will set them again.
    auto s = scoped(ss.setf(ios::hex, ios::basefield), [&](ios::fmtflags previous) {
      ss.setf(previous, ios::basefield);
    });
    ss << 255;
    EXPECT_EQ("ff", ss.str());
  }
  ss.str("");
  ss << 255;
  EXPECT_EQ("255", ss.str());
}

/// Sometimes, we don't have a value and just want something to be done on
/// scope exit.
TEST(Scoped, Void) {
  using namespace ka;
  using namespace std;
  ostringstream ss;
  {
    auto s = scoped([&]{ss << "out of scope";});
    ss << "in scope - ";
    EXPECT_EQ("in scope - ", ss.str());
  }
  EXPECT_EQ("in scope - out of scope", ss.str());
}

/// A move-only function object that takes a `move_only_t` by r-value.
/// TODO: default move operation when get rid of VS2013.
struct move_only_fun_t {
  move_only_fun_t() = default;
  move_only_fun_t(const move_only_fun_t&) = delete;
  move_only_fun_t(move_only_fun_t&&) {}
  move_only_fun_t& operator=(const move_only_fun_t&) = delete;
  move_only_fun_t& operator=(move_only_fun_t&&) {return *this;}
  void operator()(ka::move_only_t<int*>&& m) const {
    ++(**m);
  }
};

/// Tests that a non-void scoped_t is moveable (construction only).
TEST(Scoped, Moveable) {
  using namespace ka;
  int i = 5;
  {
    auto s0 = scoped(move_only_t<int*>{&i}, move_only_fun_t{}); // move ctor
  }
  std::cerr << "i = " << i << '\n';
  EXPECT_EQ(6, i);
}

int move_only_fun_void_call_count = 0;

/// A move-only nullary function object.
/// TODO: default move operation when get rid of VS2013.
struct move_only_fun_void_t {
  move_only_fun_void_t() = default;
  move_only_fun_void_t(const move_only_fun_void_t&) = delete;
  move_only_fun_void_t(move_only_fun_void_t&&) {}
  move_only_fun_void_t& operator=(const move_only_fun_void_t&) = delete;
  move_only_fun_void_t& operator=(move_only_fun_void_t&&) {return *this;}
  void operator()() const {
    ++move_only_fun_void_call_count;
  }
};

/// Tests that a void scoped_t is moveable (construction only).
TEST(Scoped, MoveableVoid) {
  using namespace ka;
  move_only_fun_void_call_count = 0;
  {
    auto s0 = scoped(move_only_fun_void_t{}); // move ctor
    ASSERT_EQ(0, move_only_fun_void_call_count);
  }
  ASSERT_EQ(1, move_only_fun_void_call_count);
}

/// If you want to implement a release mechanism in order to prevent the "free"
/// function from being called, just test for a bool taken by reference in the
/// free itself.
TEST(Scoped, Release) {
  using namespace ka;
  int i = 5;
  {
    bool released = false;
    auto s = scoped(&i, [&](int* p) {
      if (!released) *p = 0;
    });
    released = true;
  }
  EXPECT_EQ(5, i);
}

std::string str_log;

/// Appends a string to a global variable when called.
/// Using a global variable is here ok, only because we're in unit tests.
struct log_scope_exit_t {
  KA_GENERATE_FRIEND_REGULAR_OPS_0(log_scope_exit_t)
  void operator()(const std::string& s) const {
    if (s.empty()) str_log += "|exit_of_scope";
    else           str_log += s;
  }
};

/// Sometimes, a default constructed `scoped_t` value is meaningful.
/// Here this is the case because the default constructed `std::string` is the
/// empty string and `log_scope_exit_t` has no state.
TEST(Scoped, DefaultCtor) {
  using namespace ka;
  using namespace std;
  str_log.clear();
  {
    auto s0 = scoped(string{"|END"}, log_scope_exit_t{});
    str_log += "START|";
    {
      // Here, s1 is default constructed because the default behavior is ok.
      decltype(s0) s1;
      str_log += "0, 1, 2";
    }
    EXPECT_EQ("START|0, 1, 2|exit_of_scope", str_log);
    {
      // Here, s1 is default constructed and the value is set afterwards.
      decltype(s0) s1;
      s1.value = "|exit_2nd_scope";
      str_log += "|mufuu";
    }
    EXPECT_EQ("START|0, 1, 2|exit_of_scope|mufuu|exit_2nd_scope", str_log);
    str_log += "|youpi les amis";
  }
  EXPECT_EQ("START|0, 1, 2|exit_of_scope|mufuu|exit_2nd_scope|youpi les amis|END", str_log);
}

namespace test_scoped {
  /// Function object that increments by a given step when called.
  struct incr_t {
    int step;
    KA_GENERATE_FRIEND_REGULAR_OPS_1(incr_t, step)
    void operator()(int& i) const {
      i += step;
    }
  };
} // namespace test_scoped

// is_copy_constructible and is_copy_assignable are buggy on MSVC 2013 and clang 6.5.
// MSVC 19 is Visual Studio 2015.
#define QI_TRAITS_COPY_OK \
  ((BOOST_COMP_MSVC  == 0 || BOOST_COMP_MSVC >= BOOST_VERSION_NUMBER(19, 0, 0)) && \
   (BOOST_COMP_CLANG == 0 || BOOST_COMP_CLANG >= BOOST_VERSION_NUMBER(7, 0, 0)))

TEST(Scoped, NoCopyNoAssignment) {
// is_copy_constructible and is_copy_assignable are broken on VS2013:
// https://connect.microsoft.com/VisualStudio/feedback/details/819202
#if QI_TRAITS_COPY_OK
  using namespace ka;
  static_assert(!std::is_copy_constructible<scoped_t<int, test_scoped::incr_t>>::value, "");
  static_assert(!std::is_copy_assignable<scoped_t<int, test_scoped::incr_t>>::value, "");
#endif
}

TEST(Scoped, NoCopyNoAssignmentVoid) {
// is_copy_constructible and is_copy_assignable are broken on VS2013:
// https://connect.microsoft.com/VisualStudio/feedback/details/819202
#if QI_TRAITS_COPY_OK
  auto print = []{std::cout << "end\n";};
  using F = decltype(print);

  using namespace ka;
  static_assert(!std::is_copy_constructible<scoped_t<void, F>>::value, "");
  static_assert(!std::is_copy_assignable<scoped_t<void, F>>::value, "");
#endif
}

/// Move operations are not defaulted because of VS2013.
struct non_moveable_non_copyable_t {
  non_moveable_non_copyable_t() = default;
  non_moveable_non_copyable_t(const non_moveable_non_copyable_t&) = delete;
  non_moveable_non_copyable_t(non_moveable_non_copyable_t&&) = delete;
  non_moveable_non_copyable_t& operator=(const non_moveable_non_copyable_t&) = delete;
  non_moveable_non_copyable_t& operator=(non_moveable_non_copyable_t&&) = delete;
};

template<typename... Args>
struct non_moveable_non_copyable_fun_t {
  non_moveable_non_copyable_fun_t() = default;
  non_moveable_non_copyable_fun_t(const non_moveable_non_copyable_fun_t&) = delete;
  non_moveable_non_copyable_fun_t(non_moveable_non_copyable_fun_t&&) {}
  non_moveable_non_copyable_fun_t& operator=(const non_moveable_non_copyable_fun_t&) = delete;
  non_moveable_non_copyable_fun_t& operator=(non_moveable_non_copyable_fun_t&&) {return *this;}
  void operator()(const Args&...) const {
  }
};

// Unfortunately, `std::is_move_constructible` and `std::is_move_assignable` are not
// able to detect that `Scoped` with not moveable nor copyable value or function
// is not itself moveable.
// Therefore, the commented statements must fail to compile, and we leave this
// code as-is, waiting for a better way to test this point.
TEST(Scoped, NoMoveOperationWhenValueOrFunctionIsNotMoveable) {
  using namespace ka;
  using T = non_moveable_non_copyable_t;
  using F = non_moveable_non_copyable_fun_t<T>;
  //scoped_t<T, F> s0{T{}, F{}};
  scoped_t<T, F> s1;
  //s1 = s0;
}

// See NoMoveOperationWhenValueOrFunctionIsNotMoveable comment.
TEST(Scoped, NoMoveOperationWhenValueOrFunctionIsNotMoveableVoid) {
  using namespace ka;
  using F = non_moveable_non_copyable_fun_t<>;
  //scoped_t<void, F> s0{F{}};
  scoped_t<void, F> s1;
  //s1 = s0;
}

struct dummy_t {
  KA_GENERATE_FRIEND_REGULAR_OPS_0(dummy_t)
  void operator()(int) const {
  }
};

// scoped_t is by-design not Regular, but we can nonetheless test equality.
TEST(Scoped, CorrectEquality) {
  using namespace ka;
  using namespace ka::detail;
  using S = scoped_t<int, dummy_t>;
  std::array<S, 100> arr;
  int i = 0;
  for (auto& s: arr) s.value = i++;
  EXPECT_TRUE(is_equivalence(std::equal_to<S>{}, bounded_range(arr)));
}

struct dummy_void_t {
  KA_GENERATE_FRIEND_REGULAR_OPS_0(dummy_void_t)
  void operator()() const {
  }
};

// Scoped is by-design not Regular, but we can nonetheless test equality.
TEST(Scoped, CorrectEqualityVoid) {
  using namespace ka;
  using S = scoped_t<void, dummy_void_t>;
  std::array<S, 100> arr;
  EXPECT_TRUE(is_equivalence(std::equal_to<S>{}, bounded_range(arr)));
}

// Idem for ordering.
TEST(Scoped, CorrectOrdering) {
  using namespace ka;
  using S = scoped_t<int, dummy_t>;
  std::array<S, 100> arr;
  int i = 0;
  for (auto& s: arr) s.value = i++;
  EXPECT_TRUE(is_total_ordering(std::less<S>{}, bounded_range(arr)));
}

// Idem for ordering (void version).
TEST(Scoped, CorrectOrderingVoid) {
  using namespace ka;
  using S = scoped_t<void, dummy_void_t>;
  std::array<S, 100> arr;
  EXPECT_TRUE(is_total_ordering(std::less<S>{}, bounded_range(arr)));
}

TEST(ScopedSetAndRestore, Basic) {
  using namespace ka;
  const int old_value = 0x8BADF00D;
  const int new_value = 0x0DEFACED;
  int x = old_value;
  {
    auto _ = scoped_set_and_restore(x, new_value);
    EXPECT_EQ(new_value, x);
  }
  EXPECT_EQ(old_value, x);
}

TEST(ScopedSetAndRestore, DifferentButCompatibleTypes) {
  using namespace ka;
  const std::string old_value{"blibli"};
  const char* new_value = "bloublou";
  std::string x = old_value;
  {
    auto _ = scoped_set_and_restore(x, new_value);
    EXPECT_EQ(new_value, x);
  }
  EXPECT_EQ(old_value, x);
}

TEST(ScopedSetAndRestore, MoveOnly) {
  using namespace ka;
  using M = move_only_t<int>;
  const M old_value{1212};
  M new_value0{3434};
  M new_value1{*new_value0};
  M x{*old_value};
  {
    auto _ = scoped_set_and_restore(x, std::move(new_value0));
    EXPECT_EQ(new_value1, x);
  }
  EXPECT_EQ(old_value, x);
}

TEST(ScopedSetAndRestore, NewValueIsUntouched) {
  using namespace ka;
  using namespace test_scoped;
  const int initial_new_value = 2325895;
  move_aware_t old_value{8736363};
  move_aware_t new_value{initial_new_value};
  move_aware_t x{old_value.i};
  {
    auto _ = scoped_set_and_restore(x, new_value);
    EXPECT_FALSE(new_value.moved);
    EXPECT_EQ(new_value.i, initial_new_value);
    EXPECT_EQ(new_value, x);
  }
  EXPECT_EQ(old_value, x);
}

TEST(ScopedApplyAndRetract, Action) {
  using namespace ka;
  using atomic_t = std::atomic<int>;
  const int old_value{7676};
  const int new_value{old_value + 1};
  atomic_t x{old_value};
  {
    auto _ = scoped_apply_and_retract(x, incr_mono_t<atomic_t>{}, decr_mono_t<atomic_t>{});
    EXPECT_EQ(new_value, x.load());
  }
  EXPECT_EQ(old_value, x.load());
}

namespace test_scoped {
  template<typename T>
  struct half_t {
    void operator()(T& t) {
      t -= t / 2;
    }
  };

  template<typename T>
  struct twice_t {
  // Action<T>:
    void operator()(T& t) {
      t += t;
    }
  // RetractableAction<T>:
    // TODO: Remove this typedef when get rid of VS2013.
    using retract_type = half_t<T>;

    friend half_t<T> retract(twice_t const&) {
      return {};
    }
  };
} // namespace test_scoped

TEST(ScopedApplyAndRetract, RetractableAction) {
  using namespace ka;
  const int old_value{7676};
  const int new_value{2 * old_value};
  int x = old_value;
  {
    auto _ = scoped_apply_and_retract(x, test_scoped::twice_t<int>{});
    EXPECT_EQ(new_value, x);
  }
  EXPECT_EQ(old_value, x);
}

TEST(ScopedIncrAndDecr, Basic) {
  using namespace ka;
  const int old_value = 8376832;
  int x = old_value;
  {
    auto _ = scoped_incr_and_decr(x);
    EXPECT_EQ(x, old_value + 1);
  }
  EXPECT_EQ(x, old_value);
}
