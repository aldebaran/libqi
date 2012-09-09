/*
**
** Copyright (C) 2012 Aldebaran Robotics
*/

#include <iostream>
#include <string>
#include <boost/thread/recursive_mutex.hpp>

#include <qi/atomic.hpp>

#include <qimessaging/api.hpp>
#include <qimessaging/signature.hpp>
#include <qimessaging/future.hpp>
#include <qimessaging/object.hpp>
#include <qimessaging/metasignal.hpp>
#include <qimessaging/metamethod.hpp>
#include <qimessaging/metaobjectbuilder.hpp>
#include <qimessaging/signal.hpp>
#ifndef _QIMESSAGING_OBJECTPRIVATE_HPP_
#define _QIMESSAGING_OBJECTPRIVATE_HPP_

namespace qi {



  class EventLoop;
  class ObjectPrivate {
  public:
    ObjectPrivate();
    ~ObjectPrivate();

    std::map<ObjectInterface *, void *> _callbacks;
    boost::mutex                        _callbacksMutex;
    bool                                _dying;

    // Links that target us. Needed to be able to disconnect upon destruction
    std::vector<SignalSubscriber>             _registrations;
    boost::recursive_mutex              _mutexRegistration;
    // Event loop in which calls are made
    EventLoop                          *_eventLoop;

    typedef std::map<unsigned int, SignalBase*> SignalSubscriberMap;
    //eventid -> linkid -> SignalSubscriber
    SignalSubscriberMap _subscribers;


    void setMetaObject(qi::MetaObject *);

    inline MetaObject* metaObject() { return _meta; }

  private:
    MetaObject                         *_meta;

  public:
    MetaObjectBuilder                   _builder;
  };

};

#endif
