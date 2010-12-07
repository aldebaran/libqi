/**
** Copyright (C) 2010 Aldebaran Robotics
*/

#include <gtest/gtest.h>
#include <qi/messaging/src/network/endpoints.hpp>
#include <qi/messaging/src/network/machine_context.hpp>

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
  std::cout << endpoint << std::endl;
}

TEST(NegotiateEndpoint, realServerPort)
{
  EndpointContext c1;
  EndpointContext c2;
  MachineContext m;
  c2.port = 5555;
  std::string endpoint = negotiateEndpoint(c1, c2, m);
  std::cout << endpoint << std::endl;
}

TEST(NegotiateEndpoint, realServerPortWin)
{
  EndpointContext c1;
  EndpointContext c2;
  MachineContext m;
  m.platformID = (Platform)0;
  c2.port = 5555;
  std::string endpoint = negotiateEndpoint(c1, c2, m);
  std::cout << endpoint << std::endl;
}

TEST(NegotiateEndpoint, differentContext)
{
  EndpointContext c1;
  EndpointContext c2;
  MachineContext m;
  c2.port = 5555;
  c2.contextID = "somethingdifferent";
  std::string endpoint = negotiateEndpoint(c1, c2, m);
  std::cout << endpoint << std::endl;
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
  std::cout << endpoint << std::endl;
}


TEST(NegotiateEndpoint, differentMachine)
{
  EndpointContext c1;
  EndpointContext c2;
  MachineContext m;
  c2.port = 5555;
  c2.machineID = "somethingdifferent";
  std::string endpoint = negotiateEndpoint(c1, c2, m);
  std::cout << endpoint << std::endl;
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
  std::cout << endpoint << std::endl;
}

