/*
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2012 Aldebaran Robotics
*/

#include <qimessaging/url.hpp>
#include <sstream>

namespace qi {


static void split_me(const std::string &url, unsigned short &_port, std::string &_host, unsigned int &_protocol)
{
  size_t begin = 0;
  size_t end = 0;
  end = url.find(":");
  std::string type = url.substr(begin, end);

  if (type == "tcp")
  {
    _protocol = Url::Protocol_Tcp;
  }
  else if (type =="tcp+ssl" || type == "ssl+tcp")
  {
    _protocol = Url::Protocol_TcpSsl;
  }
  else
  {
    _protocol = Url::Protocol_Any;
  }

  begin = end + 3;
  end = url.find_last_of(":");
  _host = url.substr(begin, end - begin);
  begin = end + 1;
  std::stringstream ss(url.substr(begin));
  ss >> _port;
}

Url::Url(const std::string &url)
  : _url(url)
  , _port(0)
  , _host("")
  , _protocol(Protocol_Invalid)
  , _reserved(0)
{
  split_me(_url, _port, _host, _protocol);
}

Url::Url(const char *url)
  : _url(url)
  , _port(0)
  , _host("")
  , _protocol(Protocol_Invalid)
  , _reserved(0)
{
  split_me(_url, _port, _host, _protocol);
}

Url::Url(const qi::Url& url)
  : _url(url._url)
  , _port(url._port)
  , _host(url._host)
  , _protocol(url._protocol)
  , _reserved(0)
{
}

Url &Url::operator=(const std::string& rhs)
{
  _url = rhs;
  split_me(_url, _port, _host, _protocol);
  return *this;
}

}

