/*
**
** Copyright (C) 2012 Aldebaran Robotics
*/

#include <boost/thread/mutex.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include "src/object_p.hpp"
#include "src/metaobjectbuilder_p.hpp"

namespace qi {

  ObjectPrivate::ObjectPrivate()
    : _dying(false)
    , _eventLoop(getDefaultObjectEventLoop())
    , _meta(new qi::MetaObject)
    , _builder(_meta) {
  }

  ObjectPrivate::~ObjectPrivate() {
    _dying = true;
    delete _meta;
  }

  void ObjectPrivate::setMetaObject(qi::MetaObject *mo) {
    //allocate by caller, but owned by us then
    delete _meta;
    _meta = mo;
    _builder._p->_metaObject = mo;
  }

}
