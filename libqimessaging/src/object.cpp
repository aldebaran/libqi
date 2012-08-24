/*
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2012 Aldebaran Robotics
*/

#include <iostream>
#include <boost/algorithm/string/predicate.hpp>
#include "src/metamethod_p.hpp"
#include "src/metaevent_p.hpp"
#include "src/object_p.hpp"
#include <qimessaging/object.hpp>
#include <qimessaging/event_loop.hpp>

namespace qi {

  ObjectInterface::~ObjectInterface() {
  }

  MetaObject::MetaObject()
  {
    _p = new MetaObjectPrivate();
  }

  MetaObject::MetaObject(const MetaObject &other)
  {
    _p = new MetaObjectPrivate();
    *_p = *(other._p);
  }

  MetaObject& MetaObject::operator=(const MetaObject &other)
  {
    _p = new MetaObjectPrivate();
    *_p = *(other._p);
    return (*this);
  }

  MetaMethod *MetaObject::method(unsigned int id) {
    boost::recursive_mutex::scoped_lock sl(_p->_mutexMethod);
    MethodMap::iterator i = _p->_methods.find(id);
    if (i == _p->_methods.end())
      return 0;
    return &i->second;
  }

  const MetaMethod *MetaObject::method(unsigned int id) const {
    boost::recursive_mutex::scoped_lock sl(_p->_mutexMethod);
    MethodMap::const_iterator i = _p->_methods.find(id);
    if (i == _p->_methods.end())
      return 0;
    return &i->second;
  }

  MetaEvent *MetaObject::event(unsigned int id) {
    boost::recursive_mutex::scoped_lock sl(_p->_mutexEvent);
    EventMap::iterator i = _p->_events.find(id);
    if (i == _p->_events.end())
      return 0;
    return &i->second;
  }

  const MetaEvent *MetaObject::event(unsigned int id) const {
    boost::recursive_mutex::scoped_lock sl(_p->_mutexEvent);
    EventMap::const_iterator i = _p->_events.find(id);
    if (i == _p->_events.end())
      return 0;
    return &i->second;
  }

  int MetaObject::methodId(const std::string &name)
  {
    return _p->methodId(name);
  }

  int MetaObject::eventId(const std::string &name)
  {
    return _p->eventId(name);
  }

  MetaObject::MethodMap MetaObject::methods() const {
    return _p->_methods;
  }

  MetaObject::EventMap MetaObject::events() const {
    return _p->_events;
  }


  MetaObject::~MetaObject()
  {
    delete _p;
  }

  Object::Object()
    : _p(new ObjectPrivate())
  {
    advertiseMethod("__metaobject", this, &Object::metaObject);
  }

  Object::~Object() {
    {
      _p->_dying = true;
      {
        boost::recursive_mutex::scoped_lock sl(_p->_mutexRegistration);

        // Notify events that have subscribers that call us that we are dead
        // We need to make a copy of the vector, since disconnect will
        // remove entries from it.
        std::vector<EventSubscriber> regs = _p->_registrations;
        std::vector<EventSubscriber>::iterator i;
        for (i = regs.begin(); i != regs.end(); ++i)
          i->eventSource->disconnect(i->linkId);
      }
      // First get the link ids, then disconnect as calling disconnect
      // while iterating might kill us (if subscriber is on same object).
      std::vector<unsigned int> links;
      {
        boost::recursive_mutex::scoped_lock sl(metaObject()._p->_mutexEvent);
        // Then remove _registrations with one of our event as source
        //MetaObject::EventMap::iterator ii;
        std::map<unsigned int, ObjectPrivate::SubscriberMap>::iterator ii;
        for (ii = _p->_subscribers.begin(); ii!= _p->_subscribers.end(); ++ii)
        {
          ObjectPrivate::SubscriberMap::iterator j;
          for (j = ii->second.begin(); j != ii->second.end(); ++j)
            links.push_back(j->second.linkId);
        }
        for (unsigned int i = 0; i < links.size(); ++i)
          disconnect(links[i]);
      }
    }
  }

  void Object::addCallbacks(ObjectInterface *callbacks, void *data)
  {
    {
      boost::mutex::scoped_lock l(_p->_callbacksMutex);
      _p->_callbacks[callbacks] = data;
    }
  }

  void Object::removeCallbacks(ObjectInterface *callbacks)
  {
    std::map<ObjectInterface *, void *>::iterator it;
    {
      boost::mutex::scoped_lock l(_p->_callbacksMutex);
      it = _p->_callbacks.find(callbacks);
      if (it != _p->_callbacks.end())
        _p->_callbacks.erase(it);
    }
  }

