/**
** Copyright (C) 2010 Aldebaran Robotics
*/

#include <gtest/gtest.h>
#include <qi/transport/detail/network/endpoint_context.hpp>
#include <qi/transport/detail/network/network.hpp>

using qi::detail::getProcessID;
using qi::detail::getHostName;
using qi::detail::getFirstMacAddress;
using qi::detail::getUUID;
using qi::detail::getPrimaryPublicIPAddress;
using qi::detail::getIPAddresses;

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
  //ASSERT_NE(ip, std::string())  << "IP Address was empty";
}

TEST(EndpointContext, isValidAddress)
{
  std::pair<std::string, std::string> res;
  std::string test = "ckilner-linux-po:5555";
  bool b = qi::detail::isValidAddress(test, res);
  std::cout << test << " " << b <<  " " << res.first << ":" << res.second << std::endl;

  test = "bollocks";
  res.first = "";
  res.second = "";
  b = qi::detail::isValidAddress(test, res);
  std::cout << test << " " << b <<  " " << res.first << ":" << res.second << std::endl;

  test = "";
  res.first = "";
  res.second = "";
  b = qi::detail::isValidAddress(test, res);
  std::cout << test << " " << b <<  " " << res.first << ":" << res.second << std::endl;

  test = "127.0.0.1:ouoiu";
  res.first = "";
  res.second = "";
  b = qi::detail::isValidAddress(test, res);
  std::cout << test << " " << b <<  " " << res.first << ":" << res.second << std::endl;

  test = "172.16.1.12:8080";
  res.first = "";
  res.second = "";
  b = qi::detail::isValidAddress(test, res);
  std::cout << test << " " << b <<  " " << res.first << ":" << res.second << std::endl;
}

TEST(EndpointContext, EndpointContext)
{
  qi::detail::EndpointContext i;
  std::cout <<
      "endpointID " << i.endpointID << std::endl <<
      "machineID  " << i.machineID  << std::endl <<
      "hostName   " << i.hostName   << std::endl <<
      "processID  " << i.processID  << std::endl <<
      "platformID " << i.platformID << std::endl <<
      "publicIP   " << i.publicIP   << std::endl <<
      "name       " << i.name       << std::endl <<
      "contextID  " << i.contextID  << std::endl <<
      "port       " << i.port       << std::endl;
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

TEST(EndpointContextPerf, 10000PrimaryPublicIPAddress)
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

//
//#undef UNICODE
//
////#include <winsock2.h>
////#include <ws2tcpip.h>
//#include <sys/types.h>
//#include <sys/socket.h>
//#include <arpa/inet.h>
//#include <netinet/in.h>
//#include <netdb.h>
//#include <stdio.h>
//
//// link with Ws2_32.lib
//
//int  main(int argc, char **argv)
//{
//
//    //-----------------------------------------
//    // Declare and initialize variables
//    //WSADATA wsaData;
//    //int iResult;
//    //int iRetval;
//
//    int dwRetval;
//
//    int i = 1;
//
//    struct addrinfo *result = NULL;
//    struct addrinfo *ptr = NULL;
//    struct addrinfo hints;
//
//    struct sockaddr_in  *sockaddr_ipv4;
////    struct sockaddr_in6 *sockaddr_ipv6;
//    //LPSOCKADDR sockaddr_ip;
//
//    char ipstringbuffer[46];
//    unsigned long ipbufferlength = 46;
//
//    // Validate the parameters
//    if (argc != 3) {
//        printf("usage: %s <hostname> <servicename>\n", argv[0]);
//        printf("getaddrinfo provides protocol-independent translation\n");
//        printf("   from an ANSI host name to an IP address\n");
//        printf("%s example usage\n", argv[0]);
//        printf("   %s www.contoso.com 0\n", argv[0]);
//        return 1;
//    }
//
////    // Initialize Winsock
////    iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
////    if (iResult != 0) {
////        printf("WSAStartup failed: %d\n", iResult);
////        return 1;
////    }
//
//    //--------------------------------
//    // Setup the hints address info structure
//    // which is passed to the getaddrinfo() function
//    //addrinfo hints;
//    memset (&hints, 0, sizeof (hints));
//
//    //ZeroMemory( &hints, sizeof(hints) );
//    hints.ai_family = AF_UNSPEC;
//    hints.ai_socktype = SOCK_STREAM;
//    hints.ai_protocol = IPPROTO_TCP;
//
//    printf("Calling getaddrinfo with following parameters:\n");
//    printf("\tnodename = %s\n", argv[1]);
//    printf("\tservname (or port) = %s\n\n", argv[2]);
//
////--------------------------------
//// Call getaddrinfo(). If the call succeeds,
//// the result variable will hold a linked list
//// of addrinfo structures containing response
//// information
//    dwRetval = getaddrinfo(argv[1], argv[2], &hints, &result);
//    if ( dwRetval != 0 ) {
//        printf("getaddrinfo failed with error: %d\n", dwRetval);
////        WSACleanup();
//        return 1;
//    }
//
//    printf("getaddrinfo returned success\n");
//
//    // Retrieve each address and print out the hex bytes
//    for(ptr=result; ptr != NULL ;ptr=ptr->ai_next) {
//
//        printf("getaddrinfo response %d\n", i++);
//        printf("\tFlags: 0x%x\n", ptr->ai_flags);
//        printf("\tFamily: ");
//        switch (ptr->ai_family) {
//            case AF_UNSPEC:
//                printf("Unspecified\n");
//                break;
//            case AF_INET:
//                printf("AF_INET (IPv4)\n");
//                sockaddr_ipv4 = (struct sockaddr_in *) ptr->ai_addr;
//                printf("\tIPv4 address %s\n",
//                    inet_ntoa(sockaddr_ipv4->sin_addr) );
//                break;
//            case AF_INET6:
//                printf("AF_INET6 (IPv6)\n");
//            default:
//                printf("Other %ld\n", ptr->ai_family);
//                break;
//        }
//        printf("\tSocket type: ");
//        switch (ptr->ai_socktype) {
//            case 0:
//                printf("Unspecified\n");
//                break;
//            case SOCK_STREAM:
//                printf("SOCK_STREAM (stream)\n");
//                break;
//            case SOCK_DGRAM:
//                printf("SOCK_DGRAM (datagram) \n");
//                break;
//            case SOCK_RAW:
//                printf("SOCK_RAW (raw) \n");
//                break;
//            case SOCK_RDM:
//                printf("SOCK_RDM (reliable message datagram)\n");
//                break;
//            case SOCK_SEQPACKET:
//                printf("SOCK_SEQPACKET (pseudo-stream packet)\n");
//                break;
//            default:
//                printf("Other %ld\n", ptr->ai_socktype);
//                break;
//        }
//        printf("\tProtocol: ");
//        switch (ptr->ai_protocol) {
//            case 0:
//                printf("Unspecified\n");
//                break;
//            case IPPROTO_TCP:
//                printf("IPPROTO_TCP (TCP)\n");
//                break;
//            case IPPROTO_UDP:
//                printf("IPPROTO_UDP (UDP) \n");
//                break;
//            default:
//                printf("Other %ld\n", ptr->ai_protocol);
//                break;
//        }
//        printf("\tLength of this sockaddr: %d\n", ptr->ai_addrlen);
//        printf("\tCanonical name: %s\n", ptr->ai_canonname);
//    }
//
//    freeaddrinfo(result);
//    //WSACleanup();
//
//    return 0;
//}
//
