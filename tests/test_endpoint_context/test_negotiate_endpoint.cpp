/**
** Copyright (C) 2010 Aldebaran Robotics
*/

#include <gtest/gtest.h>
#include "messaging/network/endpoints.hpp"
#include "messaging/network/machine_context.hpp"
#include "messaging/network/platform.hpp"

using qi::detail::MachineContext;
using qi::detail::EndpointContext;
using qi::detail::negotiateEndpoint;
using qi::detail::Platform;

TEST(NegotiateEndpoint, sameDefaultContext)
{
  EndpointContext c1;
  EndpointContext c2;
  MachineContext m;
  std::string endpoint = negotiateEndpoint(c1, c2, m);
  // std::cout << endpoint << std::endl;
  std::string expected = "inproc://127.0.0.1:0";
  EXPECT_EQ(expected, endpoint) << "Same context should return inproc://127.0.0.1:0";
}

TEST(NegotiateEndpoint, realServerPort)
{
  EndpointContext c1;
  EndpointContext c2;
  MachineContext m;
  c2.port = 5555;
  std::string endpoint = negotiateEndpoint(c1, c2, m);
  // std::cout << endpoint << std::endl;
  std::string expected = "inproc://127.0.0.1:5555";
  EXPECT_EQ(expected, endpoint) << "Same context 5555 should return inproc://127.0.0.1:5555";
}

TEST(NegotiateEndpoint, realServerPortWin)
{
  EndpointContext c1;
  EndpointContext c2;
  MachineContext m;
  m.platformID = (Platform)0;
  c2.port = 5555;
  std::string endpoint = negotiateEndpoint(c1, c2, m);
  // std::cout << endpoint << std::endl;
  std::string expected = "inproc://127.0.0.1:5555";
  EXPECT_EQ(expected, endpoint) << "Same context win should return inproc://127.0.0.1:5555";
}

TEST(NegotiateEndpoint, differentContext)
{
  EndpointContext c1;
  EndpointContext c2;
  MachineContext m;
  c2.port = 5555;
  c2.contextID = "somethingdifferent";
  std::string endpoint = negotiateEndpoint(c1, c2, m);
  // std::cout << endpoint << std::endl;
  if (qi::detail::getPlatform() == 0) { // WIN
    std::string expected = "tcp://127.0.0.1:5555";
    EXPECT_EQ(expected, endpoint) << "Different context win should return tcp://127.0.0.1:5555";
  } else {
    std::string expected = "ipc:///tmp/qi_127.0.0.1:5555";
    EXPECT_EQ(expected, endpoint) << "Different context linux should return ipc:///tmp/127.0.0.1:5555";
  }
}

TEST(NegotiateEndpoint, differentContextWin)
{
  EndpointContext c1;
  EndpointContext c2;
  MachineContext m;
  c2.port = 5555;
  c2.contextID = "somethingdifferent";
  m.platformID = (Platform)0;
  std::string endpoint = negotiateEndpoint(c1, c2, m);
  // std::cout << endpoint << std::endl;
  std::string expected = "tcp://127.0.0.1:5555";
  EXPECT_EQ(expected, endpoint) << "Different context win should return tcp://127.0.0.1:5555";
}


TEST(NegotiateEndpoint, differentMachine)
{
  EndpointContext c1;
  EndpointContext c2;
  MachineContext m;
  c2.port = 5555;
  c2.machineID = "somethingdifferent";
  std::string endpoint = negotiateEndpoint(c1, c2, m);
  // std::cout << endpoint << std::endl;
  // should not be 127.0.0.1
}

TEST(NegotiateEndpoint, differentMachineWin)
{
  EndpointContext c1;
  EndpointContext c2;
  MachineContext m;
  c2.port = 5555;
  c2.machineID = "somethingdifferent";
  m.platformID = (Platform)0;
  std::string endpoint = negotiateEndpoint(c1, c2, m);
  // std::cout << endpoint << std::endl;
  // should not be 127.0.0.1
}

