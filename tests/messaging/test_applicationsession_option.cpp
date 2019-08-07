#include <iostream>
#include <array>
#include <gtest/gtest.h>
#include <qi/applicationsession.hpp>
#include <boost/algorithm/string/join.hpp>

static char* gav = nullptr;

TEST(QiApplicationSessionOptions, testListenURLsOptions)
{
  const std::string defaultListenUrl("tcps://0.0.0.0:9559");
  const std::vector<std::string> urlsListen({"tcp://127.0.0.1:9558",
                                             "127.0.0.1:9559",
                                             "tcp://:12344",
                                             "tcp://127.0.0.1",
                                             "127.0.0.1",
                                             "tcp://",
                                             ":12345"});
  const std::vector<qi::Url> expectedListenUrls({"tcp://127.0.0.1:9558",
                                           "tcps://127.0.0.1:9559",
                                           "tcp://0.0.0.0:12344",
                                           "tcp://127.0.0.1:9559",
                                           "tcps://127.0.0.1:9559",
                                           "tcp://0.0.0.0:9559",
                                           "tcps://0.0.0.0:12345"});

  qi::ApplicationSession::Config config;
  config.setListenUrls({ defaultListenUrl });

  const auto urls = boost::join(urlsListen, ";");
  std::array<char*, 3> av {
    gav,
    const_cast<char*>("--qi-listen-url"),
    const_cast<char*>(urls.c_str()),
  };

  auto ac = static_cast<int>(av.size());
  char** refav = av.data();

  qi::ApplicationSession appsession(ac, refav, config);
  ASSERT_EQ(expectedListenUrls, appsession.allListenUrl());
}

int main(int ac, char **av)
{
  auto scopedGav = ka::scoped_set_and_restore(gav, av[0]);
  ::testing::InitGoogleTest(&ac, av);
  return RUN_ALL_TESTS();
}
