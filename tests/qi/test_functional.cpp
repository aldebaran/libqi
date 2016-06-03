#include <cstdlib>
#include <limits>
#include <list>
#include <sstream>
#include <string>
#include <vector>
#include <future>
#include <qi/application.hpp>
#include <gtest/gtest.h>
#include <qi/detail/conceptpredicate.hpp>
#include "test_functional_common.hpp"
#include <qi/functional.hpp>
#include <qi/detail/conceptpredicate.hpp>
#include <qi/range.hpp>
#include <qi/future.hpp>
#include <qi/type/traits.hpp>
#include <qi/utility.hpp>
#include <qi/mutablestore.hpp>
#include <qi/mutex.hpp>
#include "tools.hpp"

TEST(FunctionalPolymorphicConstantFunction, RegularNonVoid)
{
  using namespace qi;
  using namespace qi::detail;
  using F = PolymorphicConstantFunction<int>;
  const auto incr = [](F& f) {
    ++f.ret;
  };
  // F is regular because int is.
  ASSERT_TRUE(isRegular(incrRange(F{0}, F{100}, incr)));
}

TEST(FunctionalPolymorphicConstantFunction, RegularVoid)
{
  using namespace qi;
  using namespace qi::detail;
  using F = PolymorphicConstantFunction<void>;
  ASSERT_TRUE(isRegular({F{}}));
}

struct NonRegular
{
  int i;
  friend bool operator==(NonRegular a, NonRegular b)
  {
    return &a == &b;
  }
  friend bool operator<(NonRegular a, NonRegular b)
  {
    return &a < &b;
  }
};

TEST(FunctionalPolymorphicConstantFunction, NonRegularNonVoid)
{
  using namespace qi;
  using namespace qi::detail;
  using F = PolymorphicConstantFunction<NonRegular>;
  auto incr = [](F& f) {
    ++f.ret.i;
  };
  // F is not regular because NonRegular isn't.
  ASSERT_FALSE(isRegular(incrRange(F{{0}}, F{{100}}, incr)));
}

TEST(FunctionalPolymorphicConstantFunction, BasicNonVoid)
{
  using namespace qi;
  const char c = 'z';
  PolymorphicConstantFunction<unsigned char> f{c};
  ASSERT_EQ(c, f());
  ASSERT_EQ(c, f(1));
  ASSERT_EQ(c, f(2.345));
  ASSERT_EQ(c, f("abcd"));
  ASSERT_EQ(c, f(true));
  ASSERT_EQ(c, f(std::vector<int>{5, 7, 2, 1}));
  ASSERT_EQ(c, f(1, 2.345, "abcd", true));
}

TEST(FunctionalPolymorphicConstantFunction, BasicVoid)
{
  using namespace qi;
  PolymorphicConstantFunction<void> f;
  ASSERT_NO_THROW(f());
  ASSERT_NO_THROW(f(1));
  ASSERT_NO_THROW(f(2.345));
  ASSERT_NO_THROW(f("abcd"));
  ASSERT_NO_THROW(f(true));
  ASSERT_NO_THROW(f(std::vector<int>{5, 7, 2, 1}));
  ASSERT_NO_THROW(f(1, 2.345, "abcd", true));
}

namespace
{
  // For use to test isRegular only.
  // The returned strings are irrelevant, the only point is that these functions
  // are regular.
  std::string strbool0(bool x)
  {
    return x ? "test test" : "1, 2, 1, 2";
  }

  std::string strbool1(bool x)
  {
    return x ? "mic mic" : "Vous etes chauds ce soir?!";
  }
}

TEST(FunctionalCompose, Regular)
{
  using namespace qi;
  using namespace std;
  using C = Composition<string (*)(bool), bool (*)(float)>;
  ASSERT_TRUE(detail::isRegular({
    C{strbool0, isnan}, C{strbool0, isfinite}, C{strbool1, isinf}
  }));
}