  MetaObject &Object::metaObject() {
    return *_p->_meta;
  }

  int Object::xForgetMethod(const std::string &meth)
  {
    boost::recursive_mutex::scoped_lock sl(metaObject()._p->_mutexMethod);
    std::map<std::string, unsigned int>::iterator it;

    it = metaObject()._p->_methodsNameToIdx.find(meth);
    if (it != metaObject()._p->_methodsNameToIdx.end())
    {
      metaObject()._p->_methodsNameToIdx.erase(it);
      return (0);
    }

    return (1);
  }

  int Object::xAdvertiseMethod(const std::string &sigret, const std::string& signature, const qi::Functor* functor) {
    boost::recursive_mutex::scoped_lock sl(metaObject()._p->_mutexMethod);

    std::map<std::string, unsigned int>::iterator it;
    it = metaObject()._p->_methodsNameToIdx.find(signature);
    if (it != metaObject()._p->_methodsNameToIdx.end())
    {
      unsigned int uid = it->second;
      MetaMethod mm(sigret, signature, functor);
      mm._p->_uid = uid;
      // find it
      metaObject()._p->_methods[uid] = mm;
      qiLogVerbose("qi.Object") << "rebinding method:" << signature;
      return uid;
    }

    MetaMethod mm(sigret, signature, functor);
    unsigned int idx = metaObject()._p->_nextNumber++;
    mm._p->_uid = idx;
    metaObject()._p->_methods[idx] = mm;
    metaObject()._p->_methodsNameToIdx[signature] = idx;
    qiLogVerbose("qi.Object") << "binding method:" << signature;
    return idx;
  }

  int Object::xAdvertiseEvent(const std::string& signature) {
    boost::recursive_mutex::scoped_lock sl(metaObject()._p->_mutexEvent);
    if (signature.empty())
    {
      qiLogError("qi.Object") << "Event has empty signature.";
      return -1;
    }
    std::map<std::string, unsigned int>::iterator it;

    it = metaObject()._p->_eventsNameToIdx.find(signature);
    if (it != metaObject()._p->_eventsNameToIdx.end())
    { // Event already there.
      qiLogError("qi.Object") << "event already there";
      return it->second;
    }
    unsigned int idx = metaObject()._p->_nextNumber++;
    MetaEvent me(signature);
    me._p->_uid = idx;
    metaObject()._p->_events[idx] = me;
    metaObject()._p->_eventsNameToIdx[signature] = idx;
    qiLogVerbose("qi.Object") << "binding event:" << signature <<" with id "
    << idx;
    return idx;
  }

  void Object::metaCall(unsigned int method, const FunctorParameters &in, qi::FunctorResult out, MetaCallType callType)
  {
    MetaMethod *mm = metaObject().method(method);
    if (!mm) {
      std::stringstream ss;
      ss << "Can't find methodID: " << method;
      out.setError(ss.str());
      return;
    }
    if (mm->_p->_functor)
    {
      bool synchronous = true;
      switch (callType)
      {
      case MetaCallType_Direct:
        break;
      case MetaCallType_Auto:
        synchronous = !_p->_eventLoop ||  _p->_eventLoop->isInEventLoopThread();
        break;
      case MetaCallType_Queued:
        synchronous = !_p->_eventLoop;
        break;
      }
      if (synchronous)
        mm->_p->_functor->call(in, out);
      else
        _p->_eventLoop->asyncCall(0,
          boost::bind(&Functor::call,
            mm->_p->_functor,
            in, out));
    }
    else {
      std::stringstream ss;
      ss << "No valid functor for methodid: " << method;
      out.setError(ss.str());
    }
  }

  class DropResult: public FunctorResultBase
  {
  public:
    virtual void setValue(const qi::Buffer &buffer) {}
    virtual void setError(const std::string &sig, const qi::Buffer &msg)
    {
      qiLogError("object") << "Event handler returned an error";
    }
  };
  void Object::metaEmit(unsigned int event, const FunctorParameters &args)
  {
    //std::cerr <<"metaemit on " << this <<" from " << pthread_self() << std::endl;
    trigger(event, args);
  }
  void Object::trigger(unsigned int event, const FunctorParameters &args)
  {
    // Note: Not thread-safe, event/method may die while we hold it.
    MetaEvent* ev = metaObject().event(event);
    if (!ev)
    {
      MetaMethod* me = metaObject().method(event);
      if (!me)
      {
        qiLogError("object") << "No such event " << event;
        return;
      }
      else
      {
        metaCall(event, args, FunctorResult(boost::shared_ptr<FunctorResultBase>(new DropResult)));
        return;
      }
    }
    boost::recursive_mutex::scoped_lock sl(metaObject()._p->_mutexEvent);
    std::map<unsigned int, ObjectPrivate::SubscriberMap>::iterator it;
    it = _p->_subscribers.find(event);
    if (it == _p->_subscribers.end())
      return;
    ObjectPrivate::SubscriberMap::iterator il;
    for (il = it->second.begin(); il != it->second.end(); ++il)
    {
      il->second.call(args);
    }
  }

