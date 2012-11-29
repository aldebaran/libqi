#pragma once
/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/

#ifndef _SRC_METAOBJECT_P_HPP_
#define _SRC_METAOBJECT_P_HPP_

#include <qi/atomic.hpp>
#include <qitype/metasignal.hpp>
#include <qitype/metamethod.hpp>
#include <boost/thread/recursive_mutex.hpp>

namespace qi {

  class MetaObjectPrivate {
  public:
    //by default we start at 10, the first 10 id are reserved for bound object
    MetaObjectPrivate()
      : _index(9)
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

    //if you want to use thoose methods thinks twice...
    //they are only useful to merge too metaobject and I bet
    //you really dont want to do that.
    bool addMethods(const MetaObject::MethodMap &mms);
    bool addSignals(const MetaObject::SignalMap &mms);

    std::vector<MetaMethod> findMethod(const std::string &name);
    std::vector<MetaMethod> findCompatibleMethod(const std::string &nameOrSignature);

    std::vector<MetaSignal> findSignal(const std::string &name);

    unsigned int addMethod(const std::string& sigret, const std::string& signature, int id = -1);
    unsigned int addSignal(const std::string &sig, int id = -1);

    // Recompute data cached in *ToIdx
    void refreshCache();

  private:
    friend class MetaObject;

  private:
    /*
     * When a member is added, serialization and deserialization
     * operators _MUST_ be updated.
     */
    NameToIdx                           _methodsNameToIdx;
    MetaObject::MethodMap               _methods;
    mutable boost::recursive_mutex      _methodsMutex;

    NameToIdx                           _eventsNameToIdx;
    MetaObject::SignalMap               _events;
    mutable boost::recursive_mutex      _eventsMutex;

    qi::Atomic<unsigned int>            _index;
    // Global uid for event subscribers.
    static qi::Atomic<int> uid;
    friend class TypeImpl<MetaObjectPrivate>;
    friend class TypeImpl<MetaObject>;
  };

}

#endif  // _SRC_METAOBJECT_P_HPP_
