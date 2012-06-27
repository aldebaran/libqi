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
#include <qimessaging/metamethod.hpp>

#ifndef _QIMESSAGING_OBJECTPRIVATE_HPP_
#define _QIMESSAGING_OBJECTPRIVATE_HPP_

namespace qi {

  class MetaObjectPrivate {
  public:
    MetaObjectPrivate() :
      _methodsNumber(0)
    {
    };

    MetaObjectPrivate(const MetaObjectPrivate &rhs);
    MetaObjectPrivate&  operator=(const MetaObjectPrivate &rhs);

    inline int methodId(const std::string &name) {
      std::map<std::string, unsigned int>::iterator it;
      it = _methodsNameToIdx.find(name);
      if (it == _methodsNameToIdx.end())
        return -1;
      return it->second;
    }

    inline const std::vector<MetaMethod> &methods() const { return _methods; }
    inline std::vector<MetaMethod> &methods() { return _methods; }
    std::vector<MetaMethod> findMethod(const std::string &name);
    /*
     * When a member is added, serialization and deserialization
     * operators _MUST_ be updated.
     */
    std::map<std::string, unsigned int> _methodsNameToIdx;
    std::vector<MetaMethod>             _methods;
    unsigned int                        _methodsNumber;
    // std::map<std::string, MethodInfo>   _signals;
    // std::map<std::string, MethodInfo>   _slots;
    // std::map<std::string, PropertyInfo> _properties;
  };

};

#endif
