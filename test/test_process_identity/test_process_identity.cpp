/**
** Copyright (C) 2010 Aldebaran Robotics
*/

#include <gtest/gtest.h>
#include <qi/nodes/detail/process_identity.hpp>

using qi::detail::getProcessID;
using qi::detail::getHostName;
using qi::detail::getFirstMacAddress;

TEST(ProcessIdentity, getProcessID)
{
  std::cout << "PID: " << getProcessID() << std::endl;
}

TEST(ProcessIdentity, getHostName)
{
  std::cout << "Host: " << getHostName() << std::endl;
}

TEST(ProcessIdentity, getFirstMacAddress)
{
  std::cout << "Mac: " << getFirstMacAddress() << std::endl;
}

TEST(ProcessIdentity, ProcessIdentity)
{
  qi::detail::ProcessIdentity i;
  std::cout <<
    i.hostName << " " <<
    i.macAddress << " " <<
    i.processID << std::endl;
}

TEST(ProcessIdentityPerf, 1000000getProcessID)
{
  int id;
  for(unsigned int i=0; i<1000000; i++) {
    id = getProcessID();
  }
  id++;
}

TEST(ProcessIdentityPerf, 100000getHostName)
{
  std::string s;
  for(unsigned int i=0; i<100000; i++) {
    s = getHostName();
  }
}

TEST(ProcessIdentityPerf, 1000getFirstMacAddress)
{
  std::string s;
  for(unsigned int i=0; i<1000; i++) {
    s = getFirstMacAddress();
  }
}
//
//#include "windows.h"
//#include "Iphlpapi.h"
//
//int __cdecl main(int argc, char **argv)
//{
//
//}