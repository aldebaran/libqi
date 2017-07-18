#include <memory>
#include <utility>
#include <gtest/gtest.h>
#include <qi/detail/conceptpredicate.hpp>
#include <qi/future.hpp>
#include <qi/moveoncopy.hpp>
#include <qi/mutablestore.hpp>
#include <qi/range.hpp>
#include "tools.hpp"

namespace qi
{
  template<typename T>
  struct ReferenceWrapper
  {
    std::reference_wrapper<T> ref;

    ReferenceWrapper(T& t) BOOST_NOEXCEPT
      : ref(t)
    {
    }
    ReferenceWrapper(T&&) = delete;

    ReferenceWrapper(const ReferenceWrapper&) = default;
    ReferenceWrapper& operator=(const ReferenceWrapper&) = default;

    // TODO: Use default version when get rid of VS2013.
    ReferenceWrapper(ReferenceWrapper&& x)
      : ref(std::move(x.ref))
    {
    }

    // TODO: Use default version when get rid of VS2013.
    ReferenceWrapper& operator=(ReferenceWrapper&& x)
    {
      ref = std::move(x.ref);
      return *this;
    }

    T& get() const BOOST_NOEXCEPT
    {
      return ref.get();
    }

    operator T&() const BOOST_NOEXCEPT
    {
      return get();
    }

    template<typename... Args>
    traits::ResultOf<T& (Args&&...)> operator()(Args&&... args) const
      BOOST_NOEXCEPT_IF(BOOST_NOEXCEPT_EXPR(ref(std::forward<Args>(args)...)))
    {
      return ref(std::forward<Args>(args)...);
    }

  // Mutable<T>:
    T& operator*() const BOOST_NOEXCEPT
    {
      return get();
    }
  };

  template<typename T>
  ReferenceWrapper<T> ref(T& t) BOOST_NOEXCEPT
  {
    return {t};
  }
} // namespace qi

static const qi::MilliSeconds defaultTimeout{1000};

TEST(MutableStore, Direct)
{
  using namespace qi;
  const int i = 0xBABA;
  MutableStore<int, int*> m{i};
  ASSERT_EQ(i, *m);
}

TEST(MutableStoreIndirect, NakedPtr)
{
  using namespace qi;
  const int gits = 2501;
  std::unique_ptr<int> p{new int{gits}};
  MutableStore<int, int*> m{p.get()};
  ASSERT_EQ(gits, *m);
}

#define NOT_VS2013 (BOOST_COMP_MSVC == 0 || BOOST_COMP_MSVC >= BOOST_VERSION_NUMBER(19, 0, 0))

// Because of a bug in boost::variant that constructs ReferenceWrapper by default,
// we deactivate this test for VS2013.
#if NOT_VS2013
TEST(MutableStoreIndirect, ReferenceWrapper)
{
  using namespace qi;
  const int i = 0xBEEF;
  int j = i;
  auto r = ref(j);
  MutableStore<int, ReferenceWrapper<int>> m{r};
  ASSERT_EQ(i, *m);
}
#endif

TEST(MutableStoreIndirect, MoveOnly)
{
  using namespace qi;
  const int i = 0xFEED;
  using P = std::unique_ptr<int>;
  MutableStore<int, P> m{P(new int(i))};
  ASSERT_EQ(i, *m);
}

TEST(MutableStoreRegular, Direct)
{
  using namespace qi;
  using M = MutableStore<int, int*>;
  auto incr = [](M& m) {++*m;};
  ASSERT_TRUE(detail::isRegular(incrRange(M{0}, M{10}, incr)));
}

TEST(MutableStoreRegular, Indirect)
{
  using namespace qi;
  const std::size_t N = 10;
  std::array<std::unique_ptr<int>, N> ptrs;
  std::array<MutableStore<int, int*>, N> ms;
  int i = 0;
  for (auto& p: ptrs)
  {
    p.reset(new int(i));
    ms[i] = p.get();
    ++i;
  }
  ASSERT_TRUE(detail::isRegular(boundedRange(ms)));
}

TEST(MutableStoreMutability, Direct)
{
  using namespace qi;
  const int i = 87632;
  MutableStore<int, int*> m{i};
  ASSERT_EQ(i, *m);
  *m = i + 1;
  ASSERT_EQ(i + 1, *m);
}

TEST(MutableStoreMutability, DirectCopy)
{
  using namespace qi;
  const int i = 87632;
  MutableStore<int, int*> m0{i};
  MutableStore<int, int*> m1{m0};
  ASSERT_EQ(i, *m0);
  ASSERT_EQ(i, *m1);
  *m0 = i + 1;
  ASSERT_EQ(i + 1, *m0);
  ASSERT_EQ(i, *m1);
}

TEST(MutableStoreMutability, Indirect)
{
  using namespace qi;
  const int i = 87632;
  using P = std::unique_ptr<int>;
  MutableStore<int, P> m{P(new int(i))};
  ASSERT_EQ(i, *m);
  *m = i + 1;
  ASSERT_EQ(i + 1, *m);
}

