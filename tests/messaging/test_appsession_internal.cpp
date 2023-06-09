/*
**  Copyright (C) 2019 SoftBank Robotics Europe
**  See COPYING for the license
*/

#include <ka/scoped.hpp>
#include <qi/os.hpp>
#include <src/messaging/applicationsession_internal.hpp>
#include <boost/algorithm/string/join.hpp>
#include <boost/range/adaptor/transformed.hpp>
#include <boost/optional/optional_io.hpp>
#include <gmock/gmock.h>

using namespace qi;
using namespace qi::appsession_internal;

namespace qi
{

  void PrintTo(const ApplicationSession::Config& conf, std::ostream* os)
  {
    *os << "{ standalone=" << std::boolalpha << conf.standalone() << std::noboolalpha << ", "
        << "connectUrl=";
    const auto& connectUrl = conf.connectUrl();
    if (!connectUrl)
      *os << "<empty>";
    else
      *os << connectUrl->str();
    *os << ", listenUrls={" << urlVecToString(conf.listenUrls(), ", ") << "} }";
  }

  void PrintTo(const UrlVector& url, std::ostream* os)
  {
    const auto urlAsStr =
      boost::join(boost::adaptors::transform(url, [](const Url& url) { return url.str(); }), ", ");
    *os << '{' << urlAsStr << '}';
  }

}

using AppSessionUrlVecToString = testing::Test;

TEST_F(AppSessionUrlVecToString, EmptyListReturnsEmptyAddresses)
{
  const auto res = urlVecToString({});
  EXPECT_TRUE(res.empty()) << res;
}

TEST_F(AppSessionUrlVecToString, UrlAreJoined)
{
  const auto res = urlVecToString({ "abc", ":2312", "tcps://", "tcp://alice:21678" }, "<>");
  EXPECT_EQ("abc<>:2312<>tcps://<>tcp://alice:21678", res);
}

using AppSessionStringToUrlVec = testing::Test;

TEST_F(AppSessionStringToUrlVec, UnsetAddressesReturnsEmptyUrlList)
{
  const auto res = stringToUrlVec({});
  EXPECT_TRUE(res.empty());
}

TEST_F(AppSessionStringToUrlVec, EmptyAddressesReturnsOneUrl)
{
  const auto res = stringToUrlVec(std::string());
  EXPECT_EQ(UrlVector{ Url() }, res);
}

TEST_F(AppSessionStringToUrlVec, AddressesAreSplit)
{
  const auto res = stringToUrlVec(std::string("abc|:21783|tcp://|tcp://francois:80"), '|');
  const UrlVector expected{ Url("abc"), Url(":21783"), Url("tcp://"), Url("tcp://francois:80") };
  EXPECT_EQ(expected, res);
}

namespace
{
  const Url configConnectUrl = "tcp://alice:2313";
  const UrlVector configListenUrl { "tcp://bob:5894", "tcps://carol:38659"};

  Config baseConfig(bool standalone = false)
  {
    return Config{}
      .setStandalone(standalone)
      .setConnectUrl(configConnectUrl)
      .setListenUrls(configListenUrl);
  }

  const Url paramConnectUrl = "tcp://william:7486";
  const UrlVector paramListenUrl = { "tcps://eve:896", "tcp://zaya:12786", "tcp://john:1567" };
}

using AppSessionReconfigureWithUrl = testing::Test;

TEST_F(AppSessionReconfigureWithUrl, EmptyConfigEmptyParamsReturnsHardcodedValues)
{
  const auto expected = Config{}
                    .setConnectUrl(SessionConfig::defaultConnectUrl())
                    .setListenUrls({ SessionConfig::defaultListenUrl() });
  const auto actual = reconfigureWithUrl(Config{}, {}, {});
  EXPECT_EQ(expected.connectUrl(), actual.connectUrl());
  EXPECT_EQ(expected.listenUrls(), actual.listenUrls());
}

TEST_F(AppSessionReconfigureWithUrl, ConfigWithEmptyParamsIsUntouched)
{
  const auto config = baseConfig();
  const auto expected = config;
  const auto actual = reconfigureWithUrl(config, {}, {});

  EXPECT_EQ(expected.connectUrl(), actual.connectUrl());
  EXPECT_EQ(expected.listenUrls(), actual.listenUrls());
}

TEST_F(AppSessionReconfigureWithUrl, EmptyConfigWithParamsReturnsParams)
{
  const auto expected = Config{}.setConnectUrl(paramConnectUrl).setListenUrls(paramListenUrl);
  const auto actual = reconfigureWithUrl(Config{}, paramConnectUrl, paramListenUrl);

  EXPECT_EQ(expected.connectUrl(), actual.connectUrl());
  EXPECT_EQ(expected.listenUrls(), actual.listenUrls());
}

TEST_F(AppSessionReconfigureWithUrl, ConfigWithParamsIsOverwritten)
{
  const auto expected = Config{}.setConnectUrl(paramConnectUrl).setListenUrls(paramListenUrl);
  const auto actual = reconfigureWithUrl(baseConfig(), paramConnectUrl, paramListenUrl);

  EXPECT_EQ(expected.connectUrl(), actual.connectUrl());
  EXPECT_EQ(expected.listenUrls(), actual.listenUrls());
}

TEST_F(AppSessionReconfigureWithUrl, IncompleteConfigWithEmptyParamsIsCompleted)
{
  const auto config = Config{}.setConnectUrl("tcp://").setListenUrls(
    { ":32412", "qwiuh", "tcps://", "tcp://12.13.14.15:8273" });
  const auto result = reconfigureWithUrl(config, {}, {});

  const auto connectURL = result.connectUrl();
  ASSERT_TRUE(connectURL);
  EXPECT_TRUE(connectURL->isValid()) << *connectURL;

  const auto listenURLs = result.listenUrls();
  ASSERT_EQ(4u, listenURLs.size());
  EXPECT_THAT(listenURLs, testing::Each(testing::Property(&Url::isValid, true)));
}

