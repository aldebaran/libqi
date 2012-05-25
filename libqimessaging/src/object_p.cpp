/*
**
** Copyright (C) 2012 Aldebaran Robotics
*/

#include <boost/algorithm/string/predicate.hpp>
#include "src/object_p.hpp"

namespace qi {

  MetaObjectPrivate::MetaObjectPrivate(const MetaObjectPrivate &rhs)
  {
    _methodsNameToIdx = rhs._methodsNameToIdx;
    _methods = rhs._methods;
    _methodsNumber = rhs._methodsNumber;
  }

  MetaObjectPrivate&  MetaObjectPrivate::operator=(const MetaObjectPrivate &rhs)
  {
    _methodsNameToIdx = rhs._methodsNameToIdx;
    _methods = rhs._methods;
    _methodsNumber = rhs._methodsNumber;
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

};