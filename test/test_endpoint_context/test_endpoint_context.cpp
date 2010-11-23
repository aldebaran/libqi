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
  ASSERT_NE(0, ips.size())  << "No IP Addresses returned";
}

TEST(EndpointContext, isValidAddressRetValue)
{
  std::pair<std::string, std::string> res;
  std::string test = "127.0.0.1:5555";
  bool b = qi::detail::isValidAddress(test, res);
  ASSERT_EQ(true, b) << "127.0.0.1:5555 was considered invalid";
}

TEST(EndpointContext, isValidAddressIP)
{
  std::pair<std::string, std::string> res;
  std::string test = "127.0.0.1:5555";
  bool b = qi::detail::isValidAddress(test, res);
  ASSERT_EQ("127.0.0.1", res.first) << "Should have returned an IP";
}

TEST(EndpointContext, isValidAddressPort)
{
  std::pair<std::string, std::string> res;
  std::string test = "127.0.0.1:5555";
  bool b = qi::detail::isValidAddress(test, res);
  ASSERT_EQ("5555", res.second) << "Should have returned a port";
}

TEST(EndpointContext, isValidAddressRetValueInvalid)
{
  std::pair<std::string, std::string> res;
  std::string test = "127.0.0.655:5555";
  bool b = qi::detail::isValidAddress(test, res);
  ASSERT_EQ(false, b) << "127.0.0.655:5555 was considered valid";
}

