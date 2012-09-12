/*
**
** Copyright (C) 2012 Aldebaran Robotics
*/

#include <boost/thread/mutex.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include "src/object_p.hpp"
#include "src/objectbuilder_p.hpp"
#include "src/metaobject_p.hpp"

namespace qi {

  ObjectPrivate::ObjectPrivate()
    : _dying(false)
    , _eventLoop(getDefaultObjectEventLoop())
  {
  }

  ObjectPrivate::ObjectPrivate(const qi::MetaObject &meta)
    : _dying(false)
    , _eventLoop(getDefaultObjectEventLoop())
    , _meta(meta)
  {
  }


  ObjectPrivate::~ObjectPrivate() {
    {
      _dying = true;
      {
        boost::recursive_mutex::scoped_lock sl(_mutexRegistration);

        // Notify events that have subscribers that call us that we are dead
        // We need to make a copy of the vector, since disconnect will
        // remove entries from it.
        std::vector<SignalSubscriber> regs = _registrations;
        std::vector<SignalSubscriber>::iterator i;
        for (i = regs.begin(); i != regs.end(); ++i)
          i->source->disconnect(i->linkId);
      }

      boost::recursive_mutex::scoped_lock sl(metaObject()._p->_mutexEvent);
      // Then remove _registrations with one of our event as source
      //MetaObject::SignalMap::iterator ii;
      std::map<unsigned int, SignalBase*>::iterator ii;
      for (ii = _subscribers.begin(); ii!= _subscribers.end(); ++ii)
      {
        delete ii->second;
      }
    }
  }

  void ObjectPrivate::addCallbacks(ObjectInterface *callbacks, void *data)
  {
    {
      boost::mutex::scoped_lock l(_callbacksMutex);
      _callbacks[callbacks] = data;
    }
  }

  void ObjectPrivate::removeCallbacks(ObjectInterface *callbacks)
  {
    std::map<ObjectInterface *, void *>::iterator it;
    {
      boost::mutex::scoped_lock l(_callbacksMutex);
      it = _callbacks.find(callbacks);
      if (it != _callbacks.end())
        _callbacks.erase(it);
    }
  }

  static void functor_call(
    Value instance,
    MetaCallable f,
    MetaFunctionParameters arg,
    qi::Promise<MetaFunctionResult> out)
  {
    out.setValue(f(instance, arg));
  }

  qi::Future<MetaFunctionResult>
  ObjectPrivate::metaCall(unsigned int method, const MetaFunctionParameters& params, qi::Object::MetaCallType callType)
  {
    qi::Promise<MetaFunctionResult> out;
    MetaMethod *mm = metaObject().method(method);
    if (!mm) {
      std::stringstream ss;
      ss << "Can't find methodID: " << method;
      out.setError(ss.str());
      return out.future();
    }

    bool synchronous = true;
    switch (callType)
    {
    case qi::Object::MetaCallType_Direct:
      break;
    case qi::Object::MetaCallType_Auto:
      synchronous = !_eventLoop ||  _eventLoop->isInEventLoopThread();
      break;
    case qi::Object::MetaCallType_Queued:
      synchronous = !_eventLoop;
      break;
    }
    if (synchronous)
      out.setValue(mm->functor()(_instance, params));
    else
    {
      // We need to copy arguments.
      MetaFunctionParameters pCopy = params.copy();
      eventLoop()->asyncCall(0,
        boost::bind(&functor_call,
          _instance,
          mm->functor(),
          pCopy, out));
    }
    return out.future();
  }

  void ObjectPrivate::metaEmit(unsigned int event, const MetaFunctionParameters& args)
  {
    trigger(event, args);
  }

