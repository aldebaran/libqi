/*
**
** Copyright (C) 2012 Aldebaran Robotics
*/

#include <boost/algorithm/string/predicate.hpp>
#include "src/object_p.hpp"

namespace qi {

  MetaObjectPrivate::MetaObjectPrivate(const MetaObjectPrivate &rhs)
  {
    (*this) = rhs;
  }

  MetaObjectPrivate&  MetaObjectPrivate::operator=(const MetaObjectPrivate &rhs)
  {
    _methodsNameToIdx = rhs._methodsNameToIdx;
    _methods = rhs._methods;
    _methodsNumber = rhs._methodsNumber;
    _events = rhs._events;
    _eventsNumber = rhs._eventsNumber;
    return (*this);
  }

  std::vector<qi::MetaMethod> MetaObjectPrivate::findMethod(const std::string &name)
  {
    std::vector<qi::MetaMethod>           ret;
    std::vector<qi::MetaMethod>::iterator it;
    std::string cname(name);
    cname += "::";

    for (it = _methods.begin(); it != _methods.end(); ++it) {
      qi::MetaMethod &mm = *it;
      if (boost::starts_with(mm.signature(), cname))
        ret.push_back(mm);
    }
    return ret;
  }

  std::vector<MetaEvent> MetaObjectPrivate::findEvent(const std::string &name)
  {
    std::vector<MetaEvent>           ret;
    std::vector<MetaEvent>::iterator it;
    std::string cname(name);
    cname += "::";

    for (it = _events.begin(); it != _events.end(); ++it) {
      MetaEvent &mm = *it;
      if (boost::starts_with(mm.signature(), cname))
        ret.push_back(mm);
    }
    return ret;
  }

  qi::atomic<long> MetaObjectPrivate::uid = 1;
};