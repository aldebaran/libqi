/*
**
** Copyright (C) 2012 Aldebaran Robotics
*/

#include <iostream>
#include <string>
#include <boost/thread/recursive_mutex.hpp>
#include <qimessaging/api.hpp>
#include <qimessaging/details/makefunctor.hpp>
#include <qimessaging/signature.hpp>
#include <qimessaging/future.hpp>
#include <qimessaging/object.hpp>
#include <qimessaging/metaevent.hpp>
#include <qimessaging/metamethod.hpp>

#ifndef _QIMESSAGING_OBJECTPRIVATE_HPP_
#define _QIMESSAGING_OBJECTPRIVATE_HPP_

namespace qi {



  class EventLoop;
  class ObjectPrivate {
  public:
    ObjectPrivate();
    ~ObjectPrivate();

    MetaObject                         *_meta;
    std::map<ObjectInterface *, void *> _callbacks;
    boost::mutex                        _callbacksMutex;
    bool                                _dying;

    // Links that target us. Needed to be able to disconnect upon destruction
    std::vector<EventSubscriber>  _registrations;
    boost::recursive_mutex              _mutexRegistration;
    // Event loop in which calls are made
    EventLoop                          *_eventLoop;

    typedef std::map<unsigned int, EventSubscriber> SubscriberMap;
    //eventid -> linkid -> Subscriber
    std::map<unsigned int, SubscriberMap> _subscribers;

  };

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

    inline int eventId(const std::string &name) {
      return idFromName(_eventsNameToIdx, name);
    }

    std::vector<MetaMethod> findMethod(const std::string &name);
    std::vector<MetaEvent> findEvent(const std::string &name);

    /*
     * When a member is added, serialization and deserialization
     * operators _MUST_ be updated.
     */
    NameToIdx                           _methodsNameToIdx;
    MetaObject::MethodMap               _methods;

    unsigned int                        _nextNumber;

    NameToIdx                           _eventsNameToIdx;
    MetaObject::EventMap                _events;

    // Recompute data cached in *ToIdx
    void refreshCache();

    // std::map<std::string, MethodInfo>   _signals;
    // std::map<std::string, MethodInfo>   _slots;
    // std::map<std::string, PropertyInfo> _properties;

    boost::recursive_mutex              _mutexEvent;
    boost::recursive_mutex              _mutexMethod;
    // Global uid for event subscribers.
    static qi::atomic<long> uid;
  };

};

#endif
