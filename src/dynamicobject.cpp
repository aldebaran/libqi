/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/
#include <map>

#include <boost/make_shared.hpp>

#include <qitype/api.hpp>
#include <qitype/genericvalue.hpp>
#include <qitype/type.hpp>
#include <qitype/genericvalue.hpp>
#include <qitype/genericobject.hpp>
#include <qitype/functiontype.hpp>
#include <qitype/metaobject.hpp>
#include <qitype/signal.hpp>
#include <qitype/dynamicobject.hpp>


namespace qi
{

  class DynamicObjectPrivate
  {
  public:
    // get or create signal, or 0 if id is not an event
    SignalBase* createSignal(unsigned int id);
    bool                                dying;
    typedef std::map<unsigned int, SignalBase*> SignalMap;
    typedef std::map<unsigned int, GenericFunction> MethodMap;
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

  class DynamicObjectType: public ObjectType, public DefaultTypeImplMethods<DynamicObject>
  {
  public:
    DynamicObjectType() {}
    virtual const MetaObject& metaObject(void* instance);
    virtual qi::Future<GenericValue> metaCall(void* instance, unsigned int method, const GenericFunctionParameters& params, MetaCallType callType = MetaCallType_Auto);
    virtual void metaEmit(void* instance, unsigned int signal, const GenericFunctionParameters& params);
    virtual qi::Future<unsigned int> connect(void* instance, unsigned int event, const SignalSubscriber& subscriber);
    /// Disconnect an event link. Returns if disconnection was successful.
    virtual qi::Future<void> disconnect(void* instance, unsigned int linkId);
    virtual Manageable* manageable(void* intance);
    virtual const std::vector<std::pair<Type*, int> >& parentTypes();

    _QI_BOUNCE_TYPE_METHODS(DefaultTypeImplMethods<DynamicObject>);
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

  void DynamicObject::setMethod(unsigned int id, GenericFunction callable)
  {
    _p->methodMap[id] = callable;
  }

  GenericFunction DynamicObject::method(unsigned int id)
  {
    DynamicObjectPrivate::MethodMap::iterator i = _p->methodMap.find(id);
    if (i == _p->methodMap.end())
      return GenericFunction();
    else
      return i->second;
  }

  SignalBase* DynamicObject::signalBase(unsigned int id) const
  {
    DynamicObjectPrivate::SignalMap::iterator i = _p->signalMap.find(id);
    if (i == _p->signalMap.end())
      return 0;
    else
      return i->second;
  }

  qi::Future<GenericValue> DynamicObject::metaCall(unsigned int method, const GenericFunctionParameters& params, MetaCallType callType)
  {
    qi::Promise<GenericValue> out;
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

  void DynamicObject::metaEmit(unsigned int event, const GenericFunctionParameters& params)
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

  qi::Future<unsigned int> DynamicObject::connect(unsigned int event, const SignalSubscriber& subscriber)
  {
    SignalBase * s = _p->createSignal(event);
    if (!s)
      return qi::makeFutureError<unsigned int>("Cannot find signal");
    SignalBase::Link l = s->connect(subscriber);
    if (l > 0xFFFF)
      qiLogError("object") << "Signal id too big";
    return qi::Future<unsigned int>((event << 16) + l);
  }

  qi::Future<void> DynamicObject::disconnect(unsigned int linkId)
  {
    unsigned int event = linkId >> 16;
    unsigned int link = linkId & 0xFFFF;
    //TODO: weird to call createSignal in disconnect
    SignalBase* s = _p->createSignal(event);
    if (!s)
      return qi::makeFutureError<void>("Cannot find local signal connection.");
    bool b = s->disconnect(link);
    if (!b) {
      return qi::makeFutureError<void>("Cannot find local signal connection.");
    }
    return qi::Future<void>(0);
  }

  class MFunctorCall
  {
  public:
    MFunctorCall(GenericFunction& func, GenericFunctionParameters& params,
       qi::Promise<GenericValue>* out, bool noCloneFirst)
    : noCloneFirst(noCloneFirst)
    {
      this->out = out;
      std::swap(this->func, func);
      std::swap((std::vector<GenericValue>&) params,
        (std::vector<GenericValue>&) this->params);
    }
    MFunctorCall(const MFunctorCall& b)
    {
      (*this) = b;
    }
    void operator = (const MFunctorCall& b)
    {
      std::swap( (std::vector<GenericValue>&) params,
        (std::vector<GenericValue>&) b.params);
      std::swap(func, const_cast<MFunctorCall&>(b).func);
      this->out = b.out;
      noCloneFirst = b.noCloneFirst;
    }
    void operator()()
    {
      out->setValue(func.call(params));
      params.destroy(noCloneFirst);
      delete out;
    }
    qi::Promise<GenericValue>* out;
    GenericFunctionParameters params;
    GenericFunction func;
    bool noCloneFirst;
  };
  qi::Future<GenericValue> metaCall(EventLoop* el,
    GenericFunction func, const GenericFunctionParameters& params, MetaCallType callType, bool noCloneFirst)
  {
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
    {
      qi::Promise<GenericValue> out;
      out.setValue(func.call(params));
      return out.future();
    }
    else
    {
      qi::Promise<GenericValue>* out = new qi::Promise<GenericValue>();
      GenericFunctionParameters pCopy = params.copy(noCloneFirst);
      qi::Future<GenericValue> result = out->future();
      el->asyncCall(0,
        MFunctorCall(func, pCopy, out, noCloneFirst));
      return result;
    }
  }

  //DynamicObjectType implementation: just bounces everything to metaobject

  const MetaObject& DynamicObjectType::metaObject(void* instance)
  {
    return reinterpret_cast<DynamicObject*>(instance)->metaObject();
  }

  qi::Future<GenericValue> DynamicObjectType::metaCall(void* instance, unsigned int method, const GenericFunctionParameters& params, MetaCallType callType)
  {
    return reinterpret_cast<DynamicObject*>(instance)
      ->metaCall(method, params, callType);
  }

  void DynamicObjectType::metaEmit(void* instance, unsigned int signal, const GenericFunctionParameters& params)
  {
    reinterpret_cast<DynamicObject*>(instance)
      ->metaEmit(signal, params);
  }

  qi::Future<unsigned int> DynamicObjectType::connect(void* instance, unsigned int event, const SignalSubscriber& subscriber)
  {
    return reinterpret_cast<DynamicObject*>(instance)
      ->connect(event, subscriber);
  }

  qi::Future<void> DynamicObjectType::disconnect(void* instance, unsigned int linkId)
  {
    return reinterpret_cast<DynamicObject*>(instance)
      ->disconnect(linkId);
  }

  const std::vector<std::pair<Type*, int> >& DynamicObjectType::parentTypes()
  {
    static std::vector<std::pair<Type*, int> > empty;
    return empty;
  }

  Manageable* DynamicObjectType::manageable(void* instance)
  {
    return reinterpret_cast<DynamicObject*>(instance);
  }

  ObjectPtr     makeDynamicObjectPtr(DynamicObject *obj)
  {
    ObjectPtr op;
    static DynamicObjectType* type = new DynamicObjectType();
    op = ObjectPtr(new GenericObject(type, obj));
    return op;
  }

}
