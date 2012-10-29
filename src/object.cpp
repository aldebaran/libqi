/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/
#include <iostream>
#include <qitype/genericobject.hpp>
#include "object_p.hpp"

namespace qi {

  GenericObject::GenericObject(ObjectType *type, void *value)
  : type(type)
  , value(value)
  {
  }

  GenericObject::~GenericObject() {
  }

  Manageable::Manageable()
  {
    _p = new ManageablePrivate();
    _p->eventLoop = getDefaultObjectEventLoop();
    _p->dying = false;
  }

  Manageable::Manageable(const Manageable& b)
  {
    _p = new ManageablePrivate();
    _p->eventLoop = b._p->eventLoop;
    _p->dying = false;
  }

  void Manageable::operator = (const Manageable& b)
  {
    this->~Manageable();
    _p = new ManageablePrivate();
    _p->eventLoop = b._p->eventLoop;
    _p->dying = false;
  }

  Manageable::~Manageable()
  {
    _p->dying = true;
    std::vector<SignalSubscriber> copy;
    {
      boost::mutex::scoped_lock sl(_p->registrationsMutex);
      copy = _p->registrations;
    }
    for (unsigned i = 0; i < copy.size(); ++i)
    {
      copy[i].source->disconnect(copy[i].linkId);
    }
    delete _p;
  }

  void Manageable::moveToEventLoop(EventLoop* el)
  {
    _p->eventLoop = el;
  }

  EventLoop* Manageable::eventLoop() const
  {
    return _p->eventLoop;
  }

  const MetaObject &GenericObject::metaObject() {
    if (!type || !value) {
      static qi::MetaObject fail;
      qiLogWarning("qi.object") << "Operating on invalid GenericObject..";
      return fail;
    }
    return type->metaObject(value);
  }

  qi::Future<GenericValue>
  GenericObject::metaCall(unsigned int method, const GenericFunctionParameters& params, MetaCallType callType)
  {
    qi::Promise<GenericValue> out;
    if (!type || !value) {
      qiLogWarning("qi.object") << "Operating on invalid GenericObject..";
      out.setError("Invalid object");
      return out.future();
    }
    try {
      return type->metaCall(value, method, params, callType);
    } catch (std::runtime_error &e) {
      out.setError(e.what());
      return out.future();
    }
  }

  void GenericObject::metaEmit(unsigned int event, const GenericFunctionParameters& args)
  {
    if (!type || !value) {
      qiLogWarning("qi.object") << "Operating on invalid GenericObject..";
      return;
    }
    type->metaEmit(value, event, args);
  }

