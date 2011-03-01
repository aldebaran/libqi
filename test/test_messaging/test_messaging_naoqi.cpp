
#include <gtest/gtest.h>
#include <qi/messaging.hpp>

using namespace qi;

Client client;

TEST(ClientTest, callNaoqi)
{
  client.connect();
  for (int i = 0; i< 100; i++) {
    std::cout << client.call<std::string>("ALLauncher.getBrokerName") << std::endl;
  }
}

