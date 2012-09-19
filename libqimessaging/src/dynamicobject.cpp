/*
** Copyright (C) 2012 Aldebaran Robotics
*/

#include <map>

#include <boost/make_shared.hpp>

#include <qimessaging/api.hpp>
#include <qimessaging/genericvalue.hpp>
#include <qimessaging/type.hpp>
#include <qimessaging/genericvalue.hpp>
#include <qimessaging/genericobject.hpp>
#include <qimessaging/functiontype.hpp>
#include <qimessaging/metaobject.hpp>
#include <qimessaging/signal.hpp>
#include <qimessaging/dynamicobject.hpp>


namespace qi
{

  class DynamicObjectPrivate
  {
  public:
    // get or create signal, or 0 if id is not an event
    SignalBase* createSignal(unsigned int id);
    bool                                dying;
    typedef std::map<unsigned int, SignalBase*> SignalMap;
    typedef std::map<unsigned int, MetaCallable> MethodMap;
    SignalMap           signalMap;
    MethodMap           methodMap;
    MetaObject          meta;
  };

  SignalBase* DynamicObjectPrivate::createSignal(unsigned int id)
  {
    SignalMap::iterator i = signalMap.find(id);
    if (i != signalMap.end())
      return i->second;

    MetaSignal* sig = meta.signal(id);
    if (sig)
    {
      SignalBase* sb = new SignalBase(sig->signature());
      signalMap[id] = sb;
      return sb;
    }
    return 0;
  }

  class DynamicObjectType: public virtual ObjectType, public virtual TypeImpl<DynamicObject>
  {
  public:
    DynamicObjectType() {}
    virtual const MetaObject& metaObject(void* instance);
    virtual qi::Future<MetaFunctionResult> metaCall(void* instance, unsigned int method, const MetaFunctionParameters& params, MetaCallType callType = MetaCallType_Auto);
    virtual void metaEmit(void* instance, unsigned int signal, const MetaFunctionParameters& params);
    virtual unsigned int connect(void* instance, unsigned int event, const SignalSubscriber& subscriber);
    /// Disconnect an event link. Returns if disconnection was successful.
    virtual bool disconnect(void* instance, unsigned int linkId);
    virtual Manageable* manageable(void* intance);
  };

  DynamicObject::DynamicObject()
  {
    _p = boost::make_shared<DynamicObjectPrivate>();
  }

  DynamicObject::~DynamicObject()
  {
  }

  void DynamicObject::setMetaObject(const MetaObject& m)
  {
    _p->meta = m;
    // We will populate stuff on demand
  }

  MetaObject& DynamicObject::metaObject()
  {
    return _p->meta;
  }

  void DynamicObject::setMethod(unsigned int id, MetaCallable callable)
  {
    _p->methodMap[id] = callable;
  }

  SignalBase* DynamicObject::getSignal(unsigned int id)
  {
    DynamicObjectPrivate::SignalMap::iterator i = _p->signalMap.find(id);
    if (i == _p->signalMap.end())
      return 0;
    else
      return i->second;
  }

  qi::Future<MetaFunctionResult> DynamicObject::metaCall(unsigned int method, const MetaFunctionParameters& params, MetaCallType callType)
  {
    qi::Promise<MetaFunctionResult> out;
    DynamicObjectPrivate::MethodMap::iterator i = _p->methodMap.find(method);
    if (i == _p->methodMap.end())
    {
      std::stringstream ss;
      ss << "Can't find methodID: " << method;
      out.setError(ss.str());
      return out.future();
    }

    return ::qi::metaCall(eventLoop(), i->second, params, callType);
  }

  void DynamicObject::metaEmit(unsigned int event, const MetaFunctionParameters& params)
  {
    SignalBase * s = _p->createSignal(event);
    if (s)
    { // signal is declared, create if needed
      s->trigger(params);
    }
    else
    {
      // Allow emit on a method
      // FIXME: call errors are lost
      if (metaObject().method(event))
        metaCall(event, params, MetaCallType_Auto);
      else
        qiLogError("object") << "No such event " << event;
    }
  }

