/*
**
** Copyright (C) 2012 Aldebaran Robotics
*/

#include <src/metamethod_p.hpp>

namespace qi {

MetaMethodPrivate::MetaMethodPrivate() :
  _signature(""),
  _functor(NULL),
  _idx(0)
  {
  }

  MetaMethodPrivate::MetaMethodPrivate(const std::string &sig, const qi::Functor *functor) :
  _signature(sig),
  _functor(functor),
  _idx(0)
  {
  }

};