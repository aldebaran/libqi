#pragma once
/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/

#ifndef _QIMESSAGING_SERVICEDIRECTORY_HPP_
#define _QIMESSAGING_SERVICEDIRECTORY_HPP_

#include <qimessaging/url.hpp>

namespace qi
{
  class ServiceDirectoryPrivate;

  class QIMESSAGING_API ServiceDirectory
  {
  public:
    ServiceDirectory();
    ~ServiceDirectory();

    bool listen(const qi::Url &listenAddress);
    void close();
    qi::Url listenUrl() const;
    std::vector<qi::Url> endpoints() const;

  private:
    ServiceDirectoryPrivate *_p;
  }; // !ServiceDirectory
} // !qi

#endif  // _QIMESSAGING_SERVICEDIRECTORY_HPP_
