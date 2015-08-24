#pragma once
/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/

#include <string>
#include <qi/api.hpp>
#include <qi/type/metasignal.hpp>
#include <qi/signature.hpp>
#include <qi/future.hpp>


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
