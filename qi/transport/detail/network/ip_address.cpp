/*
** Author(s):
**  - Chris Kilner <ckilner@aldebaran-robotics.com>
**
** Copyright (C) 2010 Aldebaran Robotics
*/
#include <qi/transport/detail/network/ip_address.hpp>

#ifdef _WIN32
#include <windows.h>
#include <winsock2.h>
#include <iphlpapi.h>
#pragma comment(lib, "IPHLPAPI.lib")

namespace qi {
  namespace detail {
    std::string getPrimaryPublicIPAddress() {
      return "";
    }

    std::vector<std::string> getIPAddresses() {
      std::vector<std::string> v;
      return v;
    }
  }
}
#else  // end WIN32

#include <arpa/inet.h>
#include <sys/socket.h>
#include <netdb.h>
#include <ifaddrs.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <boost/algorithm/string.hpp>


namespace qi {
  namespace detail {
    std::string getPrimaryPublicIPAddress() {
      std::vector<std::string> ips = getIPAddresses();
      static const std::string ipLocalHost = "127.0.0.1";
      // todo: some logic to choose between good addresses
      for(unsigned int i = 0; i< ips.size(); i++) {
        if (ipLocalHost.compare(ips[i]) != 0) {
          return ips[i];
        }
      }
      if (ips.size() > 0) {
        return ips[0];
      }
      return "";
    }

    std::vector<std::string> getIPAddresses() {
      std::vector<std::string> ret;

      struct ifaddrs *ifaddr, *ifa;
      int family, s;
      char host[NI_MAXHOST];

      if (getifaddrs(&ifaddr) == -1) {
        return ret;
      }

      for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {
        if (ifa->ifa_addr == NULL)
          continue;

        family = ifa->ifa_addr->sa_family;

        // Don't include AF_INET6 for the moment
        if (family == AF_INET) {
          s = getnameinfo(ifa->ifa_addr,
                          (family == AF_INET) ? sizeof(struct sockaddr_in) :
                              sizeof(struct sockaddr_in6),
                          host, NI_MAXHOST, NULL, 0, NI_NUMERICHOST);
          if (s != 0) {
            // err is in: gai_strerror(s));
            break;
          }
          ret.push_back(host);
          //printf("%s\n", host);
        }
      }

      freeifaddrs(ifaddr);
      return ret;
    }

//
//    bool resolveHostAndPort(const std::string& userHostString, std::pair<std::string, std::string>& hostAndPort)
//    {
//      if (userHostString.empty()) {
//        return false;
//      }
//
//      std::vector<std::string> parts;
//      boost::split(parts, userHostString, boost::is_any_of(":"));
//
//      if (parts.empty() || parts.size() > 2) {
//        return false;
//      }
//
//      if (parts[0].empty()) {
//        return false;
//      }
//      std::string host = parts[0];
//
//      if (parts.size() == 2) {
//        hostAndPort.second = parts[1];
//      }
//
//
//      addrinfo req;
//      memset(&req, 0, sizeof(req));
//      req.ai_family = AF_INET;
//      req.ai_socktype = SOCK_STREAM;
//      req.ai_flags = AI_NUMERICSERV;
//      addrinfo *res;
//
//      int rc = getaddrinfo (hostname.c_str (), service.c_str (), &req, &res);
//      if (rc) {
//        errno = EINVAL;
//        return -1;
//      }
//
//      //  Copy first result to output addr with hostname and service.
//      zmq_assert ((size_t) (res->ai_addrlen) <= sizeof (*addr_));
//      memcpy (addr_, res->ai_addr, res->ai_addrlen);
//      *addr_len_ = res->ai_addrlen;
//
//      freeaddrinfo(res);
//
//      return 0;
//    }
//
//    bool resolveHostName(const std::string& literalHostName, std::string numericHostName) {
//      addrinfo req;
//      memset(&req, 0, sizeof(req));
//      req.ai_family = AF_INET;
//      req.ai_socktype = SOCK_STREAM;
//      req.ai_flags = AI_NUMERICSERV;
//      addrinfo *res;
//
//      int rc = getaddrinfo (literalHostName.c_str (), service.c_str (), &req, &res);
//      if (rc) {
//        errno = EINVAL;
//        return -1;
//      }
//
//      //  Copy first result to output addr with hostname and service.
//      zmq_assert ((size_t) (res->ai_addrlen) <= sizeof (*addr_));
//      memcpy (addr_, res->ai_addr, res->ai_addrlen);
//      *addr_len_ = res->ai_addrlen;
//
//      freeaddrinfo(res);
//
//      return 0;
//    }
  }
}

#endif
//
//#include <sys/socket.h>
//#include <sys/ioctl.h>
//#include <linux/if.h>
//#include <netdb.h>
//#include <stdio.h>
//
//int main()
//{
//  struct ifreq s;
//  int fd = socket(PF_INET, SOCK_DGRAM, IPPROTO_IP);
//
//  strcpy(s.ifr_name, "eth0");
//  if (0 == ioctl(fd, SIOCGIFHWADDR, &s)) {
//    int i;
//    for (i = 0; i < 6; ++i)
//      printf(" %02x", (unsigned char) s.ifr_addr.sa_data[i]);
//    puts("\n");
//    return 0;
//  }
//  return 1;
//}
//}
//#endif  // !_WIN32
//
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
