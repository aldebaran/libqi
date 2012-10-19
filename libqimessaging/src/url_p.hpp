#pragma once
/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/

#ifndef _SRC_URL_P_HPP_
#define _SRC_URL_P_HPP_

#include <string>

namespace qi {
  class UrlPrivate {
  public:
    UrlPrivate();
    UrlPrivate(const UrlPrivate* url_p);
    UrlPrivate(const std::string& url);
    UrlPrivate(const char* url);

    const std::string& str() const;
    bool isValid() const;

    std::string    url;
    std::string    protocol;
    std::string    host;
    unsigned short port;

  private:
    void split_me(const std::string& url);
  };
}

#endif // _SRC_URL_P_HPP_
