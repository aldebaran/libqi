/**
** Copyright (C) 2010 Aldebaran Robotics
*/

#include <gtest/gtest.h>
#include "messaging/network/machine_context.hpp"
#include "messaging/network/endpoint_context.hpp"
#include "messaging/network/network.hpp"
#include "messaging/network/master_endpoint.hpp"

using qi::detail::getProcessID;
using qi::detail::getHostName;
using qi::detail::getFirstMacAddress;
using qi::detail::getUUID;
using qi::detail::getPrimaryPublicIPAddress;
using qi::detail::getIPAddresses;
using qi::detail::validateMasterEndpoint;

TEST(EndpointContext, getProcessID)
{
  int pid = getProcessID();
  std::cout << "PID: " << getProcessID() << std::endl;
  ASSERT_NE(pid, 0) << "PID was zero. Unlikely";
}

TEST(EndpointContext, pidConstant)
{
  int pid1 = getProcessID();
  int pid2 = getProcessID();
  ASSERT_EQ(pid1, pid2) << "PID changed";
}

TEST(EndpointContext, getHostName)
{
  std::string host = getHostName();
  std::cout << "Host: " << host << std::endl;
  ASSERT_NE(host, std::string()) << "HostName was empty";
}

TEST(EndpointContext, hostNameIsConstant)
{
  std::string host1 = getHostName();
  std::string host2 = getHostName();
  ASSERT_EQ(host1, host2) << "Host name changed";
}

TEST(EndpointContext, getFirstMacAddress)
{
  std::string mac = getFirstMacAddress();
  std::cout << "Mac: " << mac << std::endl;
  ASSERT_NE(mac, std::string()) << "Mac Address was empty";
}

TEST(EndpointContext, macAddressIsConstant)
{
  std::string mac1 = getFirstMacAddress();
  std::string mac2 = getFirstMacAddress();
  ASSERT_EQ(mac1, mac2) << "Mac Address changed";
}

TEST(EndpointContext, getUUID)
{
  std::string u = getUUID();
  std::cout << "UUID: " << u << std::endl;
  ASSERT_NE(u, std::string()) << "UUID was empty";
}

TEST(EndpointContext, uuidIsNotConstant)
{
  std::string u1 = getUUID();
  std::string u2 = getUUID();
  ASSERT_NE(u1, u2) << "uuid did not change";
}

TEST(EndpointContext, getPrimaryPublicIPAddress)
{
  std::string ip =  getPrimaryPublicIPAddress();
  std::cout << "IP: " << ip << std::endl;
  ASSERT_NE(ip, std::string())  << "IP Address was empty";
}

TEST(EndpointContext, primaryIPIsConstant)
{
  std::string ip1 =  getPrimaryPublicIPAddress();
  std::string ip2 =  getPrimaryPublicIPAddress();
  ASSERT_EQ(ip1, ip2) << "IP Changed";
}

TEST(EndpointContext, getIPAddresses)
{
  std::vector<std::string> ips =  getIPAddresses();
  for (unsigned int i = 0; i < ips.size(); ++i) {
    std::cout << "IP: " << ips[i] << std::endl;
  }
  ASSERT_NE(0, ips.size())  << "No IP Addresses returned";
}

TEST(EndpointContext, isValidAddressRetValue)
{
  std::pair<std::string, int> res;
  std::string test = "127.0.0.1:5555";
  bool b = qi::detail::isValidAddress(test, res);
  ASSERT_EQ(true, b) << "127.0.0.1:5555 was considered invalid";
}

TEST(EndpointContext, isValidAddressIP)
{
  std::pair<std::string, int> res;
  std::string test = "127.0.0.1:5555";
  bool b = qi::detail::isValidAddress(test, res);
  ASSERT_EQ("127.0.0.1", res.first) << "Should have returned an IP";
}

TEST(EndpointContext, isValidAddressPort)
{
  std::pair<std::string, int> res;
  std::string test = "127.0.0.1:5555";
  bool b = qi::detail::isValidAddress(test, res);
  ASSERT_EQ(5555, res.second) << "Should have returned a port";
}

TEST(EndpointContext, isValidAddressRetValueInvalid)
{
  std::pair<std::string, int> res;
  std::string test = "127.0.0.655:5555";
  bool b = qi::detail::isValidAddress(test, res);
  ASSERT_EQ(false, b) << "127.0.0.655:5555 was considered valid";
}

TEST(EndpointContext, isValidAddressIPRubbish)
{
  std::pair<std::string, int> res;
  std::string test = "rubbish";
  bool b = qi::detail::isValidAddress(test, res);
  ASSERT_EQ(false, b) << "rubbish was considered valid";
  ASSERT_EQ(0, res.second) << "rubbish port not zero";
}

TEST(validateMasterEndpoint, valid)
{
  std::pair<std::string, int> res;
  std::string test = "127.0.0.1:5555";
  bool b = qi::detail::validateMasterEndpoint(test, res);
  ASSERT_EQ(true, b) << "127.0.0.1:5555 considered invalid";
  ASSERT_EQ(5555, res.second) << "should be 5555";
  ASSERT_EQ("tcp://127.0.0.1:5555", res.first) << "rubbish port not zero";
}

TEST(validateMasterEndpoint, incomplete)
{
  std::pair<std::string, int> res;
  std::string test = "127.0.0.1";
  bool b = qi::detail::validateMasterEndpoint(test, res);
  ASSERT_EQ(true, b) << "127.0.0.1 considered invalid";
  ASSERT_EQ(5555, res.second) << "should be 5555";
  ASSERT_EQ("tcp://127.0.0.1:5555", res.first) << "rubbish port not zero";
}

TEST(validateMasterEndpoint, invalid)
{
  std::pair<std::string, int> res;
  std::string test = "oink:asdsad";
  bool b = qi::detail::validateMasterEndpoint(test, res);
  ASSERT_EQ(false, b) << "asdasd:asdsad considered valid";
  ASSERT_EQ(0, res.second) << "should be 0";
  ASSERT_EQ("oink", res.first) << "rubbish port not zero";
}


TEST(EndpointContext, EndpointContext)
{
  qi::detail::EndpointContext i;
  std::cout <<
      "endpointID " << i.endpointID << std::endl <<
      "machineID  " << i.machineID  << std::endl <<
      "processID  " << i.processID  << std::endl <<
      "name       " << i.name       << std::endl <<
      "contextID  " << i.contextID  << std::endl <<
      "port       " << i.port       << std::endl;
}

TEST(MachineContext, MachineContext)
{
  qi::detail::MachineContext i;
  std::cout <<
      "machineID  " << i.machineID  << std::endl <<
      "hostName   " << i.hostName   << std::endl <<
      "platformID " << i.platformID << std::endl <<
      "publicIP   " << i.publicIP   << std::endl;
}

TEST(EndpointContextPerf, 1000000getProcessID)
{
  int id;
  for(unsigned int i=0; i<1000000; i++) {
    id = getProcessID();
  }
  id++;
}

TEST(EndpointContextPerf, 100000getHostName)
{
  std::string s;
  for(unsigned int i=0; i<100000; i++) {
    s = getHostName();
  }
}

TEST(EndpointContextPerf, 1000getFirstMacAddress)
{
  std::string s;
  for(unsigned int i=0; i<1000; i++) {
    s = getFirstMacAddress();
  }
}

TEST(EndpointContextPerf, 10000getUUID)
{
  std::string s;
  for(unsigned int i=0; i<10000; i++) {
    s = getUUID();
  }
}