  unsigned int DynamicObject::connect(unsigned int event, const SignalSubscriber& subscriber)
  {
    SignalBase * s = _p->createSignal(event);
    if (!s)
      return -1;
    SignalBase::Link l = s->connect(subscriber);
     if (l > 0xFFFF)
      qiLogError("object") << "Signal id too big";
    return (event << 16) + l;
  }

  bool DynamicObject::disconnect(unsigned int linkId)
  {
    unsigned int event = linkId >> 16;
    unsigned int link = linkId & 0xFFFF;
    SignalBase* s = _p->createSignal(event);
    if (!s)
      return false;
    return s->disconnect(link);
  }

  static void functor_call(GenericFunction func, MetaFunctionParameters params,
    qi::Promise<MetaFunctionResult> out)
  {
    out.setValue(callFunction(func, params));
  }

  qi::Future<MetaFunctionResult> metaCall(EventLoop* el,
    GenericFunction func, const MetaFunctionParameters& params, MetaCallType callType)
  {

    qi::Promise<MetaFunctionResult> out;
    bool synchronous = true;
    switch (callType)
    {
    case qi::MetaCallType_Direct:
      break;
    case qi::MetaCallType_Auto:
      synchronous = !el ||  el->isInEventLoopThread();
      break;
    case qi::MetaCallType_Queued:
      synchronous = !el;
      break;
    }
    if (synchronous)
      out.setValue(callFunction(func, params));
    else
    {
      MetaFunctionParameters pCopy = params.copy();
      el->asyncCall(0,
        boost::bind(&functor_call,
          func,
          pCopy, out));
    }
    return out.future();
  }

  qi::Future<MetaFunctionResult> metaCall(EventLoop* el,
    GenericFunction func, const std::vector<GenericValue>& params, MetaCallType callType)
  {
    MetaFunctionParameters p(params, false);
    return metaCall(el, func, p, callType);
  }

  static void callable_call(MetaCallable func, MetaFunctionParameters params,
    qi::Promise<MetaFunctionResult> out)
  {
    out.setValue(func(params));
  }

  qi::Future<MetaFunctionResult> metaCall(EventLoop* el,
    MetaCallable func, const MetaFunctionParameters& params, MetaCallType callType)
  {

    qi::Promise<MetaFunctionResult> out;
    bool synchronous = true;
    switch (callType)
    {
    case qi::MetaCallType_Direct:
      break;
    case qi::MetaCallType_Auto:
      synchronous = !el ||  el->isInEventLoopThread();
      break;
    case qi::MetaCallType_Queued:
      synchronous = !el;
      break;
    }
    if (synchronous)
      out.setValue(func(params));
    else
    {
      MetaFunctionParameters pCopy = params.copy();
      el->asyncCall(0,
        boost::bind(&callable_call,
          func,
          pCopy, out));
    }
    return out.future();
  }

  //DynamicObjectType implementation: just bounces everything to metaobject

  const MetaObject& DynamicObjectType::metaObject(void* instance)
  {
    return reinterpret_cast<DynamicObject*>(instance)->metaObject();
  }
  qi::Future<MetaFunctionResult> DynamicObjectType::metaCall(void* instance, unsigned int method, const MetaFunctionParameters& params, MetaCallType callType)
  {
    return reinterpret_cast<DynamicObject*>(instance)
      ->metaCall(method, params, callType);
  }

  void DynamicObjectType::metaEmit(void* instance, unsigned int signal, const MetaFunctionParameters& params)
  {
    reinterpret_cast<DynamicObject*>(instance)
      ->metaEmit(signal, params);
  }
  unsigned int DynamicObjectType::connect(void* instance, unsigned int event, const SignalSubscriber& subscriber)
  {
    return reinterpret_cast<DynamicObject*>(instance)
      ->connect(event, subscriber);
  }
  bool DynamicObjectType::disconnect(void* instance, unsigned int linkId)
  {
    return reinterpret_cast<DynamicObject*>(instance)
      ->disconnect(linkId);
  }

  Manageable* DynamicObjectType::manageable(void* instance)
  {
    return reinterpret_cast<DynamicObject*>(instance);
  }

  GenericObject makeDynamicObject(DynamicObject* obj)
  {
    static DynamicObjectType* type = new DynamicObjectType();
    GenericObject result;
    result.type = type;
    result.value = obj;
    return result;
  }

}
