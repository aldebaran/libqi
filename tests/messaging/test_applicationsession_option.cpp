#include <iostream>
#include <gtest/gtest.h>
#include <qi/applicationsession.hpp>

static char* gav[] = {NULL};

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
  const std::vector<std::string> expectedListen({"tcp://127.0.0.1:9558",
                                                 "tcps://127.0.0.1:9559",
                                                 "tcp://0.0.0.0:12344",
                                                 "tcp://127.0.0.1:9559",
                                                 "tcps://127.0.0.1:9559",
                                                 "tcp://0.0.0.0:9559",
                                                 "tcps://0.0.0.0:12345",
                                                 "tcps://0.0.0.0:9559"});

  qi::ApplicationSession::Config config;
  config.setDefaultListenUrl(defaultListenUrl);
  int ac = 3;
  std::array<char*, 3> av;
  av[0] = gav[0];
  av[1] = qi::os::strdup("--qi-listen-url");
  std::string urls;
  for (const auto& url: urlsListen)
  {
    urls += url;
    urls += ";";
  }
  av[2] = qi::os::strdup(urls.c_str());

  char** refav = av.data();
  qi::ApplicationSession appsession(ac, refav, config);

  ASSERT_EQ(expectedListen.size(), appsession.allListenUrl().size());
  for (unsigned int i = 0; i < expectedListen.size(); ++i)
    ASSERT_EQ(expectedListen.at(i), appsession.allListenUrl().at(i).str());
}


int main(int ac, char **av)
{
  gav[0] = av[0];
  ::testing::InitGoogleTest(&ac, av);
  return RUN_ALL_TESTS();
}
