/*
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2012 Aldebaran Robotics
*/

#ifndef    METAOBJECT_P_HPP_
# define   METAOBJECT_P_HPP_

#include <qi/atomic.hpp>
#include <qimessaging/metaevent.hpp>
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

    boost::recursive_mutex              _mutexEvent;
    boost::recursive_mutex              _mutexMethod;
    // Global uid for event subscribers.
    static qi::atomic<long> uid;
  };

}

#endif /* !METAOBJECT_P_PP_ */
