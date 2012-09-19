#pragma once
/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/

#ifndef _SRC_METAOBJECT_P_HPP_
#define _SRC_METAOBJECT_P_HPP_

#include <qi/atomic.hpp>
#include <qimessaging/metasignal.hpp>
#include <qimessaging/metafunction.hpp>
#include <qimessaging/metamethod.hpp>
#include <boost/thread/recursive_mutex.hpp>

namespace qi {

  class MetaObjectPrivate {
  public:
    MetaObjectPrivate()
    :  _nextNumber(0)
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

    inline int signalId(const std::string &name) {
      return idFromName(_eventsNameToIdx, name);
    }

    std::vector<MetaMethod> findMethod(const std::string &name);
    std::vector<MetaSignal> findSignal(const std::string &name);

    /*
     * When a member is added, serialization and deserialization
     * operators _MUST_ be updated.
     */
    NameToIdx                           _methodsNameToIdx;
    MetaObject::MethodMap               _methods;

    unsigned int                        _nextNumber;

    NameToIdx                           _eventsNameToIdx;
    MetaObject::SignalMap                _events;

    // Recompute data cached in *ToIdx
    void refreshCache();

    boost::recursive_mutex              _mutexEvent;
    boost::recursive_mutex              _mutexMethod;
    // Global uid for event subscribers.
    static qi::atomic<long> uid;
  };

}

#endif  // _SRC_METAOBJECT_P_HPP_