TEST_F(AppSessionReconfigureWithUrl, EmptyConfigWithIncompleteParamsCompletesThem)
{
  const auto result =
    reconfigureWithUrl(Config{}, Url("abracadabra"), stringToUrlVec(std::string("galileo;tcp;:321")));

  const auto connectURL = result.connectUrl();
  ASSERT_TRUE(connectURL);
  EXPECT_TRUE(connectURL->isValid()) << *connectURL;

  const auto listenURLs = result.listenUrls();
  ASSERT_EQ(3u, listenURLs.size());
  EXPECT_THAT(listenURLs, testing::Each(testing::Property(&Url::isValid, true)));
}

TEST_F(AppSessionReconfigureWithUrl, ConfigWithIncompleteParamsCompletesThemFromConfig)
{
  const auto base =
    Config{}
      .setConnectUrl("tcp://default:12345")
      .setListenUrls({ "tcp://default-listen:54321", "tcps://other-default-listen:54312" });
  const auto result =
    reconfigureWithUrl(base, Url("tcp://robert"), stringToUrlVec(std::string("jessica:321;john")));

  const auto connectURL = result.connectUrl();
  ASSERT_TRUE(connectURL);
  EXPECT_EQ("tcp://robert:12345", connectURL->str());

  // Listen URLs are filled with the first listen URL from the config if multiple exist.
  UrlVector expected { "tcp://jessica:321", "tcp://john:54321" };
  EXPECT_EQ(expected, result.listenUrls());
}

namespace
{
  const Url envConnectUrl = "tcps://david:7682";
  const UrlVector envListenUrl { "tcps://eve:896", "tcps://frank:9374" };

  struct Env
  {
    Env()
      : _oldQiUrl(qi::os::getenv(qiUrlEnvVar))
      , _oldQiListenUrl(qi::os::getenv(qiListenUrlEnvVar))
    {
      qi::os::setenv(qiUrlEnvVar, envConnectUrl.str().c_str());
      qi::os::setenv(qiListenUrlEnvVar, urlVecToString(envListenUrl).c_str());
    }

    Env(const Env&) = delete;
    Env& operator=(const Env&) = delete;

    static Config config()
    {
      return Config{}.setConnectUrl(envConnectUrl).setListenUrls(envListenUrl);
    }

    ~Env()
    {
      qi::os::setenv(qiUrlEnvVar, _oldQiUrl.c_str());
      qi::os::setenv(qiListenUrlEnvVar, _oldQiListenUrl.c_str());
    }


  private:
    std::string _oldQiUrl;
    std::string _oldQiListenUrl;
  };

  const Url argsConnectUrl = "tcp://heidi:7354";
  const UrlVector argsListenUrl { "tcps://ivan:12135", "tcp://judy:43486", "tcps://mallory:3246"};

  std::vector<std::string> makeArgs(bool standalone = false)
  {
    std::vector<std::string> args{
      "--qi-url", argsConnectUrl.str(),
      "--qi-listen-url", urlVecToString(argsListenUrl),
    };
    if (standalone)
      args.push_back("--qi-standalone");
    return args;
  }

  Config argsConfig(bool standalone = false)
  {
    return Config{}
      .setStandalone(standalone)
      .setConnectUrl(argsConnectUrl)
      .setListenUrls(argsListenUrl);
  }
}

using AppSessionReconfigureWithProgOpts = testing::Test;

TEST_F(AppSessionReconfigureWithProgOpts, NoParametersUsesHardcodedUrls)
{
  const ProgramOptions opts(std::vector<std::string>{});

  const auto expected = Config{}
                          .setConnectUrl(SessionConfig::defaultConnectUrl())
                          .setListenUrls({ SessionConfig::defaultListenUrl() });
  const auto actual = reconfigureWithProgramOptions(Config{}, opts);

  EXPECT_EQ(expected.connectUrl(), actual.connectUrl());
  EXPECT_EQ(expected.listenUrls(), actual.listenUrls());
}

TEST_F(AppSessionReconfigureWithProgOpts, ConfigHasPriorityOverHardcoded)
{
  const ProgramOptions opts(std::vector<std::string>{});

  const auto config = baseConfig();
  const auto expected = config;
  const auto actual = reconfigureWithProgramOptions(config, opts);

  EXPECT_EQ(expected.connectUrl(), actual.connectUrl());
  EXPECT_EQ(expected.listenUrls(), actual.listenUrls());
}

TEST_F(AppSessionReconfigureWithProgOpts, EnvHasPriorityOverConfig)
{
  Env env;
  const ProgramOptions opts(std::vector<std::string>{});

  const auto expected = env.config();
  const auto actual = reconfigureWithProgramOptions(baseConfig(), opts);

  EXPECT_EQ(expected.connectUrl(), actual.connectUrl());
  EXPECT_EQ(expected.listenUrls(), actual.listenUrls());
}

TEST_F(AppSessionReconfigureWithProgOpts, ArgsHasPriorityOverAll)
{
  Env env;
  const ProgramOptions opts(makeArgs());

  const auto expected = argsConfig();
  const auto actual = reconfigureWithProgramOptions(baseConfig(), opts);

  EXPECT_EQ(expected.connectUrl(), actual.connectUrl());
  EXPECT_EQ(expected.listenUrls(), actual.listenUrls());
}
