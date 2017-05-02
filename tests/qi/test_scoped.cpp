#include <qi/application.hpp>
#include <gtest/gtest.h>
#include <qi/scoped.hpp>
#include <qi/detail/conceptpredicate.hpp>
#include <qi/range.hpp>
#include <qi/log.hpp>
#include <array>
#include <functional>
#include <type_traits>
#include <memory>

struct Resource
{
  bool open;
};

Resource openResource()
{
  return {true};
}

void closeResource(Resource& x)
{
  x.open = false;
}

/// This is the canonical example of Scoped as a RAII resource owner.
TEST(Scoped, FreeResource)
{
  Resource r = openResource();
  {
    auto s = qi::scoped(std::ref(r), closeResource);
    EXPECT_TRUE(r.open);
  }
  EXPECT_FALSE(r.open);
}

/// Here, Scoped is used not to free a resource, but to restore an initial state.
/// We want to restore the io flags on scope exit.
TEST(Scoped, RestoreState)
{
  using namespace std;
  ostringstream ss;
  ss.setf(ios::dec);
  {
    // setf() returns the old flags and we store them in the Scoped object.
    // The lambda will set them again.
    auto s = qi::scoped(ss.setf(ios::hex, ios::basefield), [&](ios::fmtflags previous) {
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
TEST(Scoped, Void)
{
  using namespace std;
  ostringstream ss;
  {
    auto s = qi::scoped([&]{ss << "out of scope";});
    ss << "in scope - ";
    EXPECT_EQ("in scope - ", ss.str());
  }
  EXPECT_EQ("in scope - out of scope", ss.str());
}

template<typename T>
struct DeleterNoOp
{
  void operator()(T*) const
  {
  }
};

// unique_ptr is move-only.
using MoveOnly = std::unique_ptr<int, DeleterNoOp<int>>;

/// A move-only function object that takes a MoveOnly by r-value.
/// TODO: default move operation when get rid of VS2013.
struct MoveOnlyFun
{
  MoveOnlyFun() = default;
  MoveOnlyFun(const MoveOnlyFun&) = delete;
  MoveOnlyFun(MoveOnlyFun&&) {}
  MoveOnlyFun& operator=(const MoveOnlyFun&) = delete;
  MoveOnlyFun& operator=(MoveOnlyFun&&) {return *this;}
  void operator()(MoveOnly&& m) const
  {
    ++(*m);
  }
};

/// Tests that a non-void Scoped is moveable (construction and assignment).
TEST(Scoped, Moveable)
{
  int i = 5;
  {
    auto s0 = qi::scoped(MoveOnly{&i}, MoveOnlyFun{}); // move ctor
    int j = i + 1;
    auto s1 = qi::scoped(MoveOnly{&j}, MoveOnlyFun{}); // move ctor
    s1 = std::move(s0); // move assign
    EXPECT_EQ(i, *s1.value);
  }
  std::cerr << "i = " << i << '\n';
  EXPECT_EQ(6, i);
}

int moveOnlyFunVoidCallCount = 0;

/// A move-only nullary function object.
/// TODO: default move operation when get rid of VS2013.
struct MoveOnlyFunVoid
{
  MoveOnlyFunVoid() = default;
  MoveOnlyFunVoid(const MoveOnlyFunVoid&) = delete;
  MoveOnlyFunVoid(MoveOnlyFunVoid&&) {}
  MoveOnlyFunVoid& operator=(const MoveOnlyFunVoid&) = delete;
  MoveOnlyFunVoid& operator=(MoveOnlyFunVoid&&) {return *this;}
  void operator()() const
  {
    ++moveOnlyFunVoidCallCount;
  }
};

/// Tests that a void Scoped is moveable (construction and assignment).
TEST(Scoped, MoveableVoid)
{
  moveOnlyFunVoidCallCount = 0;
  {
    auto s0 = qi::scoped(MoveOnlyFunVoid{}); // move ctor
    ASSERT_EQ(0, moveOnlyFunVoidCallCount);
    auto s1 = qi::scoped(MoveOnlyFunVoid{}); // move ctor
    ASSERT_EQ(0, moveOnlyFunVoidCallCount);
    s1 = std::move(s0); // move assign
    ASSERT_EQ(0, moveOnlyFunVoidCallCount);
  }
  ASSERT_EQ(1, moveOnlyFunVoidCallCount);
}

/// If you want to implement a release mechanism in order to prevent the "free"
/// function from being called, just test for a bool taken by reference in the
/// free itself.
TEST(Scoped, Release)
{
  int i = 5;
  {
    bool released = false;
    auto s = qi::scoped(&i, [&](int* p) {
      if (!released) *p = 0;
    });
    released = true;
  }
  EXPECT_EQ(5, i);
}

std::string strLog;

/// Appends a string to a global variable when called.
/// Using a global variable is here ok, only because we're in unit tests.
struct LogScopeExit
{
  QI_GENERATE_FRIEND_REGULAR_OPS_0(LogScopeExit)
  void operator()(const std::string& s) const
  {
    if (s.empty()) strLog += "|exit_of_scope";
    else           strLog += s;
  }
};

/// Sometimes, a default constructed Scoped value is meaningful.
/// Here this is the case because the default constructed std::string is the
/// empty string and LogScopeExit has no state.
TEST(Scoped, DefaultCtor)
{
  using namespace std;
  strLog.clear();
  {
    auto s0 = qi::scoped(string{"|END"}, LogScopeExit{});
    strLog += "START|";
    {
      // Here, s1 is default constructed because the default behavior is ok.
      decltype(s0) s1;
      strLog += "0, 1, 2";
    }
    EXPECT_EQ("START|0, 1, 2|exit_of_scope", strLog);
    {
      // Here, s1 is default constructed and the value is set afterwards.
      decltype(s0) s1;
      s1.value = "|exit_2nd_scope";
      strLog += "|mufuu";
    }
    EXPECT_EQ("START|0, 1, 2|exit_of_scope|mufuu|exit_2nd_scope", strLog);
    strLog += "|youpi les amis";
  }
  EXPECT_EQ("START|0, 1, 2|exit_of_scope|mufuu|exit_2nd_scope|youpi les amis|END", strLog);
}

/// Function object that increments by a given step when called.
struct Incr
{
  int step;
  QI_GENERATE_FRIEND_REGULAR_OPS_1(Incr, step)
  void operator()(int& i) const
  {
    i += step;
  }
};

// is_copy_constructible and is_copy_assignable are buggy on MSVC 2013 and clang 6.5.
// MSVC 19 is Visual Studio 2015.
#define QI_TRAITS_COPY_OK \
  ((BOOST_COMP_MSVC  == 0 || BOOST_COMP_MSVC >= BOOST_VERSION_NUMBER(19, 0, 0)) && \
   (BOOST_COMP_CLANG == 0 || BOOST_COMP_CLANG >= BOOST_VERSION_NUMBER(7, 0, 0)))

TEST(Scoped, NoCopyNoAssignment)
{
// is_copy_constructible and is_copy_assignable are broken on VS2013:
// https://connect.microsoft.com/VisualStudio/feedback/details/819202
#if QI_TRAITS_COPY_OK
  using namespace qi;
  static_assert(!std::is_copy_constructible<Scoped<int, Incr>>::value, "");
  static_assert(!std::is_copy_assignable<Scoped<int, Incr>>::value, "");
#endif
}

TEST(Scoped, NoCopyNoAssignmentVoid)
{
  auto print = []{std::cout << "end\n";};
  using F = decltype(print);

// is_copy_constructible and is_copy_assignable are broken on VS2013:
// https://connect.microsoft.com/VisualStudio/feedback/details/819202
#if QI_TRAITS_COPY_OK
  using namespace qi;
  static_assert(!std::is_copy_constructible<Scoped<void, F>>::value, "");
  static_assert(!std::is_copy_assignable<Scoped<void, F>>::value, "");
#endif
}

/// Move operations are not defaulted because of VS2013.
struct NonMoveableNonCopyable
{
  NonMoveableNonCopyable() = default;
  NonMoveableNonCopyable(const NonMoveableNonCopyable&) = delete;
  NonMoveableNonCopyable(NonMoveableNonCopyable&&) = delete;
  NonMoveableNonCopyable& operator=(const NonMoveableNonCopyable&) = delete;
  NonMoveableNonCopyable& operator=(NonMoveableNonCopyable&&) = delete;
};

template<typename... Args>
struct NonMoveableNonCopyableFun
{
  NonMoveableNonCopyableFun() = default;
  NonMoveableNonCopyableFun(const NonMoveableNonCopyableFun&) = delete;
  NonMoveableNonCopyableFun(NonMoveableNonCopyableFun&&) {}
  NonMoveableNonCopyableFun& operator=(const NonMoveableNonCopyableFun&) = delete;
  NonMoveableNonCopyableFun& operator=(NonMoveableNonCopyableFun&&) {return *this;}
  void operator()(const Args&...) const
  {
  }
};

// Unfortunately, std::is_move_constructible and std::is_move_assignable are not
// able to detect that Scoped with not moveable nor copyable value or function
// is not itself moveable.
// Therefore, the commented statements must fail to compile, and we leave this
// code as-is, waiting for a better way to test this point.
TEST(Scoped, NoMoveOperationWhenValueOrFunctionIsNotMoveable)
{
  using namespace qi;
  using T = NonMoveableNonCopyable;
  using F = NonMoveableNonCopyableFun<T>;
  //Scoped<T, F> s0{T{}, F{}};
  Scoped<T, F> s1;
  //s1 = s0;
}

// See NoMoveOperationWhenValueOrFunctionIsNotMoveable comment.
TEST(Scoped, NoMoveOperationWhenValueOrFunctionIsNotMoveableVoid)
{
  using namespace qi;
  using F = NonMoveableNonCopyableFun<>;
  //Scoped<void, F> s0{F{}};
  Scoped<void, F> s1;
  //s1 = s0;
}

struct Dummy
{
  QI_GENERATE_FRIEND_REGULAR_OPS_0(Dummy)
  void operator()(int i) const
  {
  }
};

// Scoped is by-design not Regular, but we can nonetheless test equality.
TEST(Scoped, CorrectEquality)
{
  using namespace qi;
  using namespace qi::detail;
  using S = Scoped<int, Dummy>;
  std::array<S, 100> arr;
  int i = 0;
  for (auto& s: arr) s.value = i++;
  EXPECT_TRUE(isEquivalence(std::equal_to<S>{}, boundedRange(arr)));
}

struct DummyVoid
{
  QI_GENERATE_FRIEND_REGULAR_OPS_0(DummyVoid)
  void operator()() const
  {
  }
};

// Scoped is by-design not Regular, but we can nonetheless test equality.
TEST(Scoped, CorrectEqualityVoid)
{
  using namespace qi;
  using namespace qi::detail;
  using S = Scoped<void, DummyVoid>;
  std::array<S, 100> arr;
  EXPECT_TRUE(isEquivalence(std::equal_to<S>{}, boundedRange(arr)));
}

// Idem for ordering.
TEST(Scoped, CorrectOrdering)
{
  using namespace qi;
  using namespace qi::detail;
  using S = Scoped<int, Dummy>;
  std::array<S, 100> arr;
  int i = 0;
  for (auto& s: arr) s.value = i++;
  EXPECT_TRUE(isTotalOrdering(std::less<S>{}, boundedRange(arr)));
}

// Idem for ordering (void version).
TEST(Scoped, CorrectOrderingVoid)
{
  using namespace qi;
  using namespace qi::detail;
  using S = Scoped<void, DummyVoid>;
  std::array<S, 100> arr;
  EXPECT_TRUE(isTotalOrdering(std::less<S>{}, boundedRange(arr)));
}