  void Object::xMetaCall(const std::string &retsig, const std::string &signature, const FunctorParameters &in, FunctorResult out)
  {
    int methodId = metaObject().methodId(signature);
    if (methodId < 0) {
      std::stringstream ss;
      ss << "Can't find method: " << signature << std::endl
         << "  Candidate(s):" << std::endl;
      std::vector<qi::MetaMethod>           mml = metaObject().findMethod(qi::signatureSplit(signature)[1]);
      std::vector<qi::MetaMethod>::const_iterator it;

      for (it = mml.begin(); it != mml.end(); ++it) {
        const qi::MetaMethod       &mm = *it;
        ss << "  " << mm.signature() << std::endl;
      }
      qiLogError("object") << ss.str();
      out.setError(ss.str());
      return;
    }
    if (retsig != "v") {
      qi::MetaMethod *mm = metaObject().method(methodId);
      if (!mm) {
        std::stringstream ss;
        ss << "method " << signature << "(id: " << methodId << ") disapeared mysteriously!";
        qiLogError("object") << ss.str();
        out.setError(ss.str());
        return;
      }
      if (mm->sigreturn() != retsig) {
        std::stringstream ss;
        ss << "signature mismatch for return value:" << std::endl
                             << "we want: " << retsig << " " << signature << std::endl
                             << "we had:" << mm->sigreturn() << " " << mm->signature();
        qiLogError("object") << ss;
        out.setError(ss.str());
        return;
      }
    }
    //TODO: check for metacall to return false when not able to send the answer
    metaCall(methodId, in, out);
    return;
  }

  /// Resolve signature and bounce
  bool Object::xMetaEmit(const std::string &signature, const FunctorParameters &in) {
    int eventId = metaObject().eventId(signature);
    if (eventId < 0)
      eventId = metaObject().methodId(signature);
    if (eventId < 0) {
      std::stringstream ss;
      ss << "Can't find event: " << signature << std::endl
      << "  Candidate(s):" << std::endl;
      std::vector<MetaEvent>           mml = metaObject().findEvent(qi::signatureSplit(signature)[1]);
      std::vector<MetaEvent>::const_iterator it;

      for (it = mml.begin(); it != mml.end(); ++it) {
        ss << "  " << it->signature() << std::endl;
      }
      qiLogError("object") << ss.str();
      return false;
    }
    metaEmit(eventId, in);
    return true;
  }

  /// Resolve signature and bounce
  unsigned int Object::xConnect(const std::string &signature, const Functor* functor,
                                EventLoop* ctx)
  {
    int eventId = metaObject().eventId(signature);
    if (eventId < 0) {
      std::stringstream ss;
      ss << "Can't find event: " << signature << std::endl
      << "  Candidate(s):" << std::endl;
      std::vector<MetaEvent>           mml = metaObject().findEvent(qi::signatureSplit(signature)[1]);
      std::vector<MetaEvent>::const_iterator it;

      for (it = mml.begin(); it != mml.end(); ++it) {
        ss << "  " << it->signature() << std::endl;
      }
      qiLogError("object") << ss.str();
      return -1;
    }
    return connect(eventId, functor, ctx);
  }

  unsigned int Object::connect(unsigned int event, const Functor* functor, EventLoop* ctx)
  {
    return connect(event, EventSubscriber(functor, ctx));
  }

