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

namespace qi {

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
    MethodMap::iterator i = _p->_methods.find(id);
    if (i == _p->_methods.end())
      return 0;
    return &i->second;
  }

  const MetaMethod *MetaObject::method(unsigned int id) const {
    MethodMap::const_iterator i = _p->_methods.find(id);
    if (i == _p->_methods.end())
      return 0;
    return &i->second;
  }

  MetaEvent *MetaObject::event(unsigned int id) {
    EventMap::iterator i = _p->_events.find(id);
    if (i == _p->_events.end())
      return 0;
    return &i->second;
  }

  const MetaEvent *MetaObject::event(unsigned int id) const {
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

  MetaObject::MethodMap &MetaObject::methods() {
    return _p->_methods;
  }

  const MetaObject::MethodMap &MetaObject::methods() const {
    return _p->_methods;
  }

  MetaObject::EventMap &MetaObject::events() {
    return _p->_events;
  }

  const  MetaObject::EventMap &MetaObject::events() const {
    return _p->_events;
  }

  MetaObject::~MetaObject()
  {
    delete _p;
  }

  Object::Object()
    : _meta(new MetaObject())
  {
    advertiseMethod("__metaobject", this, &Object::metaObject);
  }

  Object::~Object() {
    // Notify events that have subscribers that call us that we are dead
    // We need to make a copy of the vector, since disconnect will
    // remove entries from it.
    std::vector<MetaEvent::Subscriber> regs = _meta->_p->_registrations;
    std::vector<MetaEvent::Subscriber>::iterator i;
    for (i = regs.begin(); i != regs.end(); ++i)
      i->eventSource->disconnect(i->linkId);

    // Then remove _registrations with one of our event as source
    MetaObject::EventMap::iterator ii;
    for (ii = _meta->events().begin(); ii!= _meta->events().end(); ++ii)
    {
      MetaEventPrivate::Subscribers::iterator j;
      for (j = ii->second._p->_subscribers.begin();
        j != ii->second._p->_subscribers.end();
        ++j)
        disconnect(j->second.linkId);
    }
    delete _meta;
  }

  MetaObject &Object::metaObject() {
    return *_meta;
  }

  int Object::xForgetMethod(const std::string &meth)
  {
    std::map<std::string, unsigned int>::iterator it;

    it = _meta->_p->_methodsNameToIdx.find(meth);
    if (it != _meta->_p->_methodsNameToIdx.end())
    {
      _meta->_p->_methodsNameToIdx.erase(it);
      return (0);
    }

    return (1);
  }

  int Object::xAdvertiseMethod(const std::string &sigret, const std::string& signature, const qi::Functor* functor) {
    std::map<std::string, unsigned int>::iterator it;

    it = _meta->_p->_methodsNameToIdx.find(signature);
    if (it != _meta->_p->_methodsNameToIdx.end())
    {
      unsigned int uid = it->second;
      MetaMethod mm(sigret, signature, functor);
      mm._p->_uid = uid;
      // find it
      _meta->_p->_methods[uid] = mm;
      qiLogVerbose("qi.Object") << "rebinding method:" << signature;
      return uid;
    }

    MetaMethod mm(sigret, signature, functor);
    unsigned int idx = _meta->_p->_nextNumber++;
    mm._p->_uid = idx;
    _meta->_p->_methods[idx] = mm;
    _meta->_p->_methodsNameToIdx[signature] = idx;
    qiLogVerbose("qi.Object") << "binding method:" << signature;
    return idx;
  }

  int Object::xAdvertiseEvent(const std::string& signature) {
    if (signature.empty())
    {
      qiLogError("qi.Object") << "Event has empty signature.";
      return -1;
    }
    std::map<std::string, unsigned int>::iterator it;

    it = _meta->_p->_eventsNameToIdx.find(signature);
    if (it != _meta->_p->_eventsNameToIdx.end())
    { // Event already there.
      qiLogError("qi.Object") << "event already there";
      return it->second;
    }
    unsigned int idx = _meta->_p->_nextNumber++;
    MetaEvent me(signature);
    me._p->_uid = idx;
    _meta->_p->_events[idx] = me;
    _meta->_p->_eventsNameToIdx[signature] = idx;
    qiLogVerbose("qi.Object") << "binding event:" << signature <<" with id "
    << idx;
    return idx;
  }

  void Object::metaCall(unsigned int method, const FunctorParameters &in, qi::FunctorResult out)
  {
    MetaMethod *mm = _meta->method(method);
    if (!mm) {
      std::stringstream ss;
      ss << "Can't find methodID: " << method;
      out.setError(ss.str());
      return;
    }
    if (mm->_p->_functor)
      mm->_p->_functor->call(in, out);
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
    trigger(event, args);
  }
  void Object::trigger(unsigned int event, const FunctorParameters &args)
  {
    MetaEvent* ev = _meta->event(event);
    if (!ev)
    {
      MetaMethod* me = _meta->method(event);
      if (!me)
      {
        qiLogError("object") << "No such event " << event;
        return;
      }
      else
      {
        metaCall(event, args, FunctorResult(
          boost::shared_ptr<FunctorResultBase>(new DropResult)));
        return;
      }
    }

    for (MetaEventPrivate::Subscribers::iterator il =
      ev->_p->_subscribers.begin(); il != ev->_p->_subscribers.end(); ++il)
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
  unsigned int Object::xConnect(const std::string &signature, const Functor* functor)
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
    return connect(eventId, functor);
  }

  unsigned int Object::connect(unsigned int event, const Functor* functor)
  {
    return connect(event, MetaEvent::Subscriber(functor));
  }

  unsigned int Object::connect(unsigned int event,
    const MetaEvent::Subscriber& sub)
  {
    MetaEvent* ev = _meta->event(event);
    if (!ev)
    {
      qiLogError("object") << "No such event " << event;
      return 0;
    }
    // Should we validate event here too?
    // Use [] directly, will create the entry if not present.
    unsigned int uid = ++MetaObjectPrivate::uid;
    ev->_p->_subscribers[uid] = sub;
    // Get the subscrber copy in our map:
    MetaEvent::Subscriber& s = ev->_p->_subscribers[uid];
    s.linkId = uid;
    s.eventSource = this;
    s.event = event;
    // Notify the target of this subscribers
    if (s.target)
      s.target->_meta->_p->_registrations.push_back(s);
    return uid;
  }

  bool Object::disconnect(unsigned int id)
  {
    // Look it up.
    // FIXME: Maybe store the event id inside the link id for faster lookup?
    MetaObject::EventMap::iterator i;
    for (i = _meta->events().begin(); i!= _meta->events().end(); ++i)
    {
      MetaEventPrivate::Subscribers::iterator j = i->second._p->_subscribers.find(id);
      if (j != i->second._p->_subscribers.end())
      {
        MetaEvent::Subscriber& sub = j->second;
        // We have ownership of the handler, despite all the copies.
        delete sub.handler;
        // If target is an object, deregister from it
        if (sub.target)
        {
          std::vector<MetaEvent::Subscriber>& regs = sub.target->_meta->_p->_registrations;
          // Look it up in vector, then swap with last.
          for (unsigned int i=0; i< regs.size(); ++i)
            if (sub.linkId == regs[i].linkId)
            {
              regs[i] = regs[regs.size()-1];
              regs.pop_back();
              break;
            }
        }
        i->second._p->_subscribers.erase(j);
        return true;
      }
    }
    return false;
  }

  unsigned int Object::connect(unsigned int signal,
      qi::Object* target, unsigned int slot)
  {
    MetaEvent* ev = _meta->event(signal);
    if (!ev)
    {
      qiLogError("object") << "No such event " << signal;
      return 0;
    }
    return connect(signal, MetaEvent::Subscriber(target, slot));
  }

  qi::ODataStream &operator<<(qi::ODataStream &stream, const MetaMethod &meta) {
    stream << meta._p->_signature;
    stream << meta._p->_sigret;
    stream << meta._p->_uid;
    return stream;
  }

  qi::IDataStream &operator>>(qi::IDataStream &stream, MetaMethod &meta) {
    stream >> meta._p->_signature;
    stream >> meta._p->_sigret;
    stream >> meta._p->_uid;
    return stream;
  }

  qi::ODataStream &operator<<(qi::ODataStream &stream, const MetaEvent &meta) {
    stream << meta._p->_signature;
    stream << meta._p->_uid;
    return stream;
  }

  qi::IDataStream &operator>>(qi::IDataStream &stream, MetaEvent &meta) {
    stream >> meta._p->_signature;
    stream >> meta._p->_uid;
    return stream;
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
    _methodsNameToIdx.clear();
    for (MetaObject::MethodMap::iterator i = _methods.begin();
      i != _methods.end(); ++i)
      _methodsNameToIdx[i->second.signature()] = i->second.uid();
    for (MetaObject::EventMap::iterator i = _events.begin();
      i != _events.end(); ++i)
      _eventsNameToIdx[i->second.signature()] = i->second.uid();
  }
};
