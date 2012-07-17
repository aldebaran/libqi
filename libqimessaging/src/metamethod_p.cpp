/*
**
** Copyright (C) 2012 Aldebaran Robotics
*/

#include <src/metamethod_p.hpp>

namespace qi {

  MetaMethodPrivate::MetaMethodPrivate()
    : _signature(),
      _sigret(),
      _functor(),
      _idx(0)
  {
  }

  MetaMethodPrivate::MetaMethodPrivate(const std::string &sigret, const std::string &sig, const qi::Functor *functor)
    : _signature(sig),
      _sigret(sigret),
      _functor(functor),
      _idx(0)
  {
  }

  MetaMethodPrivate::MetaMethodPrivate(const MetaMethodPrivate &rhs)
  {
    _signature = rhs._signature;
    _sigret = rhs._sigret;
    _functor = rhs._functor;
    _idx = rhs._idx;
  }

  MetaMethodPrivate& MetaMethodPrivate::operator=(const MetaMethodPrivate &rhs)
  {
    _signature = rhs._signature;
    _sigret = rhs._sigret;
    _functor = rhs._functor;
    _idx = rhs._idx;
    return (*this);
  }

}
