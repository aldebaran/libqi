#include <memory>
#include <utility>
#include <gtest/gtest.h>
#include <ka/conceptpredicate.hpp>
#include <ka/functional.hpp>
#include <ka/moveoncopy.hpp>
#include <ka/mutablestore.hpp>
#include <ka/range.hpp>
#include <ka/src.hpp>
#include <ka/testutils.hpp>

namespace ka {
  template<typename T>
  struct reference_wrapper_t {
    std::reference_wrapper<T> ref;

    reference_wrapper_t(T& t) BOOST_NOEXCEPT
      : ref(t) {
    }
    reference_wrapper_t(T&&) = delete;

    reference_wrapper_t(const reference_wrapper_t&) = default;
    reference_wrapper_t& operator=(const reference_wrapper_t&) = default;

    // TODO: Use default version when get rid of VS2013.
    reference_wrapper_t(reference_wrapper_t&& x)
      : ref(std::move(x.ref)) {
    }

    // TODO: Use default version when get rid of VS2013.
    reference_wrapper_t& operator=(reference_wrapper_t&& x) {
      ref = std::move(x.ref);
      return *this;
    }

    T& get() const BOOST_NOEXCEPT {
      return ref.get();
    }

    operator T&() const BOOST_NOEXCEPT {
      return get();
    }

    template<typename... Args>
    ResultOf<T& (Args&&...)> operator()(Args&&... args) const
      BOOST_NOEXCEPT_IF(BOOST_NOEXCEPT_EXPR(ref(std::forward<Args>(args)...))) {
      return ref(std::forward<Args>(args)...);
    }

  // Mutable<T>:
    T& operator*() const BOOST_NOEXCEPT {
      return get();
    }
  };

  template<typename T>
  reference_wrapper_t<T> ref(T& t) BOOST_NOEXCEPT {
    return {t};
  }
} // namespace ka

TEST(MutableStore, Direct) {
  using namespace ka;
  const int i = 0xBABA;
  mutable_store_t<int, int*> m{i};
  ASSERT_EQ(i, *m);
}

TEST(MutableStoreIndirect, NakedPtr) {
  using namespace ka;
  const int gits = 2501;
  std::unique_ptr<int> p{new int{gits}};
  mutable_store_t<int, int*> m{p.get()};
  ASSERT_EQ(gits, *m);
}

#define NOT_VS2013 (BOOST_COMP_MSVC == 0 || BOOST_COMP_MSVC >= BOOST_VERSION_NUMBER(19, 0, 0))

// Because of a bug in boost::variant that constructs ReferenceWrapper by default,
// we deactivate this test for VS2013.
#if NOT_VS2013
TEST(MutableStoreIndirect, ReferenceWrapper) {
  using namespace ka;
  const int i = 0xBEEF;
  int j = i;
  auto r = ref(j);
  mutable_store_t<int, reference_wrapper_t<int>> m{r};
  ASSERT_EQ(i, *m);
}
#endif

TEST(MutableStoreIndirect, MoveOnly) {
  using namespace ka;
  const int i = 0xFEED;
  using P = std::unique_ptr<int>;
  mutable_store_t<int, P> m{P(new int(i))};
  ASSERT_EQ(i, *m);
}

TEST(MutableStoreRegular, Direct) {
  using namespace ka;
  using namespace functional_ops;
  using M = mutable_store_t<int, int*>;
  ASSERT_TRUE(is_regular(bounded_range(M{0}, M{10}, incr_t{} * src_t{})));
}

TEST(MutableStoreRegular, Indirect) {
  using namespace ka;
  const std::size_t N = 10;
  std::array<std::unique_ptr<int>, N> ptrs;
  std::array<mutable_store_t<int, int*>, N> ms;
  int i = 0;
  for (auto& p: ptrs)
  {
    p.reset(new int(i));
    ms[i] = p.get();
    ++i;
  }
  ASSERT_TRUE(is_regular(bounded_range(ms)));
}

TEST(MutableStoreMutability, Direct) {
  using namespace ka;
  const int i = 87632;
  mutable_store_t<int, int*> m{i};
  ASSERT_EQ(i, *m);
  *m = i + 1;
  ASSERT_EQ(i + 1, *m);
}

TEST(MutableStoreMutability, DirectCopy) {
  using namespace ka;
  const int i = 87632;
  mutable_store_t<int, int*> m0{i};
  mutable_store_t<int, int*> m1{m0};
  ASSERT_EQ(i, *m0);
  ASSERT_EQ(i, *m1);
  *m0 = i + 1;
  ASSERT_EQ(i + 1, *m0);
  ASSERT_EQ(i, *m1);
}

TEST(MutableStoreMutability, Indirect) {
  using namespace ka;
  const int i = 87632;
  using P = std::unique_ptr<int>;
  mutable_store_t<int, P> m{P(new int(i))};
  ASSERT_EQ(i, *m);
  *m = i + 1;
  ASSERT_EQ(i + 1, *m);
}

TEST(MutableStoreMutability, IndirectCopy) {
  using namespace ka;
  const int i = 87632;
  std::unique_ptr<int> p{new int(i)};
  mutable_store_t<int, int*> m0{p.get()};
  mutable_store_t<int, int*> m1{m0}; // copy of the internal pointer
  ASSERT_EQ(i, *m0);
  ASSERT_EQ(i, *m1);
  *m0 = i + 1;
  ASSERT_EQ(i + 1, *m0);
  ASSERT_EQ(i + 1, *m1);
}

TEST(MutableStore, MakeMutableAddress) {
  using namespace ka;
  const int i = 87363;
  int j = i;
  auto m = mutable_store(&j); // The mutable holds a pointer to `j`.
  ASSERT_EQ(i, *m);
  ++(*m);
  ASSERT_EQ(i + 1, *m);
  ASSERT_EQ(i + 1, j); // j's value has changed
}

TEST(MutableStore, MakeMutableValue) {
  using namespace ka;
  const int i = 87363;
  int j = i;
  auto m = mutable_store(j); // The mutable owns the value.
  ASSERT_EQ(i, *m);
  ++(*m);
  ASSERT_EQ(i + 1, *m);
  ASSERT_EQ(i, j); // j's value has not changed
}

TEST(MutableStore, MakeMutableValueMoveOnly) {
  using namespace ka;
  const int i = 87363;
  auto m = mutable_store(move_only_t<int>{i}); // The mutable owns the `MoveOnly`.
  ASSERT_EQ(i, **m);
  ++(**m);
  ASSERT_EQ(i + 1, **m);
}

TEST(MutableStore, Pointers) {
  using namespace ka;
  int i[2] = {0, 0};
  int* p[2] = {i + 0, i + 1};
  // We pass an int* _by value_.
  // Equivalent to passing p[0] _by value_.
  mutable_store_t<int*, int**> m{p[0]};
  ++(*m); // increment the copy of the pointer
  ASSERT_EQ(i + 1, *m);
  ASSERT_EQ(i + 0, p[0]); // the original pointer is untouched
}

TEST(MutableStore, PointersOfPointers) {
  using namespace ka;
  int i[2];
  int* p[2] = {i + 0, i + 1};
  // We pass an int** (a pointer on p[0]).
  // Equivalent to passing p[0] _by reference_.
  mutable_store_t<int*, int**> m{p + 0};
  ++(*m); // increment the "reference" to the pointer
  ASSERT_EQ(i + 1, *m);
  ASSERT_EQ(i + 1, p[0]); // the original pointer has been modified
}
