/*
*  Author(s):
*  - Chris  Kilner <ckilner@aldebaran-robotics.com>
*  - Cedric Gestes <gestes@aldebaran-robotics.com>
*
*  Copyright (C) 2010 Aldebaran Robotics
*/

#include "src/messaging/network/ip_address.hpp"
#include <boost/algorithm/string.hpp>

#ifdef _WIN32
#  include <windows.h>
#  include <winsock2.h>
#  include <iphlpapi.h>
#  include <Ws2tcpip.h>
#else
#  include <arpa/inet.h>
#  include <sys/socket.h>
#  include <netdb.h>
#  include <ifaddrs.h>
#  include <stdio.h>
#  include <stdlib.h>
#  include <unistd.h>
#endif

namespace qi {
namespace detail {

std::string getPrimaryPublicIPAddress()
{
  std::vector<std::string> ips = getIPAddresses();
  static const std::string ipLocalHost = "127.0.0.1";
  // todo: some logic to choose between good addresses
  for (unsigned int i = 0; i< ips.size(); i++)
  {
    if (ipLocalHost.compare(ips[i]) != 0)
      return ips[i];
  }
  return "";
}

bool isValidAddress(const std::string& userHostString,
  Address& outAddress)
{
  if (userHostString.empty())
    return false;

  std::vector<std::string> parts;
  boost::split(parts, userHostString, boost::is_any_of(":/"));

  if (parts.size() <= 2)
    parts.insert(parts.begin(), "tcp");

  if (parts.size() < 2 || parts.size() > 3)
    return false;

  if (parts[1].empty())
    return false;

  outAddress.transport = parts[0];
  outAddress.address = parts[1];

  if (parts.size() == 3)
  {
    outAddress.port = atoi(parts[2].c_str());
  }
  else
  {
    outAddress.port = 0;
    parts.push_back("0"); // hmmm
  }

  return isValidHostAndPort(outAddress.address, parts[2]);
}


bool isValidHostAndPort(const std::string& hostName, std::string& port)
{
  bool ret = true;
#ifdef _WIN32
  WSADATA WSAData;
  if(::WSAStartup(MAKEWORD(1, 0), &WSAData))
    ret = false;
#endif

  addrinfo req;
  memset(&req, 0, sizeof(req));
  req.ai_family = AF_INET;
  req.ai_socktype = SOCK_STREAM;
  req.ai_flags = AI_NUMERICSERV;
  addrinfo *res;

  if(getaddrinfo(hostName.c_str(), port.c_str(), &req, &res))
    ret = false; // lookup failed
  else
  {
    if (res == NULL)
      ret = false;
    else
      freeaddrinfo(res);
  }
#ifdef _WIN32
  WSACleanup();
#endif
  return ret;
}

std::vector<std::string> getIPAddresses()
{
  std::vector<std::string> ret;

#ifdef _WIN32
  // win version
  char szHostName[128] = "";

  WSADATA WSAData;
  if(::WSAStartup(MAKEWORD(1, 0), &WSAData))
    return ret;

  if(::gethostname(szHostName, sizeof(szHostName)))
    return ret;

  struct sockaddr_in socketAddress;
  struct hostent*    host = 0;

  host = ::gethostbyname(szHostName);
  if(!host)
    return ret;

  for(int i = 0; ((host->h_addr_list[i]) && (i < 10)); ++i)
  {
    memcpy(&socketAddress.sin_addr, host->h_addr_list[i], host->h_length);
    ret.push_back(inet_ntoa(socketAddress.sin_addr));
  }

  WSACleanup();
#else
  // linux version
  struct ifaddrs *ifaddr, *ifa;
  int family, s;
  char host[NI_MAXHOST];

  if (getifaddrs(&ifaddr) == -1)
    return ret;

  for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next)
  {
    if (ifa->ifa_addr == NULL)
      continue;
    family = ifa->ifa_addr->sa_family;

    // Don't include AF_INET6 for the moment
    if (family == AF_INET)
    {
      s = getnameinfo(ifa->ifa_addr,
        (family == AF_INET) ? sizeof(struct sockaddr_in) :
        sizeof(struct sockaddr_in6),
        host, NI_MAXHOST, NULL, 0, NI_NUMERICHOST);

      if (s != 0)
        break;
      ret.push_back(host);
    }
  }

  freeifaddrs(ifaddr);
#endif
  return ret;
}

} // namespace detail
} // namespace qi
