#pragma once
/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/

#include <iostream>
#include <string>
#include <qimessaging/api.hpp>
#include <qimessaging/metasignal.hpp>
#include <qimessaging/signature.hpp>
#include <qimessaging/future.hpp>


#ifndef _SRC_METASIGNAL_P_HPP_
#define _SRC_METASIGNAL_P_HPP_

namespace qi {

  class GenericObject;
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

#endif  // _SRC_METASIGNAL_P_HPP_