TEST(EndpointContext, isValidAddressIPRubbish)
{
  std::pair<std::string, std::string> res;
  std::string test = "rubbish";
  bool b = qi::detail::isValidAddress(test, res);
   ASSERT_EQ(false, b) << "rubbish was considered valid";
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

//TEST(EndpointContextPerf, 10000PrimaryPublicIPAddress)
//{
//  std::string s;
//  for(unsigned int i=0; i<10000; i++) {
//    s = getPrimaryPublicIPAddress();
//  }
//}

// --------------------------------------- GET IP ADDRESSES WIN



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


//
//
//#include <winsock2.h>
//#include <iphlpapi.h>
//#include <stdio.h>
//#include <stdlib.h>
//#pragma comment(lib, "IPHLPAPI.lib")
//
//#define MALLOC(x) HeapAlloc(GetProcessHeap(), 0, (x))
//#define FREE(x) HeapFree(GetProcessHeap(), 0, (x))
//
///* Note: could also use malloc() and free() */
//
//int __cdecl main()
//{
//
//    /* Declare and initialize variables */
//
//// It is possible for an adapter to have multiple
//// IPv4 addresses, gateways, and secondary WINS servers
//// assigned to the adapter. 
////
//// Note that this sample code only prints out the 
//// first entry for the IP address/mask, and gateway, and
//// the primary and secondary WINS server for each adapter. 
//
//    PIP_ADAPTER_INFO pAdapterInfo;
//    PIP_ADAPTER_INFO pAdapter = NULL;
//    DWORD dwRetVal = 0;
//    UINT i;
//
///* variables used to print DHCP time info */
//    struct tm newtime;
//    char buffer[32];
//    errno_t error;
//
//    ULONG ulOutBufLen = sizeof (IP_ADAPTER_INFO);
//    pAdapterInfo = (IP_ADAPTER_INFO *) MALLOC(sizeof (IP_ADAPTER_INFO));
//    if (pAdapterInfo == NULL) {
//        printf("Error allocating memory needed to call GetAdaptersinfo\n");
//        return 1;
//    }
//// Make an initial call to GetAdaptersInfo to get
//// the necessary size into the ulOutBufLen variable
//    if (GetAdaptersInfo(pAdapterInfo, &ulOutBufLen) == ERROR_BUFFER_OVERFLOW) {
//        FREE(pAdapterInfo);
//        pAdapterInfo = (IP_ADAPTER_INFO *) MALLOC(ulOutBufLen);
//        if (pAdapterInfo == NULL) {
//            printf("Error allocating memory needed to call GetAdaptersinfo\n");
//            return 1;
//        }
//    }
//
//    if ((dwRetVal = GetAdaptersInfo(pAdapterInfo, &ulOutBufLen)) == NO_ERROR) {
//        pAdapter = pAdapterInfo;
//        while (pAdapter) {
//            printf("\tComboIndex: \t%d\n", pAdapter->ComboIndex);
//            printf("\tAdapter Name: \t%s\n", pAdapter->AdapterName);
//            printf("\tAdapter Desc: \t%s\n", pAdapter->Description);
//            printf("\tAdapter Addr: \t");
//            for (i = 0; i < pAdapter->AddressLength; i++) {
//                if (i == (pAdapter->AddressLength - 1))
//                    printf("%.2X\n", (int) pAdapter->Address[i]);
//                else
//                    printf("%.2X-", (int) pAdapter->Address[i]);
//            }
//            printf("\tIndex: \t%d\n", pAdapter->Index);
//            printf("\tType: \t");
//            switch (pAdapter->Type) {
//            case MIB_IF_TYPE_OTHER:
//                printf("Other\n");
//                break;
//            case MIB_IF_TYPE_ETHERNET:
//                printf("Ethernet\n");
//                break;
//            case MIB_IF_TYPE_TOKENRING:
//                printf("Token Ring\n");
//                break;
//            case MIB_IF_TYPE_FDDI:
//                printf("FDDI\n");
//                break;
//            case MIB_IF_TYPE_PPP:
//                printf("PPP\n");
//                break;
//            case MIB_IF_TYPE_LOOPBACK:
//                printf("Lookback\n");
//                break;
//            case MIB_IF_TYPE_SLIP:
//                printf("Slip\n");
//                break;
//            default:
//                printf("Unknown type %ld\n", pAdapter->Type);
//                break;
//            }
//
//            printf("\tIP Address: \t%s\n",
//                   pAdapter->IpAddressList.IpAddress.String);
//            printf("\tIP Mask: \t%s\n", pAdapter->IpAddressList.IpMask.String);
//
//            printf("\tGateway: \t%s\n", pAdapter->GatewayList.IpAddress.String);
//            printf("\t***\n");
//
//            if (pAdapter->DhcpEnabled) {
//                printf("\tDHCP Enabled: Yes\n");
//                printf("\t  DHCP Server: \t%s\n",
//                       pAdapter->DhcpServer.IpAddress.String);
//
//                printf("\t  Lease Obtained: ");
//                /* Display local time */
//                error = _localtime32_s(&newtime, (__time32_t*) &pAdapter->LeaseObtained);
//                if (error)
//                    printf("Invalid Argument to _localtime32_s\n");
//                else {
//                    // Convert to an ASCII representation 
//                    error = asctime_s(buffer, 32, &newtime);
//                    if (error)
//                        printf("Invalid Argument to asctime_s\n");
//                    else
//                        /* asctime_s returns the string terminated by \n\0 */
//                        printf("%s", buffer);
//                }
//
//                printf("\t  Lease Expires:  ");
//                error = _localtime32_s(&newtime, (__time32_t*) &pAdapter->LeaseExpires);
//                if (error)
//                    printf("Invalid Argument to _localtime32_s\n");
//                else {
//                    // Convert to an ASCII representation 
//                    error = asctime_s(buffer, 32, &newtime);
//                    if (error)
//                        printf("Invalid Argument to asctime_s\n");
//                    else
//                        /* asctime_s returns the string terminated by \n\0 */
//                        printf("%s", buffer);
//                }
//            } else
//                printf("\tDHCP Enabled: No\n");
//
//            if (pAdapter->HaveWins) {
//                printf("\tHave Wins: Yes\n");
//                printf("\t  Primary Wins Server:    %s\n",
//                       pAdapter->PrimaryWinsServer.IpAddress.String);
//                printf("\t  Secondary Wins Server:  %s\n",
//                       pAdapter->SecondaryWinsServer.IpAddress.String);
//            } else
//                printf("\tHave Wins: No\n");
//            pAdapter = pAdapter->Next;
//            printf("\n");
//        }
//    } else {
//        printf("GetAdaptersInfo failed with error: %d\n", dwRetVal);
//
//    }
//    if (pAdapterInfo)
//        FREE(pAdapterInfo);
//
//    return 0;
//}