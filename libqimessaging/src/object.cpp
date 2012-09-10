/*
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2012 Aldebaran Robotics
*/

#include <iostream>
#include <qi/application.hpp>
#include "src/metamethod_p.hpp"
#include "src/metasignal_p.hpp"
#include "src/object_p.hpp"
#include "src/metaobject_p.hpp"
#include <qimessaging/signal.hpp>
#include <qimessaging/object.hpp>
#include <qimessaging/object_factory.hpp>
#include <qimessaging/event_loop.hpp>

namespace qi {

  ObjectInterface::~ObjectInterface() {
  }

  Object::Object(qi::MetaObject metaObject)
    : _p(new ObjectPrivate(metaObject))
  {
  }

  Object::Object()
    : _p()
  {
  }

  Object::~Object() {
    {
      if (!_p) {
        qiLogWarning("qi.object") << "Operating on invalid Object..";
        return;
      }
      _p->_dying = true;
      {
        boost::recursive_mutex::scoped_lock sl(_p->_mutexRegistration);

        // Notify events that have subscribers that call us that we are dead
        // We need to make a copy of the vector, since disconnect will
        // remove entries from it.
        std::vector<SignalSubscriber> regs = _p->_registrations;
        std::vector<SignalSubscriber>::iterator i;
        for (i = regs.begin(); i != regs.end(); ++i)
          i->source->disconnect(i->linkId);
      }

      boost::recursive_mutex::scoped_lock sl(metaObject()._p->_mutexEvent);
      // Then remove _registrations with one of our event as source
      //MetaObject::SignalMap::iterator ii;
      std::map<unsigned int, SignalBase*>::iterator ii;
      for (ii = _p->_subscribers.begin(); ii!= _p->_subscribers.end(); ++ii)
      {
        delete ii->second;
      }
    }
  }

  bool Object::isValid() const {
    return !!_p;
  }

  void Object::addCallbacks(ObjectInterface *callbacks, void *data)
  {
    if (!_p) {
      qiLogWarning("qi.object") << "Operating on invalid Object..";
      return;
    }
    {
      boost::mutex::scoped_lock l(_p->_callbacksMutex);
      _p->_callbacks[callbacks] = data;
    }
  }

  void Object::removeCallbacks(ObjectInterface *callbacks)
  {
    if (!_p) {
      qiLogWarning("qi.object") << "Operating on invalid Object..";
      return;
    }
    std::map<ObjectInterface *, void *>::iterator it;
    {
      boost::mutex::scoped_lock l(_p->_callbacksMutex);
      it = _p->_callbacks.find(callbacks);
      if (it != _p->_callbacks.end())
        _p->_callbacks.erase(it);
    }
  }

  MetaObject &Object::metaObject() {
    if (!_p) {
      static qi::MetaObject fail;
      qiLogWarning("qi.object") << "Operating on invalid Object..";
      return fail;
    }
    return _p->metaObject();
  }

  static void functor_call(MetaFunction f,
                           MetaFunctionParameters arg,
                           qi::Promise<MetaFunctionResult> out)
  {
    out.setValue(f(arg));
  }

  qi::Future<MetaFunctionResult>
  Object::metaCall(unsigned int method, const MetaFunctionParameters& params, MetaCallType callType)
  {
    qi::Promise<MetaFunctionResult> out;
    if (!_p) {
      qiLogWarning("qi.object") << "Operating on invalid Object..";
      out.setError("Invalid object");
      return out.future();
    }
    MetaMethod *mm = metaObject().method(method);
    if (!mm) {
      std::stringstream ss;
      ss << "Can't find methodID: " << method;
      out.setError(ss.str());
      return out.future();
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
        out.setValue(mm->_p->_functor(params));
      else
      {
        // We need to copy arguments.
        MetaFunctionParameters pCopy = params.copy();
        eventLoop()->asyncCall(0,
          boost::bind(&functor_call,
            mm->_p->_functor,
            pCopy, out));
      }
    }
    else {
      std::stringstream ss;
      ss << "No valid functor for methodid: " << method;
      out.setError(ss.str());
    }
    return out.future();
  }

