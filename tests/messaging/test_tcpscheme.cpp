/*
**  Copyright (C) 2022 SoftBank Robotics Europe
**  See COPYING for the license
*/

#include <qi/messaging/tcpscheme.hpp>
#include <boost/optional/optional_io.hpp>
#include <gtest/gtest.h>

namespace qi
{
  std::ostream& operator<<(std::ostream& os, const TcpScheme& s)
  {
    return os << qi::to_string(s);
  }
}

TEST(TcpScheme, FromUriScheme)
{
  using namespace qi;
  EXPECT_EQ(boost::make_optional(TcpScheme::Raw), tcpSchemeFromUriScheme("tcp"));
  EXPECT_EQ(boost::make_optional(TcpScheme::Tls), tcpSchemeFromUriScheme("tcps"));
  EXPECT_EQ(boost::make_optional(TcpScheme::MutualTls), tcpSchemeFromUriScheme("tcpsm"));
  EXPECT_EQ(boost::none, tcpSchemeFromUriScheme("cookies"));
}

TEST(TcpScheme, FromUrl)
{
  using namespace qi;
  EXPECT_EQ(boost::make_optional(TcpScheme::Raw), tcpScheme(Url("tcp://localhost:80")));
  EXPECT_EQ(boost::make_optional(TcpScheme::Tls), tcpScheme(Url("tcps://192.168.0.1:22")));
  EXPECT_EQ(boost::make_optional(TcpScheme::MutualTls), tcpScheme(Url("tcpsm://1.2.3.4:56")));
  EXPECT_EQ(boost::none, tcpScheme(Url("softbankrobotics.com:443")));
  EXPECT_EQ(boost::none, tcpScheme(Url("udp://mydhcpserver:67")));
}

TEST(TcpScheme, ToString)
{
  using namespace qi;
  EXPECT_EQ("tcp", to_string(TcpScheme::Raw));
  EXPECT_EQ("tcps", to_string(TcpScheme::Tls));
  EXPECT_EQ("tcpsm", to_string(TcpScheme::MutualTls));
}

TEST(TcpScheme, IsTcpSchemeWithTls)
{
  using namespace qi;
  EXPECT_EQ(false, isWithTls(TcpScheme::Raw));
  EXPECT_EQ(true, isWithTls(TcpScheme::Tls));
  EXPECT_EQ(true, isWithTls(TcpScheme::MutualTls));
}

