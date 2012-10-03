#pragma once
/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/

#ifndef _SRC_METAOBJECT_P_HPP_
#define _SRC_METAOBJECT_P_HPP_

#include <qi/atomic.hpp>
#include <qimessaging/metasignal.hpp>
#include <qimessaging/metamethod.hpp>
#include <boost/thread/recursive_mutex.hpp>

namespace qi {

  class MetaObjectPrivate {
  public:
    MetaObjectPrivate()
      : _index(-1)
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

    unsigned int addMethod(const std::string& sigret, const std::string& signature);
    unsigned int addSignal(const std::string &sig);

    // Recompute data cached in *ToIdx
    void refreshCache();

  private:
    friend class MetaObject;
    friend qi::ODataStream &operator<<(qi::ODataStream &stream, const MetaObject &meta);
    friend qi::IDataStream &operator>>(qi::IDataStream &stream, MetaObject &meta);
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

    qi::atomic<unsigned int>            _index;
    // Global uid for event subscribers.
    static qi::atomic<long> uid;
    friend class TypeImpl<MetaObjectPrivate>;
  };

}

#endif  // _SRC_METAOBJECT_P_HPP_