TEST(FunctionalCompose, NonVoid)
{
  using namespace qi;

  auto half = [](int x) {
    return x / 2.f;
  };
  auto greater_1 = [](float x) {
    return x > 1.f;
  };
  auto half_greater_1 = compose(greater_1, half);
  static_assert(traits::Equal<bool, decltype(half_greater_1(3))>::value, "");

  ASSERT_TRUE(half_greater_1(3));
  ASSERT_FALSE(half_greater_1(1));
}

TEST(FunctionalCompose, Void)
{
  using namespace qi;
  using namespace qi::traits;
  const int uninitialized = std::numeric_limits<int>::max();
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
  static_assert(Equal<void, decltype(k(3))>::value, "");

  ASSERT_EQ(uninitialized, fOrder);
  ASSERT_EQ(uninitialized, gOrder);
  k(3);
  ASSERT_EQ(0, fOrder);
  ASSERT_EQ(1, gOrder);
}

TEST(FunctionalCompose, Multi)
{
  using namespace qi;
  using namespace qi::traits;
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

  auto f = compose(str, compose(greater_1, half));
  static_assert(Equal<string, decltype(f(3))>::value, "");

  ASSERT_EQ("true", f(3));
  ASSERT_EQ("false", f(1));
}

namespace func_operators
{
  template<typename G, typename F>
  qi::Composition<qi::traits::Decay<G>, qi::traits::Decay<F>> operator*(G&& g, F&& f)
  {
    return {g, f};
  }
} // namespace

