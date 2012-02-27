/*
** Author(s):
**  - Herve Cuche <hcuche@aldebaran-robotics.com>
**
** Copyright (C) 2012 Aldebaran Robotics
*/

#pragma once
#ifndef _QIMESSAGING_SERVICE_DIRECTORY_HPP_
#define _QIMESSAGING_SERVICE_DIRECTORY_HPP_

#include <string>

namespace qi
{
  class NetworkThread;
  class Object;
  class ServiceDirectoryPrivate;


  class ServiceDirectory
  {
  public:
    ServiceDirectory();
    virtual ~ServiceDirectory();

    bool listen(const std::string &address);
    void join();

  private:
    ServiceDirectoryPrivate *_p;

  }; // !ServiceDirectory
}; // !qi

#endif  // _QIMESSAGING_SERVICE_DIRECTORY_HPP_