  unsigned int Object::connect(unsigned int event, const EventSubscriber& sub)
  {
    if (_p->_dying)
    {
      qiLogError("object") << "Cannot connect to dying object";
      return 0;
    }
    boost::recursive_mutex::scoped_lock sl(metaObject()._p->_mutexEvent);
    MetaEvent* ev = metaObject().event(event);
    if (!ev)
    {
      qiLogError("object") << "No such event " << event;
      return 0;
    }
    // Should we validate event here too?
    unsigned int uid = ++MetaObjectPrivate::uid;
    // Use [] directly, will create the entry (copy) if not present.
    _p->_subscribers[event][uid] = sub;
    // Get the subscrber copy in our map:
    EventSubscriber& s = _p->_subscribers[event][uid];
    s.linkId = uid;
    s.eventSource = this;
    s.event = event;
    // Notify the target of this subscribers
    if (s.target)
    {
      boost::recursive_mutex::scoped_lock sl(s.target->_p->_mutexRegistration);
      s.target->_p->_registrations.push_back(s);
    }
    return uid;
  }
  static void delete_functor(const Functor* ptr)
  {
    delete ptr;
  }
  bool Object::disconnect(unsigned int linkId)
  {
    boost::recursive_mutex::scoped_lock sl(metaObject()._p->_mutexEvent);
    // Look it up.
    // FIXME: Maybe store the event id inside the link id for faster lookup?
    std::map<unsigned int, ObjectPrivate::SubscriberMap>::iterator it;
    for (it = _p->_subscribers.begin(); it != _p->_subscribers.end(); ++it)
    {
      ObjectPrivate::SubscriberMap::iterator j = it->second.find(linkId);
      if (j != it->second.end())
      {
        EventSubscriber& sub = j->second;
        // We have ownership of the handler, despite all the copies.
        if (sub.handler && sub.eventLoop && !sub.eventLoop->isInEventLoopThread())
          sub.eventLoop->asyncCall(0,
            boost::bind(delete_functor, sub.handler));
        else
          delete sub.handler;
        // If target is an object, deregister from it
        if (sub.target)
        {
          boost::recursive_mutex::scoped_lock sl(sub.target->_p->_mutexRegistration);
          std::vector<EventSubscriber>& regs = sub.target->_p->_registrations;
          // Look it up in vector, then swap with last.
          for (unsigned int i=0; i< regs.size(); ++i)
            if (sub.linkId == regs[i].linkId)
            {
              regs[i] = regs[regs.size()-1];
              regs.pop_back();
              break;
            }
        }
        it->second.erase(j);
        return true;
      }
    }
    return false;
  }

  unsigned int Object::connect(unsigned int signal,
      qi::Object* target, unsigned int slot)
  {
    MetaEvent* ev = metaObject().event(signal);
    if (!ev)
    {
      qiLogError("object") << "No such event " << signal;
      return 0;
    }
    return connect(signal, EventSubscriber(target, slot));
  }

  EventLoop* Object::eventLoop()
  {
    return _p->_eventLoop;
  }

  void Object::moveToEventLoop(EventLoop* ctx)
  {
    _p->_eventLoop = ctx;
  }

  std::vector<EventSubscriber> Object::subscribers(int eventId) const
  {
    std::vector<EventSubscriber> res;
    std::map<unsigned int, ObjectPrivate::SubscriberMap>::iterator it = _p->_subscribers.find(eventId);
    ObjectPrivate::SubscriberMap::iterator jt;
    if (it == _p->_subscribers.end())
      return res;
    for (jt = it->second.begin(); jt != it->second.end(); ++jt)
      res.push_back(jt->second);
    return res;
  }


  qi::ODataStream &operator<<(qi::ODataStream &stream, const MetaObject &meta) {
    stream << meta._p->_methods;
    stream << meta._p->_events;
    stream << meta._p->_nextNumber;
    return stream;
  }

  qi::IDataStream &operator>>(qi::IDataStream &stream, MetaObject &meta) {
    stream >> meta._p->_methods;
    stream >> meta._p->_events;
    stream >> meta._p->_nextNumber;
    meta._p->refreshCache();
    return stream;
  }

  qi::SignatureStream &operator&(qi::SignatureStream &stream, const MetaObject &meta) {
    stream & meta._p->_methods;
    stream & meta._p->_events;
    stream & meta._p->_nextNumber;
    return stream;
  }

  std::vector<qi::MetaMethod> MetaObject::findMethod(const std::string &name)
  {
    return _p->findMethod(name);
  }

  std::vector<MetaEvent> MetaObject::findEvent(const std::string &name)
  {
    return _p->findEvent(name);
  }

  void MetaObjectPrivate::refreshCache()
  {
    {
      boost::recursive_mutex::scoped_lock sl(_mutexMethod);
      _methodsNameToIdx.clear();
      for (MetaObject::MethodMap::iterator i = _methods.begin();
        i != _methods.end(); ++i)
      _methodsNameToIdx[i->second.signature()] = i->second.uid();
    }
    {
      boost::recursive_mutex::scoped_lock sl(_mutexEvent);
      _eventsNameToIdx.clear();
      for (MetaObject::EventMap::iterator i = _events.begin();
        i != _events.end(); ++i)
      _eventsNameToIdx[i->second.signature()] = i->second.uid();
    }
  }
};
