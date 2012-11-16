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
    DynamicObjectPrivate()
    : threadingModel(ObjectThreadingModel_SingleThread)
    {}
    ~DynamicObjectPrivate();
    // get or create signal, or 0 if id is not an event
    SignalBase* createSignal(unsigned int id);
    bool                                dying;
    typedef std::map<unsigned int, SignalBase*> SignalMap;
    typedef std::map<unsigned int,
      std::pair<GenericFunction, MetaCallType>
    > MethodMap;
    SignalMap           signalMap;
    MethodMap           methodMap;
    MetaObject          meta;
    ObjectThreadingModel threadingModel;
  };

  DynamicObjectPrivate::~DynamicObjectPrivate()
  {
    for (SignalMap::iterator it = signalMap.begin(); it!= signalMap.end(); ++it)
      delete it->second;
  }

  SignalBase* DynamicObjectPrivate::createSignal(unsigned int id)
  {
    SignalMap::iterator i = signalMap.find(id);
    if (i != signalMap.end())
      return i->second;

    MetaSignal* sig = meta.signal(id);
    if (sig)
    {
      SignalBase* sb = new SignalBase(qi::signatureSplit(sig->signature())[2]);
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
    virtual qi::Future<GenericValuePtr> metaCall(void* instance, Manageable* context, unsigned int method, const GenericFunctionParameters& params, MetaCallType callType = MetaCallType_Auto);
    virtual void metaPost(void* instance, Manageable* context, unsigned int signal, const GenericFunctionParameters& params);
    virtual qi::Future<unsigned int> connect(void* instance, Manageable* context, unsigned int event, const SignalSubscriber& subscriber);
    /// Disconnect an event link. Returns if disconnection was successful.
    virtual qi::Future<void> disconnect(void* instance, Manageable* context, unsigned int linkId);
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

  void DynamicObject::setThreadingModel(ObjectThreadingModel model)
  {
    _p->threadingModel = model;
  }

  ObjectThreadingModel DynamicObject::threadingModel() const
  {
    return _p->threadingModel;
  }

  void DynamicObject::setMethod(unsigned int id, GenericFunction callable, MetaCallType threadingModel)
  {
    _p->methodMap[id] = std::make_pair(callable, threadingModel);
  }

  GenericFunction DynamicObject::method(unsigned int id)
  {
    DynamicObjectPrivate::MethodMap::iterator i = _p->methodMap.find(id);
    if (i == _p->methodMap.end())
      return GenericFunction();
    else
      return i->second.first;
  }

  SignalBase* DynamicObject::signalBase(unsigned int id) const
  {
    DynamicObjectPrivate::SignalMap::iterator i = _p->signalMap.find(id);
    if (i == _p->signalMap.end())
      return 0;
    else
      return i->second;
  }

  qi::Future<GenericValuePtr> DynamicObject::metaCall(Manageable* context, unsigned int method, const GenericFunctionParameters& params, MetaCallType callType)
  {
    qi::Promise<GenericValuePtr> out;
    DynamicObjectPrivate::MethodMap::iterator i = _p->methodMap.find(method);
    if (i == _p->methodMap.end())
    {
      std::stringstream ss;
      ss << "Can't find methodID: " << method;
      out.setError(ss.str());
      return out.future();
    }
    return ::qi::metaCall(context->eventLoop(), _p->threadingModel,
      i->second.second, callType, context->mutex(), i->second.first, params);
  }

  void DynamicObject::metaPost(Manageable* context, unsigned int event, const GenericFunctionParameters& params)
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
        metaCall(context, event, params, MetaCallType_Auto);
      else
        qiLogError("object") << "No such event " << event;
    }
  }

  qi::Future<unsigned int> DynamicObject::metaConnect(unsigned int event, const SignalSubscriber& subscriber)
  {
    SignalBase * s = _p->createSignal(event);
    if (!s)
      return qi::makeFutureError<unsigned int>("Cannot find signal");
    SignalBase::Link l = s->connect(subscriber);
    if (l > 0xFFFF)
      qiLogError("object") << "Signal id too big";
    return qi::Future<unsigned int>((event << 16) + l);
  }

  qi::Future<void> DynamicObject::metaDisconnect(unsigned int linkId)
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

  static GenericValuePtr locked_call(GenericFunction& function,
                                     const GenericFunctionParameters& params,
                                     Manageable::TimedMutexPtr& lock)
  {
    static long msWait = -1;
    if (msWait == -1)
    { // thread-safeness: worst case we set it twice
      std::string s = os::getenv("QI_DEADLOCK_TIMEOUT");
      if (s.empty())
        msWait = 30000; // default wait of 30 seconds
      else
        msWait = strtol(s.c_str(), 0, 0);
    }
    if (!msWait)
    {
       boost::timed_mutex::scoped_lock l(*lock);
       return function.call(params);
    }
    else
    {
      boost::system_time timeout = boost::get_system_time() + boost::posix_time::milliseconds(msWait);
      boost::timed_mutex::scoped_lock l(*lock, timeout);
      if (!l.owns_lock())
        throw std::runtime_error("Time-out acquiring lock. Deadlock?");
      return function.call(params);
    }
  }

  class MFunctorCall
  {
  public:
    MFunctorCall(GenericFunction& func, GenericFunctionParameters& params,
       qi::Promise<GenericValuePtr>* out, bool noCloneFirst, Manageable::TimedMutexPtr lock)
    : noCloneFirst(noCloneFirst)
    {
      this->out = out;
      std::swap(this->lock, lock);
      std::swap(this->func, func);
      std::swap((std::vector<GenericValuePtr>&) params,
        (std::vector<GenericValuePtr>&) this->params);
    }
    MFunctorCall(const MFunctorCall& b)
    {
      (*this) = b;
    }
    void operator = (const MFunctorCall& b)
    {
      // Implement move semantic on =
      std::swap( (std::vector<GenericValuePtr>&) params,
        (std::vector<GenericValuePtr>&) b.params);
      std::swap(func, const_cast<MFunctorCall&>(b).func);
      std::swap(this->lock, const_cast<MFunctorCall&>(b).lock);
      this->out = b.out;
      noCloneFirst = b.noCloneFirst;
    }
    void operator()()
    {
      try
      {
        if (lock)
          out->setValue(locked_call(func, params, lock));
        else
          out->setValue(func.call(params));
      }
      catch(const std::exception& e)
      {
        out->setError(e.what());
      }
      catch(...)
      {
        out->setError("Unknown exception caught.");
      }
      params.destroy(noCloneFirst);
      delete out;
    }
    qi::Promise<GenericValuePtr>* out;
    GenericFunctionParameters params;
    GenericFunction func;
    bool noCloneFirst;
    Manageable::TimedMutexPtr lock;
  };

  qi::Future<GenericValuePtr> metaCall(EventLoop* el,
    ObjectThreadingModel objectThreadingModel,
    MetaCallType methodThreadingModel,
    MetaCallType callType,
    Manageable::TimedMutexPtr objectLock,
    GenericFunction func, const GenericFunctionParameters& params, bool noCloneFirst)
  {
    // Implement rules described in header
    bool sync = true;
    if (el)
      sync = el->isInEventLoopThread();
    else if (methodThreadingModel != MetaCallType_Auto)
      sync = (methodThreadingModel == MetaCallType_Direct);
    else // callType default is synchronous
      sync = (callType != MetaCallType_Queued);

    if (!sync && !el)
      el = getDefaultThreadPoolEventLoop();
    qiLogDebug("qi.Object") << "metacall sync=" << sync << " el= " << el <<" ct= " << callType;
    if (sync)
    {
      qi::Promise<GenericValuePtr> out;
      if (objectThreadingModel == ObjectThreadingModel_SingleThread
        && methodThreadingModel == MetaCallType_Auto)
        out.setValue(locked_call(func, params, objectLock));
      else
        out.setValue(func.call(params));
      return out.future();
    }
    else
    {
      qi::Promise<GenericValuePtr>* out = new qi::Promise<GenericValuePtr>();
      GenericFunctionParameters pCopy = params.copy(noCloneFirst);
      qi::Future<GenericValuePtr> result = out->future();
      if (!(objectThreadingModel == ObjectThreadingModel_SingleThread
        && methodThreadingModel == MetaCallType_Auto))
        objectLock.reset();
      el->post(MFunctorCall(func, pCopy, out, noCloneFirst, objectLock));
      return result;
    }
  }

  //DynamicObjectType implementation: just bounces everything to metaobject

  const MetaObject& DynamicObjectType::metaObject(void* instance)
  {
    return reinterpret_cast<DynamicObject*>(instance)->metaObject();
  }

  qi::Future<GenericValuePtr> DynamicObjectType::metaCall(void* instance, Manageable* context, unsigned int method, const GenericFunctionParameters& params, MetaCallType callType)
  {
    return reinterpret_cast<DynamicObject*>(instance)
      ->metaCall(context, method, params, callType);
  }

  void DynamicObjectType::metaPost(void* instance, Manageable* context, unsigned int signal, const GenericFunctionParameters& params)
  {
    reinterpret_cast<DynamicObject*>(instance)
      ->metaPost(context, signal, params);
  }

  qi::Future<unsigned int> DynamicObjectType::connect(void* instance, Manageable* context, unsigned int event, const SignalSubscriber& subscriber)
  {
    return reinterpret_cast<DynamicObject*>(instance)
      ->metaConnect(event, subscriber);
  }

  qi::Future<void> DynamicObjectType::disconnect(void* instance, Manageable* context, unsigned int linkId)
  {
    return reinterpret_cast<DynamicObject*>(instance)
      ->metaDisconnect(linkId);
  }

  const std::vector<std::pair<Type*, int> >& DynamicObjectType::parentTypes()
  {
    static std::vector<std::pair<Type*, int> > empty;
    return empty;
  }

  static void cleanupDynamicObject(GenericObject *obj, bool destroyObject,
    boost::function<void (GenericObject*)> onDelete)
  {
    if (onDelete)
      onDelete(obj);
    if (destroyObject)
      delete reinterpret_cast<DynamicObject*>(obj->value);
    delete obj;
  }

  ObjectPtr     makeDynamicObjectPtr(DynamicObject *obj, bool destroyObject,
    boost::function<void (GenericObject*)> onDelete)
  {
    ObjectPtr op;
    static DynamicObjectType* type = new DynamicObjectType();
    if (destroyObject || onDelete)
      op = ObjectPtr(new GenericObject(type, obj),
        boost::bind(&cleanupDynamicObject, _1, destroyObject, onDelete));
    else
      op = ObjectPtr(new GenericObject(type, obj));
    return op;
  }

}
