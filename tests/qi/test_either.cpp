#include <string>
#include <gtest/gtest.h>
#include <ka/conceptpredicate.hpp>
#include <qi/either.hpp>

TEST(Either, Regular)
{
  using E = qi::Either<std::string, int>;
  ASSERT_TRUE(ka::is_regular({E{"a"}, E{0}, E{"bc"}, E{12}}));
}

namespace
{
  struct Norm : qi::VisitorBase<std::size_t>
  {
    std::size_t operator()(const std::string& x) const
    {
      return x.size();
    }
    std::size_t operator()(int i) const
    {
      return i;
    }
  };
} // namespace

TEST(Either, Visit)
{
  using namespace qi;
  using E = Either<std::string, int>;
  {
    E e{"abc"};
    ASSERT_EQ(3u, visit(Norm{}, e));
  }
  {
    E e{12};
    ASSERT_EQ(12u, visit(Norm{}, e));
  }
}