  void ObjectPrivate::trigger(unsigned int event, const MetaFunctionParameters &args)
  {
    // Note: Not thread-safe, event/method may die while we hold it.
    MetaSignal* ev = metaObject().signal(event);
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
        metaCall(event, args);
        return;
      }
    }
    boost::recursive_mutex::scoped_lock sl(metaObject()._p->_mutexEvent);
    std::map<unsigned int, SignalBase*>::iterator it;
    it = _subscribers.find(event);
    if (it == _subscribers.end())
      return;
    it->second->trigger(args);
  }

  qi::Future<MetaFunctionResult>
  ObjectPrivate::xMetaCall(const std::string &retsig, const std::string &signature, const MetaFunctionParameters& args)
  {
    qi::Promise<MetaFunctionResult> out;
    const MetaFunctionParameters* newArgs = 0;
    int methodId = metaObject().methodId(signature);
#ifndef QI_REQUIRE_SIGNATURE_EXACT_MATCH
    if (methodId < 0) {

      // Try to find an other method with compatible signature
      std::vector<qi::MetaMethod> mml = metaObject().findMethod(qi::signatureSplit(signature)[1]);
      Signature sargs(signatureSplit(signature)[2]);
      for (unsigned i=0; i<mml.size(); ++i)
      {
        Signature s(signatureSplit(mml[i].signature())[2]);
        if (sargs.isConvertibleTo(s))
        {
          qiLogVerbose("qi.object")
              << "Signature mismatch, but found compatible type "
              << mml[i].signature() <<" for " << signature;
          methodId = mml[i].uid();
          // Signature is wrapped in a tuple, unwrap
          newArgs = new MetaFunctionParameters(args.convert(s.begin().children()));
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
      qi::MetaMethod *mm = metaObject().method(methodId);
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
        //out.setError(ss.str());
        //return out.future();
      }
    }
    //TODO: check for metacall to return false when not able to send the answer
    if (newArgs)
    {
      qi::Future<MetaFunctionResult> res = metaCall(methodId, *newArgs);
      delete newArgs;
      return res;
    }
    else
      return metaCall(methodId, args);
  }
  /// Resolve signature and bounce
  bool ObjectPrivate::xMetaEmit(const std::string &signature, const MetaFunctionParameters &in) {
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

  unsigned int ObjectPrivate::connect(unsigned int event, MetaFunction functor, EventLoop* ctx)
  {
    return connect(event, SignalSubscriber(functor, ctx));
  }

  /// Resolve signature and bounce
  unsigned int ObjectPrivate::xConnect(const std::string &signature, MetaFunction functor,
                                       EventLoop* ctx)
  {
    int eventId = metaObject().signalId(signature);
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
      return -1;
    }
    return connect(eventId, functor, ctx);
  }

  unsigned int ObjectPrivate::connect(unsigned int event, const SignalSubscriber& sub)
  {
    if (_dying)
    {
      qiLogError("object") << "Cannot connect to dying object";
      return 0;
    }
    boost::recursive_mutex::scoped_lock sl(metaObject()._p->_mutexEvent);
    MetaSignal* ev = metaObject().signal(event);
    if (!ev)
    {
      qiLogError("object") << "No such event " << event;
      return 0;
    }

    // Fetch the signal associated with this event, for this instance
    boost::recursive_mutex::scoped_lock sl2(_mutexRegistration);
    ObjectPrivate::SignalSubscriberMap::iterator i = _subscribers.find(event);
    if (i == _subscribers.end())
    {
      // Lazy-creation of the signalbase
      _subscribers[event] = new SignalBase(ev->signature());
      i = _subscribers.find(event);
    }
    SignalBase::Link l = i->second->connect(sub);
    if (l > 0xFFFF)
      qiLogError("object") << "Signal id too big";
    return (event << 16) + l;
  }

  bool ObjectPrivate::disconnect(unsigned int linkId)
  {
    unsigned int event = linkId >> 16;
    unsigned int link = linkId & 0xFFFF;
    boost::recursive_mutex::scoped_lock sl(metaObject()._p->_mutexEvent);
    ObjectPrivate::SignalSubscriberMap::iterator i = _subscribers.find(event);
    if (i == _subscribers.end())
    {
      qiLogWarning("qi.object") << "Disconnect on non instanciated signal";
      return false;
    }
    return i->second->disconnect(link);
  }

  unsigned int ObjectPrivate::connect(unsigned int signal, qi::Object target, unsigned int slot)
  {
    MetaSignal* ev = metaObject().signal(signal);
    if (!ev)
    {
      qiLogError("object") << "No such event " << signal;
      return 0;
    }
    return connect(signal, SignalSubscriber(target, slot));
  }

  EventLoop* ObjectPrivate::eventLoop()
  {
    return _eventLoop;
  }

  void ObjectPrivate::moveToEventLoop(EventLoop* ctx)
  {
    _eventLoop = ctx;
  }

  std::vector<SignalSubscriber> ObjectPrivate::subscribers(int eventId) const
  {
    std::vector<SignalSubscriber> res;
    SignalSubscriberMap::const_iterator it = _subscribers.find(eventId);
    if (it == _subscribers.end())
      return res;
    else
      return it->second->subscribers();
  }

  void ObjectPrivate::emitEvent(const std::string& eventName,
                                qi::AutoValue p1,
                                qi::AutoValue p2,
                                qi::AutoValue p3,
                                qi::AutoValue p4,
                                qi::AutoValue p5,
                                qi::AutoValue p6,
                                qi::AutoValue p7,
                                qi::AutoValue p8)
  {
    qi::AutoValue* vals[8]= {&p1, &p2, &p3, &p4, &p5, &p6, &p7, &p8};
    std::vector<qi::Value> params;
    for (unsigned i=0; i<8; ++i)
      if (vals[i]->value)
        params.push_back(*vals[i]);
    // Signature construction
    std::string signature = eventName + "::(";
    for (unsigned i=0; i< params.size(); ++i)
      signature += params[i].signature();
    signature += ")";
    xMetaEmit(signature, MetaFunctionParameters(params, true));
  }



}
