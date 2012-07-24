/*
**
** Copyright (C) 2012 Aldebaran Robotics
*/

#include <src/metaevent_p.hpp>
#include <qimessaging/object.hpp>

namespace qi {

  MetaEventPrivate::MetaEventPrivate()
    : _signature(),
      _uid(0)
  {
  }

  MetaEventPrivate::MetaEventPrivate(const std::string &sig)
    : _signature(sig),
      _uid(0)
  {
  }

  MetaEventPrivate::MetaEventPrivate(const MetaEventPrivate &rhs)
  {
    _signature = rhs._signature;
    _uid = rhs._uid;
  }

  MetaEventPrivate& MetaEventPrivate::operator=(const MetaEventPrivate &rhs)
  {
    _signature = rhs._signature;
    _uid = rhs._uid;
    return (*this);
  }

}
