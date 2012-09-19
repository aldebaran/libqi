#pragma once
/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/

#ifndef _QIMESSAGING_URL_HPP_
#define _QIMESSAGING_URL_HPP_

#include <qimessaging/api.hpp>
#include <string>

namespace qi {

  class QIMESSAGING_API Url
  {
  public:

    Url();
    Url(const char *url);
    Url(const std::string &url);
    Url(const qi::Url& url);

    //Url &operator=(const std::string& rhs);

    unsigned short     port() const     { return _port; }
    const std::string& host() const     { return _host; }
    const std::string& protocol() const { return _protocol; }
    const std::string& str() const      { return _url; }
    bool isValid() const;

  private:
    std::string    _url;
    unsigned short _port;
    std::string    _host;
    std::string    _protocol;
    void          *_reserved;
  };
}

#endif  // _QIMESSAGING_URL_HPP_