TEST(FunctionalCompose, Associative)
{
  using namespace qi;
  using std::string;
  using namespace func_operators;

  auto f = [](int x) {
    return x / 2.f;
  };
  auto g = [](float x) {
    return x > 1.f;
  };
  auto h = [](bool x) -> string {
    return x ? "true" : "false";
  };
  auto i = [](const string& x) -> std::size_t {
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

TEST(FunctionalCompose, Id)
{
  using namespace qi;
  using std::string;
  using namespace func_operators;

  auto f = [](int x) {
    return x / 2.f;
  };
  auto g = [](float x) {
    return x > 1.f;
  };
  IdTransfo id;

  auto a = f * id;
  auto b = id * f;
  auto c = (g * f) * id;
  auto d = id * (g * f);

  ASSERT_EQ(a(3), b(3));
  ASSERT_EQ(c(3), d(3));

  ASSERT_EQ(a(0), b(0));
  ASSERT_EQ(c(0), d(0));
}

namespace
{
  struct X {
    bool b;
    bool operator==(X x) const {return b == x.b;}
  };

  template<typename T>
  struct ConstantUnit
  {
    T operator()() const
    {
      return T{};
    }
  };

  template<>
  struct ConstantUnit<qi::Future<void>>
  {
    qi::Future<void> operator()() const
    {
      return qi::Future<void>{nullptr};
    }
  };

  template<typename T>
  bool equal(const T& a, const T& b)
  {
    return a == b;
  }

  template<typename T>
  bool equal(const qi::Future<T>& a, const qi::Future<T>& b)
  {
    return a.value() == b.value();
  }
}

using types = testing::Types<
  X, qi::Future<bool>, boost::optional<bool>, std::list<bool>
>;

template<typename T>
struct FunctionalSemiLift0 : testing::Test
{
};

TYPED_TEST_CASE(FunctionalSemiLift0, types);

TYPED_TEST(FunctionalSemiLift0, NonVoidCodomain)
{
  using namespace qi;
  using T = TypeParam;

  auto positive = [](int i) {
    return i > 0;
  };
  auto unit = [](bool b) {
    return T{b};
  };
  auto f = semiLift(positive, unit);

  static_assert(traits::Equal<T, decltype(f(0))>::value, "");
  ASSERT_TRUE(equal(T{true}, f(1)));
  ASSERT_TRUE(equal(T{false}, f(-1)));
}

using void_types = testing::Types<
  X, qi::Future<void>, boost::optional<bool>, std::list<bool>
>;

template<typename T>
struct FunctionalSemiLift1 : testing::Test
{
};

TYPED_TEST_CASE(FunctionalSemiLift1, void_types);

TYPED_TEST(FunctionalSemiLift1, VoidCodomain)
{
  using namespace qi;
  using namespace qi::traits;
  using T = TypeParam;

  auto noop = [](int) {
  };
  ConstantUnit<T> unit;
  auto f = semiLift(noop, unit);

  static_assert(Equal<T, decltype(f(0))>::value, "");
  ASSERT_TRUE(equal(unit(), f(0)));
}

TYPED_TEST(FunctionalSemiLift1, VoidCodomainVoidDomain)
{
  using namespace qi;
  using namespace qi::traits;
  using T = TypeParam;

  auto noop = [] {
  };
  ConstantUnit<T> unit;
  auto f = semiLift(noop, unit);

  static_assert(Equal<T, decltype(f())>::value, "");
  ASSERT_TRUE(equal(unit(), f()));
}

TEST(FunctionalMoveAssign, Basic)
{
  using namespace qi;
  using MoveOnly = test::MoveOnly<int>;
  const int i = 3;
  MoveOnly original{i};
  MoveAssign<MoveOnly, MoveOnly> moveAssign{std::move(original)};
  MoveOnly x{i + 1};
  moveAssign(x); // x = std::move(original);
  ASSERT_EQ(i, *x);
}

TEST(FunctionalIncr, Regular)
{
  using namespace qi;
  using namespace qi::detail;
  Incr<int> incr;
  ASSERT_TRUE(isRegular({incr})); // only one possible value because no state
}

TEST(FunctionalIncr, Arithmetic)
{
  using namespace qi;
  {
    Incr<int> incr;
    int x = 0;
    incr(x);
    ASSERT_EQ(1, x);
  }
  {
    Incr<double> incr;
    double x = 0.0;
    incr(x);
    ASSERT_EQ(1.0, x);
  }
}

TEST(FunctionalIncr, InputIterator)
{
  using namespace qi;
  using namespace std;
  stringstream ss{"youpi les amis"};
  using I = istream_iterator<string>;
  I b(ss);
  Incr<I> incr;
  ASSERT_EQ("youpi", *b);
  incr(b);
  ASSERT_EQ("les", *b);
  incr(b);
  ASSERT_EQ("amis", *b);
}

TEST(FunctionalDecr, Regular)
{
  using namespace qi;
  using namespace qi::detail;
  Decr<int> decr;
  ASSERT_TRUE(isRegular({decr})); // only one possible value because no state
}

TEST(FunctionalDecr, Arithmetic)
{
  using namespace qi;
  {
    Decr<int> decr;
    int x = 1;
    decr(x);
    ASSERT_EQ(0, x);
  }
  {
    Decr<double> decr;
    double x = 1.0;
    decr(x);
    ASSERT_EQ(0.0, x);
  }
}

TEST(FunctionalDecr, BidirectionalIterator)
{
  using namespace qi;
  using namespace std;
  Decr<list<string>::iterator> decr;
  list<string> l{"youpi", "les", "amis"};
  auto b = end(l);
  decr(b);
  ASSERT_EQ("amis", *b);
  decr(b);
  ASSERT_EQ("les", *b);
  decr(b);
  ASSERT_EQ("youpi", *b);
}

TEST(FunctionalIncr, IsomorphicIntegral)
{
  using namespace qi;
  {
    Incr<int> incr;
    auto inv = retract(incr);
    int i = 0;
    incr(i);
    inv(i);
    ASSERT_EQ(0, i);
  }
  {
    Incr<int> incr;
    auto inv = retract(incr);
    int i = 0;
    inv(i);
    incr(i);
    ASSERT_EQ(0, i);
  }
}

TEST(FunctionalIncr, IsomorphicBidirectionalIterator)
{
  using namespace qi;
  using namespace std;
  Incr<list<string>::iterator> incr;
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

TEST(FunctionalDecr, IsomorphicIntegral)
{
  using namespace qi;
  {
    Decr<int> decr;
    auto inv = retract(decr);
    int i = 0;
    decr(i);
    inv(i);
    ASSERT_EQ(0, i);
  }
  {
    Decr<int> decr;
    auto inv = retract(decr);
    int i = 0;
    inv(i);
    decr(i);
    ASSERT_EQ(0, i);
  }
}

TEST(FunctionalDecr, IsomorphicBidirectionalIterator)
{
  using namespace qi;
  using namespace std;
  Decr<list<string>::iterator> decr;
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

TEST(FunctionalApply, Tuple)
{
  using namespace qi;
  auto g = [](int i, char c, float f) {
    return std::make_tuple(i, c, f);
  };
  const auto args = std::make_tuple(5, 'a', 3.14f);
  ASSERT_EQ(args, apply(g, args));
  ASSERT_EQ(args, apply(g)(args));
}

TEST(FunctionalApply, Pair)
{
  using namespace qi;
  auto g = [](int i, char c) {
    return std::make_pair(i, c);
  };
  const auto args = std::make_pair(5, 'a');
  ASSERT_EQ(args, apply(g, args));
  ASSERT_EQ(args, apply(g)(args));
}

TEST(FunctionalApply, Array)
{
  using namespace qi;
  auto g = [](int i, int j, int k, int l) {
    return std::array<int, 4>{i, j, k, l};
  };
  const std::array<int, 4> args = {0, 1, 2, 3};
  ASSERT_EQ(args, apply(g, args));
  ASSERT_EQ(args, apply(g)(args));
}

TEST(FunctionalApply, Custom)
{
  using namespace qi;
  using X = test::X<int, char, float>;
  auto g = [](int i, char c, float f) {
    return X{i, c, f};
  };
  const X args{5, 'a', 3.14f};
  ASSERT_EQ(args, apply(g, args));
  ASSERT_EQ(args, apply(g)(args));
}

TEST(FunctionalApply, MoveOnly)
{
  using namespace qi;
  using test::MoveOnly;
  auto g = [](MoveOnly<int> i, MoveOnly<char> c, MoveOnly<float> f) {
    return std::make_tuple(*i, *c, *f);
  };
  const auto res = std::make_tuple(5, 'a', 3.14f);
  {
    auto args = std::make_tuple(MoveOnly<int>{5}, MoveOnly<char>{'a'}, MoveOnly<float>{3.14f});
    ASSERT_EQ(res, apply(g, std::move(args)));
  }
  {
    auto args = std::make_tuple(MoveOnly<int>{5}, MoveOnly<char>{'a'}, MoveOnly<float>{3.14f});
    ASSERT_EQ(res, apply(g)(std::move(args)));
  }
}

struct TrivialScopeLockable
{
  bool success;
  friend bool scopelock(TrivialScopeLockable& x) { return x.success; }
};

struct StrictScopeLockable
{
  struct Lock
  {
    test::MoveOnly<bool*> locked;

    Lock(bool* l) : locked{ l } { **locked = true; }
    ~Lock() { **locked = false; }

    Lock(Lock&& o) : locked{std::move(o.locked)} {}
    Lock& operator=(Lock&& o) { locked = std::move(o.locked); return *this; }

    explicit operator bool() const { return true; }
  };

  friend Lock scopelock(StrictScopeLockable& l)
  {
    return Lock{ l.locked };
  }

  bool* locked;
};

TEST(FunctionalScopeLock, ReturnsVoidSuccess)
{
  using namespace qi;
  using L = TrivialScopeLockable;

  bool called = false;
  // TODO: pass by value instead of mutable store when source is available
  auto proc = scopeLockProc([&]{ called = true; }, makeMutableStore(L{ true }));
  proc();
  ASSERT_TRUE(called);
}

TEST(FunctionalScopeLock, ReturnsVoidFailure)
{
  using namespace qi;
  using L = TrivialScopeLockable;

  bool called = false;
  // TODO: pass by value instead of mutable store when source is available
  auto proc = scopeLockProc([&]{ called = true; }, makeMutableStore(L{ false }));
  proc();
  ASSERT_FALSE(called);
}

TEST(FunctionalScopeLock, ReturnsProcResultOnLockSuccess)
{
  using namespace qi;
  using L = TrivialScopeLockable;

  // TODO: pass by value instead of mutable store when source is available
  auto proc = scopeLockProc([](int i){ return i + 10; }, makeMutableStore(L{ true }));
  auto res = proc(5);
  ASSERT_TRUE(res);
  ASSERT_EQ(15, res.value());
}

TEST(FunctionalScopeLock, ReturnsEmptyOptionalOnLockFailure)
{
  using namespace qi;
  using L = TrivialScopeLockable;

  // TODO: pass by value instead of mutable store when source is available
  auto proc = scopeLockProc([](int i){ return i + 10; }, makeMutableStore(L{ false }));
  auto res = proc(12);
  ASSERT_FALSE(res);
}

TEST(FunctionalScopeLock, StaysLockedUntilProcIsFinished)
{
  using namespace qi;
  using L = StrictScopeLockable;

  bool locked = false;
  // TODO: pass by value instead of mutable store when source is available
  auto proc = scopeLockProc(
      [&] {
        if (!locked)
          throw std::runtime_error("was not locked");
      },
      makeMutableStore(L{ &locked }));
  ASSERT_NO_THROW(proc());
  ASSERT_FALSE(locked);
}

using SharedPtrTypes = testing::Types<std::shared_ptr<int>,
                                      boost::shared_ptr<int>>;

template<typename T>
struct FunctionalScopeLockWeakPtr : testing::Test
{
};

TYPED_TEST_CASE(FunctionalScopeLockWeakPtr, SharedPtrTypes);

TYPED_TEST(FunctionalScopeLockWeakPtr, SuccessfulLock)
{
  using namespace qi;
  using ShPtr = TypeParam;

  ShPtr shptr{ new int{ 42 } };
  auto wkptr = weakPtr(shptr);
  auto l = scopelock(wkptr);
  ASSERT_TRUE(l);
  ASSERT_EQ(2, l.use_count());
  ASSERT_EQ(shptr.get(), l.get());
}

TYPED_TEST(FunctionalScopeLockWeakPtr, FailureExpired)
{
  using namespace qi;
  using ShPtr = TypeParam;

  ShPtr shptr;
  auto wkptr = weakPtr(shptr);
  auto l = scopelock(wkptr);
  ASSERT_FALSE(l);
}

TEST(FunctionalScopeLock, AtomicFlag)
{
  using namespace qi;

  std::atomic_flag atomic = ATOMIC_FLAG_INIT;
  auto l = scopelock(atomic);
  ASSERT_TRUE(l);
  ASSERT_TRUE(atomic.test_and_set());
}

TEST(FunctionalScopeLock, AtomicFlagAlreadySet)
{
  using namespace qi;

  std::atomic_flag atomic = ATOMIC_FLAG_INIT;
  atomic.test_and_set();
  auto l = scopelock(atomic);
  ASSERT_FALSE(l);
}

namespace
{
  template<typename M>
  bool isLocked(M& m)
  {
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
struct FunctionalScopeLockMutexes : testing::Test
{
};

TYPED_TEST_CASE(FunctionalScopeLockMutexes, MutexTypes);

TYPED_TEST(FunctionalScopeLockMutexes, Mutexes)
{
  using namespace qi;
  using Mutex = TypeParam;

  Mutex m;
  ASSERT_FALSE(isLocked(m));
  {
    auto l = scopelock(m);
    ASSERT_TRUE(l);
    ASSERT_TRUE(isLocked(m));
  }
  ASSERT_FALSE(isLocked(m));
}

TEST(FunctionalScopeLockTransfo, Basic)
{
  using namespace qi;

  std::mutex m;
  int count = 0;
  const auto syncTransfo = scopeLockTransfo(&m);
  std::vector<Future<void>> futs;
  for (int i = 0; i < 10; ++i)
  {
    futs.push_back(async(syncTransfo([&]{
      if (!isLocked(m))
        throw std::runtime_error("Mutex was not locked");
      ++count;
    })));
  }

  for (const auto& fut : futs)
    ASSERT_EQ(FutureState_FinishedWithValue, fut.wait()) << fut.error();
  ASSERT_EQ(10, count);
}
