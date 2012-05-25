/*
**
** Copyright (C) 2012 Aldebaran Robotics
*/

#include <boost/algorithm/string/predicate.hpp>
#include "src/object_p.hpp"

namespace qi {

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