/*
**
** Copyright (C) 2012 Aldebaran Robotics
*/

#ifndef _QIMESSAGING_OBJECTPRIVATE_HPP_
#define _QIMESSAGING_OBJECTPRIVATE_HPP_

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
#include <qimessaging/signal.hpp>

namespace qi {

  class EventLoop;
  class ObjectPrivate {
  public:
    ObjectPrivate();
    ObjectPrivate(const qi::MetaObject &meta);
    virtual ~ObjectPrivate();

    void addCallbacks(ObjectInterface *callbacks, void *data = 0);
    void removeCallbacks(ObjectInterface *callbacks);

    virtual qi::Future<MetaFunctionResult> metaCall(unsigned int method, const MetaFunctionParameters& params, qi::Object::MetaCallType callType = qi::Object::MetaCallType_Auto);
    qi::Future<MetaFunctionResult> xMetaCall(const std::string &retsig, const std::string &signature, const MetaFunctionParameters& params);

    void emitEvent(const std::string& eventName,
                   qi::AutoValue p1 = qi::AutoValue(),
                   qi::AutoValue p2 = qi::AutoValue(),
                   qi::AutoValue p3 = qi::AutoValue(),
                   qi::AutoValue p4 = qi::AutoValue(),
                   qi::AutoValue p5 = qi::AutoValue(),
                   qi::AutoValue p6 = qi::AutoValue(),
                   qi::AutoValue p7 = qi::AutoValue(),
                   qi::AutoValue p8 = qi::AutoValue());

    virtual void metaEmit(unsigned int event, const MetaFunctionParameters& params);
    bool xMetaEmit(const std::string &signature, const MetaFunctionParameters &in);
    template <typename FUNCTOR_TYPE>
    unsigned int connect(const std::string& eventName, FUNCTOR_TYPE callback,
                         EventLoop* ctx = getDefaultObjectEventLoop());
    unsigned int xConnect(const std::string &signature, MetaFunction functor,
                          EventLoop* ctx = getDefaultObjectEventLoop());
    unsigned int connect(unsigned int event, MetaFunction Functor,
                         EventLoop* ctx = getDefaultObjectEventLoop());
    virtual unsigned int connect(unsigned int event, const SignalSubscriber& subscriber);
    virtual bool disconnect(unsigned int linkId);
    std::vector<SignalSubscriber> subscribers(int eventId) const;
    unsigned int connect(unsigned int signal, qi::Object target, unsigned int slot);
    void trigger(unsigned int event, const MetaFunctionParameters& params);
    void moveToEventLoop(EventLoop* ctx);
    EventLoop* eventLoop();

    //TODO: remove, this should move to metatype.
    inline MetaObject &metaObject() { return _meta; }

  public:
    // Links that target us. Needed to be able to disconnect upon destruction
    std::vector<SignalSubscriber>       _registrations;
    boost::recursive_mutex              _mutexRegistration;

  protected:
    std::map<ObjectInterface *, void *> _callbacks;
    boost::mutex                        _callbacksMutex;
    bool                                _dying;

    typedef std::map<unsigned int, SignalBase*> SignalSubscriberMap;
    //eventid -> linkid -> SignalSubscriber
    SignalSubscriberMap _subscribers;

    // Event loop in which calls are made
    EventLoop                          *_eventLoop;
    Value  _instance;
  private:
    qi::MetaObject _meta;
  };

};

#endif