  void Object::metaEmit(unsigned int event, const MetaFunctionParameters& args)
  {
    trigger(event, args);
  }

  void Object::trigger(unsigned int event, const MetaFunctionParameters &args)
  {
    if (!_p) {
      qiLogWarning("qi.object") << "Operating on invalid Object..";
      return;
    }
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
    it = _p->_subscribers.find(event);
    if (it == _p->_subscribers.end())
      return;
    it->second->trigger(args);
  }

  qi::Future<MetaFunctionResult>
  Object::xMetaCall(const std::string &retsig, const std::string &signature, const MetaFunctionParameters& args)
  {
    qi::Promise<MetaFunctionResult> out;
    if (!_p) {
      qiLogWarning("qi.object") << "Operating on invalid Object..";
      out.setError("Invalid object");
      return out.future();
    }
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
  bool Object::xMetaEmit(const std::string &signature, const MetaFunctionParameters &in) {
    if (!_p) {
      qiLogWarning("qi.object") << "Operating on invalid Object..";
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
  unsigned int Object::xConnect(const std::string &signature, MetaFunction functor,
                                EventLoop* ctx)
  {
    if (!_p) {
      qiLogWarning("qi.object") << "Operating on invalid Object..";
      return -1;
    }

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

  unsigned int Object::connect(unsigned int event, MetaFunction functor, EventLoop* ctx)
  {
    if (!_p) {
      qiLogWarning("qi.object") << "Operating on invalid Object..";
      return -1;
    }
    return connect(event, SignalSubscriber(functor, ctx));
  }

  unsigned int Object::connect(unsigned int event, const SignalSubscriber& sub)
  {
    if (!_p) {
      qiLogWarning("qi.object") << "Operating on invalid Object..";
      return -1;
    }
    if (_p->_dying)
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
    boost::recursive_mutex::scoped_lock sl2(_p->_mutexRegistration);
    ObjectPrivate::SignalSubscriberMap::iterator i = _p->_subscribers.find(event);
    if (i == _p->_subscribers.end())
    {
      // Lazy-creation of the signalbase
      _p->_subscribers[event] = new SignalBase(ev->signature());
      i = _p->_subscribers.find(event);
    }
    SignalBase::Link l = i->second->connect(sub);
    if (l > 0xFFFF)
      qiLogError("object") << "Signal id too big";
    return (event << 16) + l;
  }

  bool Object::disconnect(unsigned int linkId)
  {
    if (!_p) {
      qiLogWarning("qi.object") << "Operating on invalid Object..";
      return false;
    }
    unsigned int event = linkId >> 16;
    unsigned int link = linkId & 0xFFFF;
    boost::recursive_mutex::scoped_lock sl(metaObject()._p->_mutexEvent);
    ObjectPrivate::SignalSubscriberMap::iterator i = _p->_subscribers.find(event);
    if (i == _p->_subscribers.end())
    {
      qiLogWarning("qi.object") << "Disconnect on non instanciated signal";
      return false;
    }
    return i->second->disconnect(link);
  }

  unsigned int Object::connect(unsigned int signal, qi::Object target, unsigned int slot)
  {
    if (!_p) {
      qiLogWarning("qi.object") << "Operating on invalid Object..";
      return -1;
    }
    MetaSignal* ev = metaObject().signal(signal);
    if (!ev)
    {
      qiLogError("object") << "No such event " << signal;
      return 0;
    }
    return connect(signal, SignalSubscriber(target, slot));
  }

  EventLoop* Object::eventLoop()
  {
    if (!_p) {
      qiLogWarning("qi.object") << "Operating on invalid Object..";
      return 0;
    }
    return _p->_eventLoop;
  }

  void Object::moveToEventLoop(EventLoop* ctx)
  {
    if (!_p) {
      qiLogWarning("qi.object") << "Operating on invalid Object..";
      return;
    }
    _p->_eventLoop = ctx;
  }

  std::vector<SignalSubscriber> Object::subscribers(int eventId) const
  {
    std::vector<SignalSubscriber> res;
    if (!_p) {
      qiLogWarning("qi.object") << "Operating on invalid Object..";
      return res;
    }
    std::map<unsigned int, SignalBase*>::iterator it = _p->_subscribers.find(eventId);
    if (it == _p->_subscribers.end())
      return res;
    else
      return it->second->subscribers();
  }



  // Factory system
  // We need thread-safeness, and we can be used at static init.
  // But at static init, thread-safeness is not required.
  // So lazy-init of the mutex should do the trick.
  static boost::recursive_mutex *_f_mutex_struct = 0;
  static boost::recursive_mutex *_f_mutex_load = 0;
  static std::vector<std::string>* _f_keys = 0;
  typedef std::map<std::string, boost::function<qi::Object (const std::string&)> > FactoryMap;
  static FactoryMap* _f_map = 0;
  static void _f_init()
  {
    if (!_f_mutex_struct)
    {
      _f_mutex_struct = new boost::recursive_mutex;
      _f_mutex_load = new boost::recursive_mutex;
      _f_keys = new std::vector<std::string>;
      _f_map = new FactoryMap;
    }
  }

  bool registerObjectFactory(const std::string& name, boost::function<qi::Object (const std::string&)> factory)
  {
    qiLogDebug("qi.factory") << "registering " << name;
    _f_init();
    boost::recursive_mutex::scoped_lock sl(*_f_mutex_struct);
    FactoryMap::iterator i = _f_map->find(name);
    if (i != _f_map->end())
      qiLogWarning("qi.object") << "Overriding factory for " <<name;
    else
      _f_keys->push_back(name);
    (*_f_map)[name] = factory;
    return true;
  }

  Object createObject(const std::string& name)
  {
    _f_init();
    boost::recursive_mutex::scoped_lock sl(*_f_mutex_struct);
    FactoryMap::iterator i = _f_map->find(name);
    if (i == _f_map->end())
      return Object();
    return (i->second)(name);
  }

  std::vector<std::string> listObjectFactories()
  {
    _f_init();
    boost::recursive_mutex::scoped_lock sl(*_f_mutex_struct);
    return *_f_keys;
  }

  std::vector<std::string> loadObject(const std::string& name, int flags)
  {
    /* Do not hold mutex_struct while calling dlopen/loadModule,
    * just in case static initialization of the module happens
    * in an other thread: it will likely call registerObjectFactory
    * which will acquire the mutex_struct.
    * We are still asserting that said initialization synchronously
    * finishes before dlopen/loadModule returns.
    */
    _f_init();
    std::vector<std::string>& keys = *_f_keys;
    boost::recursive_mutex::scoped_lock sl(*_f_mutex_load);
    unsigned int count = keys.size();
    qiLogDebug("qi.factory") << count <<" object before load";
    Application::loadModule(name, flags);
    boost::recursive_mutex::scoped_lock sl2(*_f_mutex_struct);
    qiLogDebug("qi.factory") << keys.size() <<" object after load";
    if (count != keys.size())
      return std::vector<std::string>(&keys[count], &keys[keys.size()]);
    else
      return std::vector<std::string>();
  }

  void Object::emitEvent(const std::string& eventName,
                         qi::AutoMetaValue p1,
                         qi::AutoMetaValue p2,
                         qi::AutoMetaValue p3,
                         qi::AutoMetaValue p4,
                         qi::AutoMetaValue p5,
                         qi::AutoMetaValue p6,
                         qi::AutoMetaValue p7,
                         qi::AutoMetaValue p8)
  {
    qi::AutoMetaValue* vals[8]= {&p1, &p2, &p3, &p4, &p5, &p6, &p7, &p8};
    std::vector<qi::MetaValue> params;
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

