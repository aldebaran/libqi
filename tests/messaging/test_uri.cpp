#include <qi/uri.hpp>
#include <ka/relationpredicate.hpp>
#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <array>
#include <algorithm>

using namespace qi;

struct UriIsLoopbackAddressWithInput : testing::TestWithParam<std::string>{};

INSTANTIATE_TEST_SUITE_P(LoopbackHosts,
                        UriIsLoopbackAddressWithInput,
                        testing::Values(
                          "localhost",
                          "127.0.0.1",
                          "127.0.0.89",
                          "127.42.42.42",
                          "::1"
                        ));

TEST_P(UriIsLoopbackAddressWithInput, Success)
{
  EXPECT_TRUE(qi::isLoopbackAddress(GetParam()));
}

TEST(UriIsLoopbackAddress, FailsOnConfusingHost)
{
  EXPECT_FALSE(qi::isLoopbackAddress("127.foo"));
}

TEST(UriIsRelativeEndpoint, Success)
{
  const auto optUri = uri("qi:");
  ASSERT_FALSE(optUri.empty());
  EXPECT_TRUE(isRelativeEndpoint(*optUri));
}

TEST(UriIsRelativeEndpoint, FailureTcp)
{
  const auto optUri = uri("tcp:");
  ASSERT_FALSE(optUri.empty());
  EXPECT_FALSE(isRelativeEndpoint(*optUri));
}

TEST(UriIsRelativeEndpoint, FailureTcps)
{
  const auto optUri = uri("tcps:");
  ASSERT_FALSE(optUri.empty());
  EXPECT_FALSE(isRelativeEndpoint(*optUri));
}

TEST(UriIsRelativeEndpoint, FailureAuthority)
{
  const auto optUri = uri("qi://");
  ASSERT_FALSE(optUri.empty());
  EXPECT_FALSE(isRelativeEndpoint(*optUri));
}

namespace
{
  const auto noAuth                    = uri("tcp:");
  const auto auth                      = uri("tcp://10.0.1.23");
  const auto localhost                 = uri("tcp://localhost");
  const auto localhostv4               = uri("tcp://127.0.0.1");
  const auto loopback                  = uri("tcp://127.0.0.2");
  const auto qiCookies                 = uri("qi:cookies");
  const auto qiMuffins                 = uri("qi:muffins");
  const auto qiUpperMuffins            = uri("QI:muffins");
  const auto qiServiceDirectory        = uri("qi:ServiceDirectory");
  const auto qiServiceDirectoryRelPath = uri("qi:./ServiceDirectory");

  qi::Uri const uris[] = {
    *noAuth,
    *auth,
    *localhost,
    *localhostv4,
    *loopback,
    *qiCookies,
    *qiMuffins,
    *qiUpperMuffins,
    *qiServiceDirectory,
    *qiServiceDirectoryRelPath
  };
}

TEST(UriIsPreferredEndpoint, TotalOrdering)
{
  using namespace qi;
  using namespace testing;
  EXPECT_TRUE(ka::is_total_ordering(isPreferredEndpoint, ka::bounded_range(uris)));
}

TEST(UriIsPreferredEndpoint, OrderIsRespected)
{
  using namespace qi;
  using namespace testing;

  qi::Uri expected[] = {
    *qiServiceDirectory,
    *qiServiceDirectoryRelPath,
    *qiCookies,
    *qiMuffins,
    *qiUpperMuffins,
    *localhostv4,
    *loopback,
    *localhost, // when lexicographical ordered, digits come before alphabetic symbols.
    *auth,
    *noAuth,
  };

  EXPECT_THAT(uris, WhenSortedBy(&isPreferredEndpoint, ElementsAreArray(expected)));
}

TEST(Uri, Projections)
{
  using namespace qi;
  const auto u = *uri("https://admin:passwd@softbankrobotics.com:443/logs?level=error#message");

  EXPECT_EQ("https", scheme(u));

  const auto oa = authority(u);
  ASSERT_FALSE(oa.empty());
  const auto a = *oa;

  EXPECT_EQ("softbankrobotics.com", host(a));
  EXPECT_EQ(ka::opt<std::uint16_t>(443), port(a));

  const auto oui = userinfo(a);
  ASSERT_FALSE(oui.empty());
  const auto ui = *oui;

  EXPECT_EQ("admin", username(ui));
  EXPECT_EQ(ka::opt<std::string>("passwd"), password(ui));

  EXPECT_EQ("/logs", uriPath(u));
  EXPECT_EQ(ka::opt<std::string>("level=error"), query(u));
  EXPECT_EQ(ka::opt<std::string>("message"), fragment(u));
}
