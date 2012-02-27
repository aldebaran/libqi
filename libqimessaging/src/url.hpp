/*
*  Author(s):
*  - Laurent Lec <llec@aldebaran-robotics.com>
*
*  Copyright (C) 2012 Aldebaran Robotics
*/


#pragma once
#ifndef _SRC_URL_HPP_
#define _SRC_URL_HPP_

#include <string>
#include <sstream>

namespace qi {
  class Url
  {
  private:
    std::string    _url;
    unsigned short _port;
    std::string    _host;
    unsigned int   _protocol;
  public:
    enum UrlProtocol {
      Invalid = 0,
      Unknown = 1,
      Tcp     = 2,
    };

    Url(const std::string &url)
      : _url(url)
    {
      size_t begin = 0;
      size_t end = 0;
      end = url.find(":");
      std::string type = url.substr(begin, end);

      if (type == "tcp")
      {
        _protocol = Tcp;
      }
      else
      {
        _protocol = Unknown;
      }

      begin = end + 3;
      end = url.find(":", begin);
      _host = url.substr(begin, end - begin);
      begin = end + 1;
      std::stringstream ss(url.substr(begin));
      ss >> _port;
    }

    unsigned short port() const { return _port; }
    const std::string& host() const { return _host; }
    unsigned int protocol() const { return _protocol; }
    const std::string& str() const { return _url; }
  };
}

#endif  // _SRC_URL_HPP_
