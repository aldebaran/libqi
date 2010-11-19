/**
** Copyright (C) 2010 Aldebaran Robotics
*/

#include <gtest/gtest.h>
#include <qi/messaging/detail/process_identity.hpp>

using qi::detail::getProcessID;
using qi::detail::getHostName;
using qi::detail::getFirstMacAddress;
using qi::detail::getUUID;

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

TEST(ProcessIdentity, getUUID)
{
  std::cout << "UUID: " << getUUID() << std::endl;
}

TEST(ProcessIdentity, ProcessIdentity)
{
  qi::detail::ProcessIdentity i;
  std::cout <<
    i.hostName << " " <<
    i.macAddress << " " <<
    i.processID << " " <<
    i.id << std::endl;
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
//// ----------------------- GET MAC ADDRES LINUX ---------------
//
//#include <stdio.h>
//#include <string.h>
//#include <net/if.h>
//#include <sys/ioctl.h>
//
////
//// Global public data
////
//unsigned char cMacAddr[8]; // Server's MAC address
//
//static int GetSvrMacAddress( char *pIface )
//{
//int nSD; // Socket descriptor
//struct ifreq sIfReq; // Interface request
//struct if_nameindex *pIfList; // Ptr to interface name index
//struct if_nameindex *pListSave; // Ptr to interface name index
//
////
//// Initialize this function
////
//pIfList = (struct if_nameindex *)NULL;
//pListSave = (struct if_nameindex *)NULL;
//#ifndef SIOCGIFADDR
//// The kernel does not support the required ioctls
//return( 0 );
//#endif
//
////
//// Create a socket that we can use for all of our ioctls
////
//nSD = socket( PF_INET, SOCK_STREAM, 0 );
//if ( nSD < 0 )
//{
//// Socket creation failed, this is a fatal error
//printf( "File %s: line %d: Socket failed\n", __FILE__, __LINE__ );
//return( 0 );
//}
//
////
//// Obtain a list of dynamically allocated structures
////
//pIfList = pListSave = if_nameindex();
//
////
//// Walk thru the array returned and query for each interface's
//// address
////
//for ( pIfList; *(char *)pIfList != 0; pIfList++ )
//{
////
//// Determine if we are processing the interface that we
//// are interested in
////
//if ( strcmp(pIfList->if_name, pIface) )
//// Nope, check the next one in the list
//continue;
//strncpy( sIfReq.ifr_name, pIfList->if_name, IF_NAMESIZE );
//
////
//// Get the MAC address for this interface
////
//if ( ioctl(nSD, SIOCGIFHWADDR, &sIfReq) != 0 )
//{
//// We failed to get the MAC address for the interface
//printf( "File %s: line %d: Ioctl failed\n", __FILE__, __LINE__ );
//return( 0 );
//}
//memmove( (void *)&cMacAddr[0], (void *)&sIfReq.ifr_ifru.ifru_hwaddr.sa_data[0], 6 );
//break;
//}
//
////
//// Clean up things and return
////
//if_freenameindex( pListSave );
//close( nSD );
//return( 1 );
//}
//
//int main( int argc, char * argv[] )
//{
////
//// Initialize this program
////
//bzero( (void *)&cMacAddr[0], sizeof(cMacAddr) );
//if ( !GetSvrMacAddress("eth0") )
//{
//// We failed to get the local host's MAC address
//printf( "Fatal error: Failed to get local host's MAC address\n" );
//}
//printf( "HWaddr %02X:%02X:%02X:%02X:%02X:%02X\n",
//cMacAddr[0], cMacAddr[1], cMacAddr[2],
//cMacAddr[3], cMacAddr[4], cMacAddr[5] );
//
////
//// And exit
////
//exit( 0 );
//}


// ------------------------------- GET IP ADDRESSES LINUX
#include <arpa/inet.h>
       #include <sys/socket.h>
       #include <netdb.h>
       #include <ifaddrs.h>
       #include <stdio.h>
       #include <stdlib.h>
       #include <unistd.h>

       int
       main(int argc, char *argv[])
       {
           struct ifaddrs *ifaddr, *ifa;
           int family, s;
           char host[NI_MAXHOST];

           if (getifaddrs(&ifaddr) == -1) {
               perror("getifaddrs");
               exit(EXIT_FAILURE);
           }

           /* Walk through linked list, maintaining head pointer so we
              can free list later */

           for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {
               if (ifa->ifa_addr == NULL)
                   continue;

               family = ifa->ifa_addr->sa_family;

               /* Display interface name and family (including symbolic
                  form of the latter for the common families) */

               printf("%s  address family: %d%s\n",
                       ifa->ifa_name, family,
                       (family == AF_PACKET) ? " (AF_PACKET)" :
                       (family == AF_INET) ?   " (AF_INET)" :
                       (family == AF_INET6) ?  " (AF_INET6)" : "");

               /* For an AF_INET* interface address, display the address */

               if (family == AF_INET || family == AF_INET6) {
                   s = getnameinfo(ifa->ifa_addr,
                           (family == AF_INET) ? sizeof(struct sockaddr_in) :
                                                 sizeof(struct sockaddr_in6),
                           host, NI_MAXHOST, NULL, 0, NI_NUMERICHOST);
                   if (s != 0) {
                       printf("getnameinfo() failed: %s\n", gai_strerror(s));
                       exit(EXIT_FAILURE);
                   }
                   printf("\taddress: <%s>\n", host);
               }
           }

           freeifaddrs(ifaddr);
           exit(EXIT_SUCCESS);
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
