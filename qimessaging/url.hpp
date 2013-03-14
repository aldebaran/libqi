#pragma once
/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/

#ifndef _QIMESSAGING_URL_HPP_
#define _QIMESSAGING_URL_HPP_

#include <string>
#include <vector>

#include <qimessaging/api.hpp>

namespace qi {
  class UrlPrivate;
  class QIMESSAGING_API Url
  {
  public:
    Url();
    Url(const std::string &url);
    Url(const char *url);

    bool operator ==(const Url& url);

    virtual ~Url();

    Url(const qi::Url& url);
    Url& operator= (const Url& rhs);

    bool operator< (const Url& rhs) const;

    bool isValid() const;
    const std::string& str() const;

    const std::string& protocol() const;
    const std::string& host() const;
    unsigned short port() const;

    UrlPrivate* _p;
  };

  typedef std::vector<Url> UrlVector;
}

#endif  // _QIMESSAGING_URL_HPP_
