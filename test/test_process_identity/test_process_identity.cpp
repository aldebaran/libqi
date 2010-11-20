/**
** Copyright (C) 2010 Aldebaran Robotics
*/

#include <gtest/gtest.h>
#include <qi/transport/detail/network/process_identity.hpp>
#include <qi/transport/detail/network/network.hpp>

using qi::detail::getProcessID;
using qi::detail::getHostName;
using qi::detail::getFirstMacAddress;
using qi::detail::getUUID;
using qi::detail::getPrimaryPublicIPAddress;

TEST(ProcessIdentity, getProcessID)
{
  int pid = getProcessID();
  std::cout << "PID: " << getProcessID() << std::endl;
  ASSERT_NE(pid, 0) << "PID was zero. Unlikely";
}

TEST(ProcessIdentity, pidConstant)
{
  int pid1 = getProcessID();
  int pid2 = getProcessID();
  ASSERT_EQ(pid1, pid2) << "PID changed";
}

TEST(ProcessIdentity, getHostName)
{
  std::string host = getHostName();
  std::cout << "Host: " << host << std::endl;
  ASSERT_NE(host, std::string()) << "HostName was empty";
}

TEST(ProcessIdentity, hostNameIsConstant)
{
  std::string host1 = getHostName();
  std::string host2 = getHostName();
  ASSERT_EQ(host1, host2) << "Host name changed";
}

TEST(ProcessIdentity, getFirstMacAddress)
{
  std::string mac = getFirstMacAddress();
  std::cout << "Mac: " << mac << std::endl;
  ASSERT_NE(mac, std::string()) << "Mac Address was empty";
}

TEST(ProcessIdentity, macAddressIsConstant)
{
  std::string mac1 = getFirstMacAddress();
  std::string mac2 = getFirstMacAddress();
  ASSERT_EQ(mac1, mac2) << "Mac Address changed";
}

TEST(ProcessIdentity, getUUID)
{
  std::string u = getUUID();
  std::cout << "UUID: " << u << std::endl;
  ASSERT_NE(u, std::string()) << "UUID was empty";
}

TEST(ProcessIdentity, uuidIsNotConstant)
{
  std::string u1 = getUUID();
  std::string u2 = getUUID();
  ASSERT_NE(u1, u2) << "uuid did not change";
}

TEST(ProcessIdentity, getPrimaryPublicIPAddress)
{
  std::string ip =  getPrimaryPublicIPAddress();
  std::cout << "IP: " << ip << std::endl;
  ASSERT_NE(ip, std::string())  << "IP Address was empty";
}

TEST(ProcessIdentity, primaryIPIsConstant)
{
  std::string ip1 =  getPrimaryPublicIPAddress();
  std::string ip2 =  getPrimaryPublicIPAddress();
  ASSERT_EQ(ip1, ip2) << "IP Changed";
}

TEST(ProcessIdentity, ProcessIdentity)
{
  qi::detail::ProcessIdentity i;
  std::cout <<
    "hostName:   " << i.hostName   << std::endl <<
    "macAddress: " << i.macAddress << std::endl <<
    "processID:  " << i.processID  << std::endl <<
    "id:         " << i.id         << std::endl <<
    "ipAddress:  " << i.ipAddress  << std::endl;
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

TEST(ProcessIdentityPerf, 10000getUUID)
{
  std::string s;
  for(unsigned int i=0; i<10000; i++) {
    s = getUUID();
  }
}

TEST(ProcessIdentityPerf, 10000PrimaryPublicIPAddress)
{
  std::string s;
  for(unsigned int i=0; i<10000; i++) {
    s = getPrimaryPublicIPAddress();
  }
}

// --------------------------------------- GET IP ADDRESSES WIN
//
//#include <winsock2.h>
//
//// Add 'ws2_32.lib' to your linker options
//
//WSADATA WSAData;
//
//// Initialize winsock dll
//if(::WSAStartup(MAKEWORD(1, 0), &WSAData))
//{
//  // Error handling
//}
//
//// Get local host name
//char szHostName[128] = "";
//
//if(::gethostname(szHostName, sizeof(szHostName)))
//{
//  // Error handling -> call 'WSAGetLastError()'
//}
//
//// Get local IP addresses
//struct sockaddr_in SocketAddress;
//struct hostent     *pHost        = 0;
//
//pHost = ::gethostbyname(szHostName);
//if(!pHost)
//{
//  // Error handling -> call 'WSAGetLastError()'
//}
//
//char aszIPAddresses[10][16]; // maximum of ten IP addresses
//
//for(int iCnt = 0; ((pHost->h_addr_list[iCnt]) && (iCnt < 10)); ++iCnt)
//{
//  memcpy(&SocketAddress.sin_addr, pHost->h_addr_list[iCnt], pHost->h_length);
//  strcpy(aszIPAddresses[iCnt], inet_ntoa(SocketAddress.sin_addr));
//}
//
//// Cleanup
//WSACleanup();


