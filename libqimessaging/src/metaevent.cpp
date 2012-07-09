/*
**
** Copyright (C) 2012 Aldebaran Robotics
*/

#include "src/metaevent_p.hpp"
#include <qimessaging/metaevent.hpp>

namespace qi {

  MetaEvent::MetaEvent()
    : _p(new MetaEventPrivate())
  {
  }

  MetaEvent::MetaEvent(const std::string &sig)
    : _p(new MetaEventPrivate(sig))
  {
  }

  MetaEvent::MetaEvent(const MetaEvent &other)
    : _p(new MetaEventPrivate())
  {
    *_p = *(other._p);
  }

  MetaEvent& MetaEvent::operator=(const MetaEvent &other)
  {
    *_p = *(other._p);
    return (*this);
  }

  MetaEvent::~MetaEvent()
  {
    delete _p;
  }

  const std::string &MetaEvent::signature() const
  {
    return _p->signature();
  }

  unsigned int       MetaEvent::index() const
  {
    return _p->index();
  }

};
