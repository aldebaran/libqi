/*
**  Copyright (C) 2022 Aldebaran Robotics
**  See COPYING for the license
*/

#include <qi/messaging/tcpscheme.hpp>
#include <qi/assert.hpp>

namespace qi
{

boost::optional<TcpScheme> tcpSchemeFromUriScheme(const std::string& scheme)
{
  if (scheme == "tcp")
    return TcpScheme::Raw;
  if (scheme == "tcps")
    return TcpScheme::Tls;
  if (scheme == "tcpsm")
    return TcpScheme::MutualTls;
  return {};
}

boost::optional<TcpScheme> tcpScheme(const Url& url)
{
  if (!url.hasProtocol())
    return {};
  return tcpSchemeFromUriScheme(url.protocol());
}

std::string to_string(TcpScheme scheme)
{
  switch (scheme)
  {
    case TcpScheme::Raw: return "tcp";
    case TcpScheme::Tls: return "tcps";
    case TcpScheme::MutualTls: return "tcpsm";
    default:
      QI_ASSERT_UNREACHABLE();
      throw std::domain_error("unexpected TcpScheme value");
  }
}

bool isWithTls(TcpScheme scheme)
{
  switch (scheme)
  {
    case TcpScheme::Tls:
    case TcpScheme::MutualTls:
      return true;
    default:
      return false;
  }
}

} // namespace qi
