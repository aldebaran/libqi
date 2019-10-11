#include <list>
#include <atomic>
#include <memory>
#include <vector>
#include <ka/macro.hpp>

KA_WARNING_DISABLE(, unused-function)

#include <boost/optional.hpp>
#include <boost/thread/synchronized_value.hpp>
#include <gtest/gtest.h>
#include <ka/conceptpredicate.hpp>
#include <ka/mutablestore.hpp>
#include <ka/src.hpp>
#include <ka/typetraits.hpp>
#include <ka/utility.hpp>

TEST(Src, ReadablePtr) {
  using namespace ka;
  const int i = 5;
  const int* p = &i;
  ASSERT_EQ(i, src(p));
  ASSERT_EQ(&i, &src(p)); // This is not part of the concept,  but is true for pointers.
}

TEST(Src, MutablePtr) {
  using namespace ka;
  const int i = 5;
  int j = i;
  int* p = &j;
  ASSERT_EQ(i, src(p));
  ++src(p);
  ASSERT_EQ(i + 1, src(p));
}

TEST(Src, ReadableVecIter) {
  using namespace ka;
  const int i = 5;
  const std::vector<int> v{i};
  ASSERT_EQ(i, src(begin(v)));
}

TEST(Src, MutableVecIter) {
  using namespace ka;
  const int i = 5;
  std::vector<int> v{i};
  auto b = begin(v);
  ASSERT_EQ(i, src(b));
  ++src(b);
  ASSERT_EQ(i + 1, src(b));
}

TEST(Src, ReadableInt) {
  using namespace ka;
  const int i = 5;
  ASSERT_EQ(i, src(i));
}

TEST(Src, MutableInt) {
  using namespace ka;
  const int i = 5;
  int j = i;
  ASSERT_EQ(i, src(j));
  ++j;
  ASSERT_EQ(i + 1, src(j));
}

namespace {
  struct with_op_star_t {
    int i;
    KA_GENERATE_FRIEND_REGULAR_OPS_1(with_op_star_t, i)
    int operator*() const {
      return i;
    }
  };
}

TEST(Src, OtherTypes) {
  using namespace ka;
  const int i = 5;
  {
    const std::atomic<int> x{i};
    ASSERT_EQ(i, src(x));
  } {
    const std::unique_ptr<int> x{new int{i}};
    ASSERT_EQ(i, src(x));
  } {
    const boost::optional<int> x{i};
    ASSERT_EQ(i, src(x));
  } {
    const boost::synchronized_value<int> x{i};
    const int j = src(x);
    ASSERT_EQ(i, j);
  } {
    with_op_star_t x{i};
    ASSERT_EQ(i, src(x));
  } {
    int i = 5;
    const mutable_store_t<int, int*> x0{i};
    ASSERT_EQ(i, src(x0));
    const mutable_store_t<int, int*> x1{&i};
    ASSERT_EQ(i, src(x1));
  }
}

namespace {
  struct without_op_star_t {
    int i;
    KA_GENERATE_FRIEND_REGULAR_OPS_1(without_op_star_t, i)
  };
} // namespace

TEST(Src, NoOpStar) {
  using namespace ka;
  const int i = 5;
  {
    std::vector<int> x{i};
    ASSERT_EQ(x, src(x));
  } {
    without_op_star_t x{i};
    ASSERT_EQ(x, src(x));
  }
}

TEST(Src, Polymorphic) {
  using namespace ka;

  const int i = 42;
  src_t src;

  std::unique_ptr<int> p{new int{i}};
  ASSERT_EQ(i, src(p));
  ++src(p);
  ASSERT_EQ(i + 1, src(p));

  ASSERT_EQ(i, src(i));
}
