/*
**
** Copyright (C) 2012 Aldebaran Robotics
*/

#include <iostream>
#include <string>
#include <qimessaging/api.hpp>
#include <qimessaging/metaevent.hpp>
#include <qimessaging/signature.hpp>
#include <qimessaging/future.hpp>


#ifndef __METAEVENT_P_HPP__
#define __METAEVENT_P_HPP__

namespace qi {

  class Object;
  class MetaEventPrivate {
  public:
    explicit MetaEventPrivate(const std::string &sig);
    MetaEventPrivate();
    MetaEventPrivate(const MetaEventPrivate &rhs);
    MetaEventPrivate &operator=(const MetaEventPrivate &rhs);

    const std::string &signature() const { return _signature; }
    unsigned int      uid() const { return _uid; }

  public:
    std::string        _signature;
    unsigned int       _uid;
  };

};

#endif
