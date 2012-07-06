/*
*  Author(s):
*  - Laurent Lec <llec@aldebaran-robotics.com>
*
*  Copyright (C) 2012 Aldebaran Robotics
*/


#pragma once
#ifndef _QIMESSAGING_URL_HPP_
#define _QIMESSAGING_URL_HPP_

#include <qimessaging/api.hpp>
#include <string>

namespace qi {

  class QIMESSAGING_API Url
  {
  public:
    enum Protocol {
      Protocol_Invalid = 0,
      Protocol_Unknown = 1,
      Protocol_Any     = 2,
      Protocol_Tcp     = 3,
      Protocol_TcpSsl  = 4,
    };

    Url();
    Url(const char *url);
    Url(const std::string &url);
    Url(const qi::Url& url);
    Url &operator=(const std::string& rhs);

    unsigned short     port() const     { return _port; }
    const std::string& host() const     { return _host; }
    unsigned int       protocol() const { return _protocol; }
    const std::string& str() const      { return _url; }

  private:
    std::string    _url;
    unsigned short _port;
    std::string    _host;
    unsigned int   _protocol;
    void          *_reserved;
  };
}

#endif  // _QIMESSAGING_URL_HPP_
