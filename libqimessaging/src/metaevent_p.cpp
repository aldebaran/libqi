/*
**
** Copyright (C) 2012 Aldebaran Robotics
*/

#include <src/metaevent_p.hpp>
#include <qimessaging/object.hpp>

namespace qi {

  MetaEventPrivate::MetaEventPrivate()
    : _signature(),
      _idx(0)
  {
  }

  MetaEventPrivate::MetaEventPrivate(const std::string &sig)
    : _signature(sig),
      _idx(0)
  {
  }

  MetaEventPrivate::MetaEventPrivate(const MetaEventPrivate &rhs)
  {
    _signature = rhs._signature;
    _idx = rhs._idx;
  }

  MetaEventPrivate& MetaEventPrivate::operator=(const MetaEventPrivate &rhs)
  {
    _signature = rhs._signature;
    _idx = rhs._idx;
    return (*this);
  }

}
