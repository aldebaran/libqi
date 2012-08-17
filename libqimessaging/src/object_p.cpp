/*
**
** Copyright (C) 2012 Aldebaran Robotics
*/

#include <boost/thread/mutex.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include "src/object_p.hpp"

namespace qi {

  ObjectPrivate::ObjectPrivate()
    : _meta(new qi::MetaObject)
    , _dying(false)
    , _eventLoop(getDefaultObjectEventLoop()) {
  }

  ObjectPrivate::~ObjectPrivate() {
    _dying = true;
    delete _meta;
  }

  MetaObjectPrivate::MetaObjectPrivate(const MetaObjectPrivate &rhs)
  {
    (*this) = rhs;
  }

  MetaObjectPrivate&  MetaObjectPrivate::operator=(const MetaObjectPrivate &rhs)
  {
    _methodsNameToIdx = rhs._methodsNameToIdx;
    _methods = rhs._methods;
    _events = rhs._events;
    _nextNumber = rhs._nextNumber;
    return (*this);
  }

  std::vector<qi::MetaMethod> MetaObjectPrivate::findMethod(const std::string &name)
  {
    boost::recursive_mutex::scoped_lock sl(_mutexMethod);
    std::vector<qi::MetaMethod>           ret;
    MetaObject::MethodMap::iterator it;
    std::string cname(name);
    cname += "::";

    for (it = _methods.begin(); it != _methods.end(); ++it) {
      qi::MetaMethod &mm = it->second;
      if (boost::starts_with(mm.signature(), cname))
        ret.push_back(mm);
    }
    return ret;
  }

  std::vector<MetaEvent> MetaObjectPrivate::findEvent(const std::string &name)
  {
    boost::recursive_mutex::scoped_lock sl(_mutexEvent);
    std::vector<MetaEvent>           ret;
    MetaObject::EventMap::iterator it;
    std::string cname(name);
    cname += "::";

    for (it = _events.begin(); it != _events.end(); ++it) {
      MetaEvent &mm = it->second;
      if (boost::starts_with(mm.signature(), cname))
        ret.push_back(mm);
    }
    return ret;
  }

  qi::atomic<long> MetaObjectPrivate::uid = 1;
};
