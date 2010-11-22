/**
** Copyright (C) 2010 Aldebaran Robotics
*/

#include <gtest/gtest.h>
#include <qi/transport/detail/network/negotiate_endpoint.hpp>

using qi::detail::EndpointContext;
using qi::detail::negotiateEndpoint;

TEST(NegotiateEndpoint, sameDefaultContext)
{
  EndpointContext c1;
  EndpointContext c2;
  std::string endpoint = negotiateEndpoint(c1, c2);
  std::cout << endpoint << std::endl;
}

TEST(NegotiateEndpoint, realServerPort)
{
  EndpointContext c1;
  EndpointContext c2;
  c2.port = 5555;
  std::string endpoint = negotiateEndpoint(c1, c2);
  std::cout << endpoint << std::endl;
}

TEST(NegotiateEndpoint, realServerPortWin)
{
  EndpointContext c1;
  EndpointContext c2;
  c2.platformID = 0;
  c2.port = 5555;
  std::string endpoint = negotiateEndpoint(c1, c2);
  std::cout << endpoint << std::endl;
}

TEST(NegotiateEndpoint, differentContext)
{
  EndpointContext c1;
  EndpointContext c2;
  c2.port = 5555;
  c2.contextID = "somethingdifferent";
  std::string endpoint = negotiateEndpoint(c1, c2);
  std::cout << endpoint << std::endl;
}

TEST(NegotiateEndpoint, differentContextWin)
{
  EndpointContext c1;
  EndpointContext c2;
  c2.port = 5555;
  c2.contextID = "somethingdifferent";
  c2.platformID = 0;
  std::string endpoint = negotiateEndpoint(c1, c2);
  std::cout << endpoint << std::endl;
}


TEST(NegotiateEndpoint, differentMachine)
{
  EndpointContext c1;
  EndpointContext c2;
  c2.port = 5555;
  c2.machineID = "somethingdifferent";
  std::string endpoint = negotiateEndpoint(c1, c2);
  std::cout << endpoint << std::endl;
}

TEST(NegotiateEndpoint, differentMachineWin)
{
  EndpointContext c1;
  EndpointContext c2;
  c2.port = 5555;
  c2.machineID = "somethingdifferent";
  c2.platformID = 0;
  std::string endpoint = negotiateEndpoint(c1, c2);
  std::cout << endpoint << std::endl;
}

