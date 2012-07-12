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
#include <qimessaging/metaevent.hpp>
#include <qimessaging/metamethod.hpp>

#ifndef _QIMESSAGING_OBJECTPRIVATE_HPP_
#define _QIMESSAGING_OBJECTPRIVATE_HPP_

namespace qi {

  class MetaObjectPrivate {
  public:
    MetaObjectPrivate()
    :  _methodsNumber(0)
    , _eventsNumber(0)
    {
    };

    MetaObjectPrivate(const MetaObjectPrivate &rhs);
    MetaObjectPrivate&  operator=(const MetaObjectPrivate &rhs);

    typedef std::map<std::string, unsigned int> NameToIdx;

    inline int idFromName(const NameToIdx& map, const std::string& name) {
      NameToIdx::const_iterator it = map.find(name);
      if (it == map.end())
        return -1;
      else
        return it->second;
    }

    inline int methodId(const std::string &name) {
      return idFromName(_methodsNameToIdx, name);
    }

    inline int eventId(const std::string &name) {
      return idFromName(_eventsNameToIdx, name);
    }

    inline const std::vector<MetaMethod> &methods() const { return _methods; }
    inline std::vector<MetaMethod> &methods() { return _methods; }
    std::vector<MetaMethod> findMethod(const std::string &name);
    std::vector<MetaEvent> findEvent(const std::string &name);
    /*
     * When a member is added, serialization and deserialization
     * operators _MUST_ be updated.
     */
    NameToIdx                           _methodsNameToIdx;
    std::vector<MetaMethod>             _methods;

    unsigned int                        _methodsNumber;

    inline const std::vector<MetaEvent> &events() const { return _events; }
    inline std::vector<MetaEvent> &events() { return _events; }


    NameToIdx                           _eventsNameToIdx;
    std::vector<MetaEvent>              _events;
    unsigned int                        _eventsNumber;

    // Links that target us. Needed to be able to disconnec upon destruction
    std::vector<MetaEvent::Subscriber>  _registrations;
    // Recompute data cached in *ToIdx
    void refreshCache();

    // std::map<std::string, MethodInfo>   _signals;
    // std::map<std::string, MethodInfo>   _slots;
    // std::map<std::string, PropertyInfo> _properties;

    // Global uid for event subscribers.
    static qi::atomic<long> uid;
  };

};

#endif