TEST(MutableStoreMutability, IndirectCopy)
{
  using namespace qi;
  const int i = 87632;
  std::unique_ptr<int> p{new int(i)};
  MutableStore<int, int*> m0{p.get()};
  MutableStore<int, int*> m1{m0}; // copy of the internal pointer
  ASSERT_EQ(i, *m0);
  ASSERT_EQ(i, *m1);
  *m0 = i + 1;
  ASSERT_EQ(i + 1, *m0);
  ASSERT_EQ(i + 1, *m1);
}

TEST(MutableStore, MakeMutableLValue)
{
  using namespace qi;
  const int i = 87363;
  int j = i;

  // We pass an lvalue, so the mutable holds a pointer to `j`,
  auto m = makeMutableStore(std::ref(j));
  ASSERT_EQ(i, *m);
  ++(*m);
  ASSERT_EQ(i + 1, *m);
  ASSERT_EQ(i + 1, j);
}

TEST(MutableStore, MakeMutableRValue)
{
  using namespace qi;
  using test::MoveOnly;
  const int i = 87363;

  // We pass an rvalue, so the mutable holds a pointer to `j`.
  auto m = makeMutableStore(MoveOnly<int>{i});
  ASSERT_EQ(i, **m);
  ++(**m);
  ASSERT_EQ(i + 1, **m);
}

namespace test
{
  RefKind refkind(MoveOnly<int>&&)
  {
    return RefKind::RValue;
  }

  RefKind refkind(MoveOnly<int>&)
  {
    return RefKind::LValue;
  }

  RefKind refkind(MoveAware&&)
  {
    return RefKind::RValue;
  }

  RefKind refkind(MoveAware&)
  {
    return RefKind::LValue;
  }

  template<typename T>
  qi::Future<RefKind> async_refkind(T&& t)
  {
    using namespace qi;
    // If `T&&` is an Rvalue, t is moved inside the `MutableStore`.
    //  Then, `MoveOnCopy` moves `t` inside the lambda.
    //
    // If `T&&` is an Lvalue, the addresse t is put inside the `MutableStore`.
    //  Then, `MoveOnCopy` copies `t`'s address into the lambda.
    auto m = qi::makeMoveOnCopy(qi::makeMutableStore(std::forward<T>(t)));
    return async([=]() mutable {
      // Unwrap the value on two levels (MoveOnCopy and MutableStore) and
      // perfect-forward it.
      // `refkind` is overloaded to distinguish an Rvalue from an Lvalue.
      return refkind(std::forward<T>(**m));
    });
  }
} // namespace test

TEST(MutableStore, MakeMutableMoveOnCopyRValue)
{
  using namespace qi;
  using namespace test;
  auto fut = async_refkind(MoveOnly<int>{8763}); // Rvalue
  ASSERT_EQ(FutureState_FinishedWithValue, fut.wait(defaultTimeout));
  // The Rvalue version must be called.
  ASSERT_EQ(RefKind::RValue, fut.value());
}

TEST(MutableStore, MakeMutableMoveOnCopyLValue)
{
  using namespace qi;
  using namespace test;
  MoveOnly<int> m{8763};
  auto fut = async_refkind(std::ref(m));
  ASSERT_EQ(FutureState_FinishedWithValue, fut.wait(defaultTimeout));
  // The Lvalue version must be called: it increments the value.
  ASSERT_EQ(RefKind::LValue, fut.value());
}

TEST(MutableStore, MakeMutableMoveOnCopyLValueNotMoved)
{
  using namespace qi;
  using namespace test;
  MoveAware m{8763};
  auto fut = async_refkind(std::ref(m));
  ASSERT_EQ(FutureState_FinishedWithValue, fut.wait(defaultTimeout));
  // The Rvalue version must be called.
  ASSERT_EQ(RefKind::LValue, fut.value());
  ASSERT_FALSE(m.moved);
}

TEST(MutableStore, Pointers)
{
  using namespace qi;
  int i[2] = {0, 0};
  int* p[2] = {i + 0, i + 1};
  // We pass an int* _by value_.
  // Equivalent to passing p[0] _by value_.
  MutableStore<int*, int**> m{p[0]};
  ++(*m); // increment the copy of the pointer
  ASSERT_EQ(i + 1, *m);
  ASSERT_EQ(i + 0, p[0]); // the original pointer is untouched
}

TEST(MutableStore, PointersOfPointers)
{
  using namespace qi;
  int i[2];
  int* p[2] = {i + 0, i + 1};
  // We pass an int** (a pointer on p[0]).
  // Equivalent to passing p[0] _by reference_.
  MutableStore<int*, int**> m{p + 0};
  ++(*m); // increment the "reference" to the pointer
  ASSERT_EQ(i + 1, *m);
  ASSERT_EQ(i + 1, p[0]); // the original pointer has been modified
}
