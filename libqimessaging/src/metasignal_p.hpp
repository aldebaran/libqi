/*
**
** Copyright (C) 2012 Aldebaran Robotics
*/

#include <iostream>
#include <string>
#include <qimessaging/api.hpp>
#include <qimessaging/metasignal.hpp>
#include <qimessaging/signature.hpp>
#include <qimessaging/future.hpp>


#ifndef __METASIGNAL_P_HPP__
#define __METASIGNAL_P_HPP__

namespace qi {

  class Object;
  class MetaSignalPrivate {
  public:
    explicit MetaSignalPrivate(const std::string &sig);
    MetaSignalPrivate();
    MetaSignalPrivate(const MetaSignalPrivate &rhs);
    MetaSignalPrivate &operator=(const MetaSignalPrivate &rhs);

    const std::string &signature() const { return _signature; }
    unsigned int      uid() const { return _uid; }

  public:
    std::string        _signature;
    unsigned int       _uid;
  };

};

#endif
