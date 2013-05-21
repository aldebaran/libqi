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


qiLogCategory("qitype.dynamicobject");

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
    PropertyBase* property(unsigned int id);
    bool                                dying;
    typedef std::map<unsigned int, SignalBase*> SignalMap;
    typedef std::map<unsigned int,
      std::pair<GenericFunction, MetaCallType>
    > MethodMap;
    SignalMap           signalMap;
    MethodMap           methodMap;
    MetaObject          meta;
    ObjectThreadingModel threadingModel;

    typedef std::map<unsigned int, PropertyBase*> PropertyMap;
    PropertyMap propertyMap;
  };

  DynamicObjectPrivate::~DynamicObjectPrivate()
  {
    for (SignalMap::iterator it = signalMap.begin(); it!= signalMap.end(); ++it)
      delete it->second;
    //properties are also in signals, do not delete
  }

  SignalBase* DynamicObjectPrivate::createSignal(unsigned int id)
  {

    SignalMap::iterator i = signalMap.find(id);
    if (i != signalMap.end())
      return i->second;
    if (meta.property(id))
    { // Replicate signal of prop in signalMap
      SignalBase* sb = property(id)->signal();
      signalMap[id] = sb;
      return sb;
    }
    MetaSignal* sig = meta.signal(id);
    if (sig)
    {
      SignalBase* sb = new SignalBase(sig->parametersSignature());
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
    virtual qi::Future<Link> connect(void* instance, Manageable* context, unsigned int event, const SignalSubscriber& subscriber);
    /// Disconnect an event link. Returns if disconnection was successful.
    virtual qi::Future<void> disconnect(void* instance, Manageable* context, Link linkId);
    virtual const std::vector<std::pair<Type*, int> >& parentTypes();
    virtual qi::Future<GenericValue> getProperty(void* instance, unsigned int id);
    virtual qi::Future<void> setProperty(void* instance, unsigned int id, GenericValue val);
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

  void DynamicObject::setSignal(unsigned int id, SignalBase* signal)
  {
    _p->signalMap[id] = new SignalBase(*signal);
  }


  void DynamicObject::setProperty(unsigned int id, PropertyBase* property)
  {
    _p->propertyMap[id] = property;
  }

  GenericFunction DynamicObject::method(unsigned int id) const
  {
    DynamicObjectPrivate::MethodMap::iterator i = _p->methodMap.find(id);
    if (i == _p->methodMap.end())
      return GenericFunction();
    else
      return i->second.first;
  }

  SignalBase* DynamicObject::signal(unsigned int id) const
  {
    if (_p->meta.property(id))
      return const_cast<DynamicObject*>(this)->property(id)->signal();
    DynamicObjectPrivate::SignalMap::iterator i = _p->signalMap.find(id);
    if (i == _p->signalMap.end())
      return 0;
    else
      return i->second;
  }

  PropertyBase* DynamicObject::property(unsigned int id) const
  {
    return _p->property(id);
  }

  PropertyBase* DynamicObjectPrivate::property(unsigned int id)
  {
    DynamicObjectPrivate::PropertyMap::iterator i = propertyMap.find(id);
    if (i == propertyMap.end())
    {
      MetaProperty* p = meta.property(id);
      if (!p)
        throw std::runtime_error("Id is not id of a property");
      // Fetch its type
      std::string sig = p->signature();
      Type* type = Type::fromSignature(sig);
      if (!type)
        throw std::runtime_error("Unable to construct a type from " + sig);
      PropertyBase* res = new GenericProperty(type);
      propertyMap[id] = res;
      return res;
    }
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
    GenericFunctionParameters p;
    p.reserve(params.size()+1);
    p.push_back(GenericValueRef(this));
    p.insert(p.end(), params.begin(), params.end());
    return ::qi::metaCall(context->eventLoop(), _p->threadingModel,
      i->second.second, callType, context, method, i->second.first, p);
  }

  qi::Future<void> DynamicObject::metaSetProperty(unsigned int id, GenericValue val)
  {
    try
    {
      property(id)->setValue(val);
    }
    catch(const std::exception& e)
    {
      return qi::makeFutureError<void>(std::string("setProperty: ") + e.what());
    }
    qi::Promise<void> p;
    p.setValue(0);
    return p.future();
  }

  qi::Future<GenericValue> DynamicObject::metaProperty(unsigned int id)
  {
    qi::Promise<GenericValue> p;
    p.setValue(property(id)->value());
    return p.future();
  }

  void DynamicObject::metaPost(Manageable* context, unsigned int event, const GenericFunctionParameters& params)
  {
    SignalBase * s = _p->createSignal(event);
    if (s)
    { // signal is declared, create if needed
      s->trigger(params);
      return;
    }

    // Allow emit on a method
    // FIXME: call errors are lost
    if (metaObject().method(event)) {
      metaCall(context, event, params, MetaCallType_Auto);
      //TODO: return metacall status: check params
      return;
    }
    qiLogError() << "No such event " << event;
    return;
  }

  qi::Future<Link> DynamicObject::metaConnect(unsigned int event, const SignalSubscriber& subscriber)
  {
    SignalBase * s = _p->createSignal(event);
    if (!s)
      return qi::makeFutureError<Link>("Cannot find signal");
    SignalBase::Link l = s->connect(subscriber);
    Link link = ((Link)event << 32) + l;
    assert(link >> 32 == event);
    assert((link & 0xFFFFFFFF) == l);
    qiLogDebug() << "New subscriber " << link <<" to event " << event;
    return qi::Future<Link>(link);
  }

  qi::Future<void> DynamicObject::metaDisconnect(Link linkId)
  {
    unsigned int event = linkId >> 32;
    unsigned int link = linkId & 0xFFFFFFFF;
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
                                     Manageable::TimedMutexPtr lock)
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
      qiLogDebug() << "Aquiering module lock...";
      boost::timed_mutex::scoped_lock l(*lock, timeout);
      qiLogDebug() << "Checking lock acquisition...";
      if (!l.owns_lock())
      {
        qiLogWarning() << "Time-out acquiring object lock when calling method. Deadlock?";
        throw std::runtime_error("Time-out acquiring lock. Deadlock?");
      }
      return function.call(params);
    }
  }

  class MFunctorCall
  {
  public:
    MFunctorCall(GenericFunction& func, GenericFunctionParameters& params,
       qi::Promise<GenericValuePtr>* out, bool noCloneFirst, Manageable* context, unsigned int methodId, bool lock)
    : noCloneFirst(noCloneFirst)
    {
      this->out = out;
      this->methodId = methodId;
      this->context = context;
      this->lock = lock;
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
      context = b.context;
      methodId = b.methodId;
      this->lock = b.lock;
      this->out = b.out;
      noCloneFirst = b.noCloneFirst;
    }
    void operator()()
    {
      try
      {
        bool stats = context && context->isStatsEnabled();
        qi::int64_t time = stats?qi::os::ustime():0;
        if (lock)
          out->setValue(locked_call(func, params, context->mutex()));
        else
          out->setValue(func.call(params));
        if (stats)
          context->pushStats(methodId, (float)(qi::os::ustime() - time)/1e6);
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
    Manageable* context;
    bool lock;
    unsigned int methodId;
  };

  qi::Future<GenericValuePtr> metaCall(EventLoop* el,
    ObjectThreadingModel objectThreadingModel,
    MetaCallType methodThreadingModel,
    MetaCallType callType,
    Manageable* context,
    unsigned int methodId,
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
    qiLogDebug() << "metacall sync=" << sync << " el= " << el <<" ct= " << callType;
    if (sync)
    {
      qi::Promise<GenericValuePtr> out;
      bool stats = context && context->isStatsEnabled();
      qi::int64_t time = stats?qi::os::ustime():0;
      if (context && objectThreadingModel == ObjectThreadingModel_SingleThread
        && methodThreadingModel == MetaCallType_Auto)
        out.setValue(locked_call(func, params, context->mutex()));
      else
        out.setValue(func.call(params));
      if (stats)
        context->pushStats(methodId, (float)(qi::os::ustime() - time)/1e6);
      return out.future();
    }
    else
    {
      qi::Promise<GenericValuePtr>* out = new qi::Promise<GenericValuePtr>();
      GenericFunctionParameters pCopy = params.copy(noCloneFirst);
      qi::Future<GenericValuePtr> result = out->future();
      bool doLock = (context && objectThreadingModel == ObjectThreadingModel_SingleThread
        && methodThreadingModel == MetaCallType_Auto);
      el->post(MFunctorCall(func, pCopy, out, noCloneFirst, context, methodId, doLock));
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
    reinterpret_cast<DynamicObject*>(instance)->metaPost(context, signal, params);
  }

  qi::Future<Link> DynamicObjectType::connect(void* instance, Manageable* context, unsigned int event, const SignalSubscriber& subscriber)
  {
    return reinterpret_cast<DynamicObject*>(instance)->metaConnect(event, subscriber);
  }

  qi::Future<void> DynamicObjectType::disconnect(void* instance, Manageable* context, Link linkId)
  {
    return reinterpret_cast<DynamicObject*>(instance)->metaDisconnect(linkId);
  }

  const std::vector<std::pair<Type*, int> >& DynamicObjectType::parentTypes()
  {
    static std::vector<std::pair<Type*, int> > empty;
    return empty;
  }

  qi::Future<GenericValue> DynamicObjectType::getProperty(void* instance, unsigned int id)
  {
    return reinterpret_cast<DynamicObject*>(instance)
      ->metaProperty(id);
  }

  qi::Future<void> DynamicObjectType::setProperty(void* instance, unsigned int id, GenericValue value)
  {
    return reinterpret_cast<DynamicObject*>(instance)
      ->metaSetProperty(id, value);
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
