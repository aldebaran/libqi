#include "uri_common.hpp"
#include <ka/uri/uri.hpp>
#include <ka/uri/io.hpp>
#include <ka/conceptpredicate.hpp>
#include <ka/range.hpp>
#include <boost/range/size.hpp>
#include <gtest/gtest.h>

struct Uri : testing::Test {};

TEST(Uri, IsRegular) {
  using namespace ka;
  ASSERT_TRUE(is_regular(bounded_range(test::uri_list)));
}

struct UriConstructionAtIndex : testing::TestWithParam<std::size_t> {};

INSTANTIATE_TEST_SUITE_P(GoodUriIndex,
                        UriConstructionAtIndex,
                        testing::Range(std::size_t{}, boost::size(ka::test::uri_strings)));

TEST_P(UriConstructionAtIndex, GoodUriParse) {
  using namespace ka;
  auto const param = GetParam();
  auto const& input = test::uri_strings[param];
  auto const& expected = test::uri_list[param];
  auto const actual = uri(input.begin(), input.end());
  ASSERT_FALSE(actual.empty());
  EXPECT_EQ(expected, *actual);
}

struct UriConstructionWithInput : testing::TestWithParam<std::string> {};

INSTANTIATE_TEST_SUITE_P(BadInput,
                        UriConstructionWithInput,
                        testing::ValuesIn(ka::test::bad_uri_list()));

TEST_P(UriConstructionWithInput, Parse) {
  using namespace ka;
  auto const input = GetParam();
  auto const result = uri(input.begin(), input.end());
  EXPECT_TRUE(result.empty());
}

TEST(UriConstruction, AuthorityIsFollowedByAbsoluteOrEmptyPath) {
  using namespace ka;
  std::string const input = "s://a:1234abc";
  auto const result = uri(std::begin(input), std::end(input));
  // Parsing still succeeds but stops after port.
  ASSERT_FALSE(result.empty());
  auto const uri = *result;
  EXPECT_EQ("s", uri.scheme());
  ASSERT_FALSE(uri.authority().empty());
  auto const auth = (*uri.authority());
  EXPECT_EQ("a", auth.host());
  ASSERT_FALSE(auth.port().empty());
  EXPECT_EQ(1234, *auth.port());
  EXPECT_TRUE(uri.path().empty());
  EXPECT_EQ(std::begin(input) + 10, iter(result));
}

struct UriAtIndex : testing::TestWithParam<std::size_t> {};

INSTANTIATE_TEST_SUITE_P(WithGoodUriIndex,
                        UriAtIndex,
                        testing::Range(std::size_t{}, boost::size(ka::test::uri_strings)));

TEST_P(UriAtIndex, ToString) {
  using namespace ka;
  auto const param = GetParam();
  auto const& input = test::uri_list[param];
  auto const& expected = ka::test::uri_strings[param];
  EXPECT_EQ(expected, to_string(input));
}
