/*
**
** Copyright (C) 2012 Aldebaran Robotics
*/

#include <iostream>
#include <string>
#include <qimessaging/api.hpp>
#include <qimessaging/details/makefunctor.hpp>
#include <qimessaging/signature.hpp>
#include <qimessaging/future.hpp>

#ifndef __METAMETHOD_P_HPP__
#define __METAMETHOD_P_HPP__

namespace qi {

  class MetaMethodPrivate {
  public:
    MetaMethodPrivate(const std::string &sig, const qi::Functor *functor);
    MetaMethodPrivate();

    const std::string &signature() const { return _signature; }
    unsigned int      index() const { return _idx; }

  protected:
  public:
    QI_API std::string        _signature;
    const qi::Functor *_functor;
    unsigned int       _idx;
  };

};

#endif