  qi::Future<GenericValue>
  GenericObject::xMetaCall(const std::string &retsig, const std::string &signature, const GenericFunctionParameters& args)
  {
    qi::Promise<GenericValue> out;
    if (!type || !value) {
      qiLogWarning("qi.object") << "Operating on invalid GenericObject..";
      out.setError("Invalid object");
      return out.future();
    }
    const GenericFunctionParameters* newArgs = 0;
    int methodId = metaObject().methodId(signature);
#ifndef QI_REQUIRE_SIGNATURE_EXACT_MATCH
    if (methodId < 0) {

      // Try to find an other method with compatible signature
      std::vector<qi::MetaMethod> mml = metaObject().findMethod(qi::signatureSplit(signature)[1]);
      Signature sargs(signatureSplit(signature)[2]);
      for (unsigned i = 0; i < mml.size(); ++i)
      {
        Signature s(signatureSplit(mml[i].signature())[2]);
        if (sargs.isConvertibleTo(s))
        {
          qiLogVerbose("qi.object")
              << "Signature mismatch, but found compatible type "
              << mml[i].signature() <<" for " << signature;
          methodId = mml[i].uid();
          // Signature is wrapped in a tuple, unwrap
          newArgs = new GenericFunctionParameters(args.convert(s.begin().children()));
          break;
        }
      }
    }
#endif
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
      return out.future();
    }
    if (retsig != "v") {
      const qi::MetaMethod *mm = metaObject().method(methodId);
      if (!mm) {
        std::stringstream ss;
        ss << "method " << signature << "(id: " << methodId << ") disapeared mysteriously!";
        qiLogError("object") << ss.str();
        out.setError(ss.str());
        return out.future();
      }
      if (mm->sigreturn() != retsig) {
        std::stringstream ss;
        ss << "signature mismatch for return value:" << std::endl
           << "we want: " << retsig << " " << signature << std::endl
           << "we had:" << mm->sigreturn() << " " << mm->signature();
        qiLogWarning("object") << ss;
        // Let it pass, conversion system will do it's job
      }
    }
    //TODO: check for metacall to return false when not able to send the answer
    if (newArgs)
    {
      qi::Future<GenericValue> res = metaCall(methodId, *newArgs);
      delete newArgs;
      return res;
    }
    else
      return metaCall(methodId, args);
  }
  /// Resolve signature and bounce
  bool GenericObject::xMetaEmit(const std::string &signature, const GenericFunctionParameters &in) {
    if (!value || !type) {
      qiLogWarning("qi.object") << "Operating on invalid GenericObject..";
      return false;
    }
       int eventId = metaObject().signalId(signature);
    if (eventId < 0)
      eventId = metaObject().methodId(signature);
    if (eventId < 0) {
      std::stringstream ss;
      ss << "Can't find event: " << signature << std::endl
         << "  Candidate(s):" << std::endl;
      std::vector<MetaSignal>           mml = metaObject().findSignal(qi::signatureSplit(signature)[1]);
      std::vector<MetaSignal>::const_iterator it;

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
  qi::FutureSync<unsigned int> GenericObject::xConnect(const std::string &signature, const SignalSubscriber& functor)
  {
    if (!type || !value) {
      qiLogWarning("qi.object") << "Operating on invalid GenericObject..";
      return qi::makeFutureError<unsigned int>("Operating on invalid GenericObject..");
    }
    int eventId = metaObject().signalId(signature);

  #ifndef QI_REQUIRE_SIGNATURE_EXACT_MATCH
    if (eventId < 0) {
      // Try to find an other event with compatible signature
      std::vector<qi::MetaSignal> mml = metaObject().findSignal(qi::signatureSplit(signature)[1]);
      Signature sargs(signatureSplit(signature)[2]);
      for (unsigned i = 0; i < mml.size(); ++i)
      {
        Signature s(signatureSplit(mml[i].signature())[2]);
        qiLogDebug("qi.object") << "Checking compatibility " << s.toString() << ' '
         << sargs.toString();
         // Order is reversed from method call check.
        if (s.isConvertibleTo(sargs))
        {
          qiLogVerbose("qi.object")
              << "Signature mismatch, but found compatible type "
              << mml[i].signature() <<" for " << signature;
          eventId = mml[i].uid();
          break;
        }
      }
    }
#endif
    if (eventId < 0) {
      std::stringstream ss;
      ss << "Can't find event: " << signature << std::endl
         << "  Candidate(s):" << std::endl;
      std::vector<MetaSignal>           mml = metaObject().findSignal(qi::signatureSplit(signature)[1]);
      std::vector<MetaSignal>::const_iterator it;

      for (it = mml.begin(); it != mml.end(); ++it) {
        ss << "  " << it->signature() << std::endl;
      }
      qiLogError("object") << ss.str();
      return qi::makeFutureError<unsigned int>(ss.str());
    }
    return connect(eventId, functor);
  }

  qi::FutureSync<unsigned int> GenericObject::connect(unsigned int event, const SignalSubscriber& sub)
  {
    if (!type || !value) {
      qiLogWarning("qi.object") << "Operating on invalid GenericObject..";
      return qi::makeFutureError<unsigned int>("Operating on invalid GenericObject..");
    }
    return type->connect(value, event, sub);
  }

  qi::FutureSync<void> GenericObject::disconnect(unsigned int linkId)
  {
    if (!type || !value) {
      qiLogWarning("qi.object") << "Operating on invalid GenericObject..";
      return qi::makeFutureError<void>("Operating on invalid GenericObject");
    }
    return type->disconnect(value, linkId);
  }

  qi::FutureSync<unsigned int> GenericObject::connect(unsigned int signal, ObjectPtr target, unsigned int slot)
  {
    return connect(signal, SignalSubscriber(target, slot));
  }

  /*
  std::vector<SignalSubscriber> GenericObject::subscribers(int eventId) const
  {
    std::vector<SignalSubscriber> res;
    if (!_p) {
      qiLogWarning("qi.object") << "Operating on invalid GenericObject..";
      return res;
    }
    return _p->subscribers(eventId);
  }*/

  void GenericObject::emitEvent(const std::string& eventName,
                         qi::AutoGenericValue p1,
                         qi::AutoGenericValue p2,
                         qi::AutoGenericValue p3,
                         qi::AutoGenericValue p4,
                         qi::AutoGenericValue p5,
                         qi::AutoGenericValue p6,
                         qi::AutoGenericValue p7,
                         qi::AutoGenericValue p8)
  {
    if (!type || !value) {
      qiLogWarning("qi.object") << "Operating on invalid GenericObject..";
      return;
    }
    qi::AutoGenericValue* vals[8]= {&p1, &p2, &p3, &p4, &p5, &p6, &p7, &p8};
    std::vector<qi::GenericValue> params;
    for (unsigned i=0; i<8; ++i)
      if (vals[i]->value)
        params.push_back(*vals[i]);
    // Signature construction
    std::string signature = eventName + "::(";
    for (unsigned i=0; i< params.size(); ++i)
      signature += params[i].signature();
    signature += ")";
    xMetaEmit(signature, GenericFunctionParameters(params));
  }

  int ObjectType::inherits(Type* other)
  {
    /* A registered class C can have to Type* around:
    * - TypeImpl<C*>
    * - The staticObjectType that was created by the builder.
    * So assume that any of them can be in the parentTypes list.
    */
    if (this == other)
      return 0;
    const std::vector<std::pair<Type*, int> >& parents = parentTypes();
    qiLogDebug("qi.meta") << infoString() <<" has " << parents.size() <<" parents";
    for (unsigned i=0; i<parents.size(); ++i)
    {
      if (parents[i].first->info() == other->info())
        return parents[i].second;
      ObjectType* op = dynamic_cast<ObjectType*>(parents[i].first);
      if (op)
      {
        int offset = op->inherits(other);
        if (offset != -1)
        {
          qiLogDebug("qi.meta") << "Inheritance offsets " << parents[i].second
           << " " << offset;
          return parents[i].second + offset;
        }
      }
      qiLogDebug("qi.meta") << parents[i].first->infoString() << " does not match " << other->infoString()
      <<" " << ((bool)op == (bool)dynamic_cast<ObjectType*>(other));
    }
    return -1;
  }
}

