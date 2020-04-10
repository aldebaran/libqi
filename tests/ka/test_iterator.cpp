#include <gtest/gtest.h>
#include <iterator>
#include <forward_list>
#include <list>
#include <vector>
#include <sstream>
#include <ka/iterator.hpp>
#include <ka/src.hpp>

TEST(Iterator, OperatorPlusInputIter) {
  using ka::src;
  using ka::iterator_ops::operator+;
  using I = std::istream_iterator<int>;
  {
    std::istringstream ss{"1 23 45"};
    ASSERT_EQ(1, src(I{ss}));
  }
  {
    std::istringstream ss{"1 23 45"};
    ASSERT_EQ(1, src(I{ss} + 0));
  }
  {
    std::istringstream ss{"1 23 45"};
    ASSERT_EQ(23, src(I{ss} + 1));
  }
  {
    std::istringstream ss{"1 23 45"};
    ASSERT_EQ(45, src(I{ss} + 2));
  }
}

TEST(Iterator, OperatorMinusBidirectionalIter) {
  using ka::src;
  using ka::iterator_ops::operator-;
  std::list<int> c{1, 23, 45};
  ASSERT_EQ(45, src(c.end() - 1));
  ASSERT_EQ(23, src(c.end() - 2));
  ASSERT_EQ(1, src(c.end() - 3));
}

TEST(Iterator, OperatorPlusEqualInputIter) {
  using ka::src;
  using ka::iterator_ops::operator+=;
  using I = std::istream_iterator<int>;
  {
    std::istringstream ss{"1 23 45"};
    ASSERT_EQ(1, src(I{ss}));
  }
  {
    std::istringstream ss{"1 23 45"};
    I x{ss};
    x += 0;
    ASSERT_EQ(1, src(x));
  }
  {
    std::istringstream ss{"1 23 45"};
    I x{ss};
    x += 1;
    ASSERT_EQ(23, src(x));
  }
  {
    std::istringstream ss{"1 23 45"};
    I x{ss};
    x += 2;
    ASSERT_EQ(45, src(x));
  }
}

TEST(Iterator, OperatorMinusEqualBidirectionalIter) {
  using ka::src;
  using ka::iterator_ops::operator-=;
  std::list<int> c{1, 23, 45};
  {
    auto x = c.end();
    x -= 1;
    ASSERT_EQ(45, src(x));
  }
  {
    auto x = c.end();
    x -= 2;
    ASSERT_EQ(23, src(x));
  }
  {
    auto x = c.end();
    x -= 3;
    ASSERT_EQ(1, src(x));
  }
}

TEST(Iterator, OperatorPlusForwardIter) {
  using ka::src;
  using ka::iterator_ops::operator+;
  std::forward_list<int> c{1, 23, 45};
  ASSERT_EQ(1, src(c.begin()));
  ASSERT_EQ(1, src(c.begin() + 0));
  ASSERT_EQ(23, src(c.begin() + 1));
  ASSERT_EQ(45, src(c.begin() + 2));
}

TEST(Iterator, OperatorPlusBidirectionalIter) {
  using ka::src;
  using ka::iterator_ops::operator+;
  std::list<int> c{1, 23, 45};
  ASSERT_EQ(1, src(c.begin()));
  ASSERT_EQ(1, src(c.begin() + 0));
  ASSERT_EQ(23, src(c.begin() + 1));
  ASSERT_EQ(45, src(c.begin() + 2));

  ASSERT_EQ(45, src(c.end() + (-1)));
  ASSERT_EQ(23, src(c.end() + (-2)));
  ASSERT_EQ(1, src(c.end() + (-3)));

  using ka::iterator_ops::operator-;
  ASSERT_EQ(45, src(c.end() - 1));
  ASSERT_EQ(23, src(c.end() - 2));
  ASSERT_EQ(1, src(c.end() - 3));
}

TEST(Iterator, OperatorMinusDistanceInputIterator) {
  using ka::src;
  using ka::iterator_ops::operator-;
  using I = std::istream_iterator<int>;

  std::istringstream ss{"1 23 45"};
  auto b = I{ss};
  auto e = I{};
  ASSERT_EQ(3, e - b);
}

TEST(Iterator, OperatorMinusDistanceForwardIterator) {
  using ka::src;
  using ka::iterator_ops::operator-;

  std::forward_list<int> c{1, 23, 45, 67};
  ASSERT_EQ(4, c.end() - c.begin());
}
