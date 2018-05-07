#include <gtest/gtest.h>
#include <ka/moveoncopy.hpp>
#include <ka/mutablestore.hpp>
#include <ka/testutils.hpp>
#include <qi/async.hpp>
#include <qi/future.hpp>

namespace test
{
  enum class RefKind
  {
    LValue,
    RValue
  };

  RefKind refkind(ka::move_only_t<int>&&)
  {
    return RefKind::RValue;
  }

  RefKind refkind(ka::move_only_t<int>&)
  {
    return RefKind::LValue;
  }

  RefKind refkind(ka::move_aware_t&&)
  {
    return RefKind::RValue;
  }

  RefKind refkind(ka::move_aware_t&)
  {
    return RefKind::LValue;
  }

  template<typename T>
  qi::Future<RefKind> async_refkind(T&& t)
  {
    using namespace ka;
    // If `T&&` is an Rvalue, t is moved inside the `MutableStore`.
    //  Then, `MoveOnCopy` moves `t` inside the lambda.
    //
    // If `T&&` is an Lvalue, the addresse t is put inside the `MutableStore`.
    //  Then, `MoveOnCopy` copies `t`'s address into the lambda.
    auto m = move_on_copy(mutable_store_fwd(std::forward<T>(t)));
    return qi::async([=]() mutable {
      // Unwrap the value on two levels (MoveOnCopy and MutableStore) and
      // perfect-forward it.
      // `refkind` is overloaded to distinguish an Rvalue from an Lvalue.
      return refkind(fwd<T>(**m));
    });
  }
} // namespace test

static qi::MilliSeconds defaultTimeout{500};

TEST(MutableStore, MakeMutableMoveOnCopyRValue)
{
  using namespace qi;
  using namespace test;
  auto fut = async_refkind(ka::move_only_t<int>{8763}); // Rvalue
  ASSERT_EQ(FutureState_FinishedWithValue, fut.waitFor(defaultTimeout));
  // The Rvalue version must be called.
  ASSERT_EQ(RefKind::RValue, fut.value());
}

TEST(MutableStore, MakeMutableMoveOnCopyLValue)
{
  using namespace qi;
  using namespace test;
  ka::move_only_t<int> m{8763};
  auto fut = async_refkind(std::ref(m));
  ASSERT_EQ(FutureState_FinishedWithValue, fut.waitFor(defaultTimeout));
  // The Lvalue version must be called: it increments the value.
  ASSERT_EQ(RefKind::LValue, fut.value());
}

TEST(MutableStore, MakeMutableMoveOnCopyLValueNotMoved)
{
  using namespace qi;
  using namespace test;
  ka::move_aware_t m{8763};
  auto fut = async_refkind(std::ref(m));
  ASSERT_EQ(FutureState_FinishedWithValue, fut.waitFor(defaultTimeout));
  // The Rvalue version must be called.
  ASSERT_EQ(RefKind::LValue, fut.value());
  ASSERT_FALSE(m.moved);
}
