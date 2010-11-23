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
      // TODO implement
      return "";
    }

    std::vector<std::string> getIPAddresses() {
      std::vector<std::string> v;
      // TODO implement
      return v;
    }

    bool isValidAddress(const std::string& userHostString, std::pair<std::string, std::string>& outHostAndPort)
    {
      // TODO implement
      return false;
    }

    bool isValidHostAndPort(const std::string& hostName, std::string& port) {
      // TODO implement
      return false;
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


    bool isValidAddress(const std::string& userHostString, std::pair<std::string, std::string>& outHostAndPort)
    {
      if (userHostString.empty()) {
        return false;
      }

      std::vector<std::string> parts;
      boost::split(parts, userHostString, boost::is_any_of(":"));

      if (parts.empty() || parts.size() > 2) {
        return false;
      }

      if (parts[0].empty()) {
        return false;
      }
      outHostAndPort.first = parts[0];

      if (parts.size() == 2) {
        outHostAndPort.second = parts[1];
      }

      return isValidHostAndPort(outHostAndPort.first, outHostAndPort.second);
    }

    bool isValidHostAndPort(const std::string& hostName, std::string& port) {
      addrinfo req;
      memset(&req, 0, sizeof(req));
      req.ai_family = AF_INET;
      req.ai_socktype = SOCK_STREAM;
      req.ai_flags = AI_NUMERICSERV;
      addrinfo *res;

      int ok = getaddrinfo(hostName.c_str(), port.c_str(), &req, &res);
      if (ok) {
        return false;
      }
      if (res == NULL) {
        return false;
      }
      freeaddrinfo(res);
      return true;
    }
  }
}

#endif
