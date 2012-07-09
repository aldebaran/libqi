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

#ifndef __METAEVENT_P_HPP__
#define __METAEVENT_P_HPP__

namespace qi {

  class MetaEventPrivate {
  public:
    explicit MetaEventPrivate(const std::string &sig);
    MetaEventPrivate();
    MetaEventPrivate(const MetaEventPrivate &rhs);
    MetaEventPrivate &operator=(const MetaEventPrivate &rhs);

    const std::string &signature() const { return _signature; }
    unsigned int      index() const { return _idx; }

    struct Subscriber
    {
      const Functor* handler;
    };
    // Subscriber indexed by link id for one given event
    typedef std::map<unsigned int, Subscriber> Subscribers;

  protected:
  public:
    Subscribers        _subscribers;
    std::string        _signature;
    unsigned int       _idx;
  };

};

#endif
