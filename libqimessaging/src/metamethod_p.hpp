/*
**
** Copyright (C) 2012 Aldebaran Robotics
*/

#include <iostream>
#include <string>
#include <qimessaging/api.hpp>
#include <qimessaging/signature.hpp>
#include <qimessaging/future.hpp>
#include <qimessaging/metafunction.hpp>

#ifndef __METAMETHOD_P_HPP__
#define __METAMETHOD_P_HPP__

namespace qi {

  class MetaMethodPrivate {
  public:
    MetaMethodPrivate(const std::string &sigret, const std::string &sig, MetaFunction functor);
    MetaMethodPrivate();
    MetaMethodPrivate(const MetaMethodPrivate &rhs);
    MetaMethodPrivate &operator=(const MetaMethodPrivate &rhs);

    const std::string &signature() const { return _signature; }
    const std::string &sigreturn() const { return _sigret; }
    unsigned int      uid() const { return _uid; }

  protected:
  public:
    std::string        _signature;
    std::string        _sigret;
    MetaFunction       _functor;
    unsigned int       _uid;
  };

};

#endif
