/*
**
** Copyright (C) 2012 Aldebaran Robotics
*/

#include <boost/thread/mutex.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include "src/object_p.hpp"
#include "src/objectbuilder_p.hpp"

namespace qi {

  ObjectPrivate::ObjectPrivate()
    : _dying(false)
    , _eventLoop(getDefaultObjectEventLoop())
  {
  }

  ObjectPrivate::ObjectPrivate(const qi::MetaObject &meta)
    : _dying(false)
    , _eventLoop(getDefaultObjectEventLoop())
    , _meta(meta)
  {
  }


  ObjectPrivate::~ObjectPrivate() {
    _dying = true;
  }

}
