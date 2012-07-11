/*
** Author(s):
**  - Herve Cuche <hcuche@aldebaran-robotics.com>
**
** Copyright (C) 2012 Aldebaran Robotics
*/

#pragma once
#ifndef _QIMESSAGING_SERVICE_DIRECTORY_HPP_
#define _QIMESSAGING_SERVICE_DIRECTORY_HPP_

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
    bool close();
    void join();
    qi::Url listenUrl() const;

  private:
    ServiceDirectoryPrivate *_p;
  }; // !ServiceDirectory
}; // !qi

#endif  // _QIMESSAGING_SERVICE_DIRECTORY_HPP_
