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
    if (id < _p->_methods.size())
      return &_p->_methods[id];
    return 0;
  }

  MetaMethod *MetaObject::method(unsigned int id) const {
    if (id < _p->_methods.size())
      return &_p->_methods[id];
    return 0;
  }

  MetaEvent *MetaObject::event(unsigned int id) {
    if (id < _p->_events.size())
      return &_p->_events[id];
    return 0;
  }

  MetaEvent *MetaObject::event(unsigned int id) const {
    if (id < _p->_events.size())
      return &_p->_events[id];
    return 0;
  }

  int MetaObject::methodId(const std::string &name)
  {
    return _p->methodId(name);
  }

  int MetaObject::eventId(const std::string &name)
  {
    return _p->eventId(name);
  }

  std::vector<MetaMethod> &MetaObject::methods() {
    return _p->methods();
  }

  const std::vector<MetaMethod> &MetaObject::methods() const {
    return _p->methods();
  }

  std::vector<MetaEvent> &MetaObject::events() {
    return _p->events();
  }

  const std::vector<MetaEvent> &MetaObject::events() const {
    return _p->events();
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
      unsigned int idx = it->second;
      MetaMethod mm(sigret, signature, functor);
      mm._p->_idx = idx;
      _meta->methods()[idx] = mm;
      qiLogVerbose("qi.Object") << "rebinding method:" << signature;
      return idx;
    }

    MetaMethod mm(sigret, signature, functor);
    unsigned int idx = _meta->_p->_methodsNumber++;
    mm._p->_idx = idx;
    _meta->methods().push_back(mm);
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
    unsigned int idx = _meta->_p->_eventsNumber++;
    MetaEvent me(signature);
    me._p->_idx = idx;
    _meta->events().push_back(me);
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
      qi::Buffer     buf;
      qi::DataStream ds(buf);
      ds << ss.str();
      out.setError(buf);
      return;
    }
    if (mm->_p->_functor)
      mm->_p->_functor->call(in, out);
  }

  class DropResult: public FunctorResultBase
  {
  public:
    virtual void setValue(const qi::Buffer &buffer) {}
    virtual void setError(const qi::Buffer &msg)
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
      qiLogError("object") << "No such event " << event;
      return;
    }

    for (MetaEventPrivate::Subscribers::iterator il =
      ev->_p->_subscribers.begin(); il != ev->_p->_subscribers.end(); ++il)
    {
      il->second.handler->call(args, FunctorResult(
        boost::shared_ptr<FunctorResultBase>(new DropResult())));
    }
  }

  bool Object::xMetaCall(const std::string &retsig, const std::string &signature, const FunctorParameters &in, FunctorResult out)
  {
    int methodId = metaObject().methodId(signature);
    if (methodId < 0) {
      qi::Buffer     buf;
      qi::DataStream ds(buf);
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
      ds << ss.str();
      out.setError(buf);
      return false;
    }
    if (retsig != "v") {
      qi::MetaMethod *mm = metaObject().method(methodId);
      if (!mm) {
        qiLogError("object") << "method " << signature << "(id: " << methodId << ") disapeared mysteriously!";
        return false;
      }
      if (mm->sigreturn() != retsig) {
        qiLogError("object") << "signature mismatch for return value:" << std::endl
                             << "we want: " << retsig << " " << signature << std::endl
                             << "we had:" << mm->sigreturn() << " " << mm->signature();
        return false;
      }
    }
    //TODO: check for metacall to return false when not able to send the answer
    metaCall(methodId, in, out);
    return true;
  }

  /// Resolve signature and bounce
  bool Object::xMetaEmit(const std::string &signature, const FunctorParameters &in) {
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
      return false;
    }
    metaEmit(eventId, in);
    return true;
  }

  qi::DataStream &operator<<(qi::DataStream &stream, const MetaMethod &meta) {
    stream << meta._p->_signature;
    stream << meta._p->_sigret;
    stream << meta._p->_idx;
    return stream;
  }

  qi::DataStream &operator>>(qi::DataStream &stream, MetaMethod &meta) {
    stream >> meta._p->_signature;
    stream >> meta._p->_sigret;
    stream >> meta._p->_idx;
    return stream;
  }

  qi::DataStream &operator<<(qi::DataStream &stream, const MetaEvent &meta) {
    stream << meta._p->_signature;
    stream << meta._p->_idx;
    return stream;
  }

  qi::DataStream &operator>>(qi::DataStream &stream, MetaEvent &meta) {
    stream >> meta._p->_signature;
    stream >> meta._p->_idx;
    return stream;
  }

  qi::DataStream &operator<<(qi::DataStream &stream, const MetaObject &meta) {
    stream << meta._p->_methods;
    stream << meta._p->_methodsNumber;
    stream << meta._p->_events;
    stream << meta._p->_eventsNumber;
    return stream;
  }

  qi::DataStream &operator>>(qi::DataStream &stream, MetaObject &meta) {
    stream >> meta._p->_methods;
    stream >> meta._p->_methodsNumber;
    stream >> meta._p->_events;
    stream >> meta._p->_eventsNumber;
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
    for (std::vector<MetaMethod>::iterator i = _methods.begin();
      i != _methods.end(); ++i)
      _methodsNameToIdx[i->signature()] = i->index();
    for (std::vector<MetaEvent>::iterator i = _events.begin();
      i != _events.end(); ++i)
      _eventsNameToIdx[i->signature()] = i->index();
  }
};
