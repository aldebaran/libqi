#include <future>
#include <boost/thread/shared_mutex.hpp>
#include <gtest/gtest.h>
#include <ka/functional.hpp>
#include <ka/mutablestore.hpp>
#include <ka/mutex.hpp>
#include <ka/typetraits.hpp>
#include <ka/testutils.hpp>
#include <qi/atomic.hpp>
#include <qi/future.hpp>

namespace
{
  template<typename T>
  struct ConstantDefault
  {
    T operator()() const
    {
      return T{};
    }
  };

  template<>
  struct ConstantDefault<qi::Future<void>>
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
} // namespace

using types = testing::Types<
  qi::Future<bool>
>;

template<typename T>
struct FunctionalSemiLift0 : testing::Test
{
};

TYPED_TEST_CASE(FunctionalSemiLift0, types);

TYPED_TEST(FunctionalSemiLift0, NonVoidCodomain)
{
  using namespace qi;
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
  qi::Future<void>
>;

template<typename T>
struct FunctionalSemiLift1 : testing::Test
{
};

TYPED_TEST_CASE(FunctionalSemiLift1, void_types);

TYPED_TEST(FunctionalSemiLift1, VoidCodomain)
{
  using namespace ka;
  using T = TypeParam;

  auto noop = [](int) {
  };
  ConstantDefault<T> unit;
  auto f = semilift(noop, unit);

  static_assert(Equal<T, decltype(f(0))>::value, "");
  ASSERT_TRUE(equal(unit(), f(0)));
}

TYPED_TEST(FunctionalSemiLift1, VoidCodomainVoidDomain)
{
  using namespace ka;
  using T = TypeParam;

  auto noop = [] {
  };
  ConstantDefault<T> unit;
  auto f = semilift(noop, unit);

  static_assert(Equal<T, decltype(f())>::value, "");
  ASSERT_TRUE(equal(unit(), f()));
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

TEST(FunctionalScopeLockTransfo, Basic)
{
  using namespace qi;
  using namespace ka;

  std::mutex m;
  int count = 0;
  const auto syncTransfo = scope_lock_transfo(&m);
  std::vector<Future<opt_t<void>>> futs;
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
