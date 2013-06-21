/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/
#include <map>

#include <boost/make_shared.hpp>

#include <qitype/api.hpp>
#include <qitype/anyvalue.hpp>
#include <qitype/typeinterface.hpp>
#include <qitype/anyvalue.hpp>
#include <qitype/anyobject.hpp>
#include <qitype/anyfunction.hpp>
#include <qitype/metaobject.hpp>
#include <qitype/signal.hpp>
#include <qitype/dynamicobject.hpp>


qiLogCategory("qitype.dynamicobject");

namespace qi
{

  class DynamicObjectPrivate
  {
  public:
    DynamicObjectPrivate();

    ~DynamicObjectPrivate();
    // get or create signal, or 0 if id is not an event
    SignalBase* createSignal(unsigned int id);
    PropertyBase* property(unsigned int id);
    bool                                dying;
    typedef std::map<unsigned int, SignalBase*> SignalMap;
    typedef std::map<unsigned int,
      std::pair<AnyFunction, MetaCallType>
    > MethodMap;
    SignalMap           signalMap;
    MethodMap           methodMap;
    MetaObject          meta;
    ObjectThreadingModel threadingModel;

    typedef std::map<unsigned int, PropertyBase*> PropertyMap;
    PropertyMap propertyMap;
  };

  DynamicObjectPrivate::DynamicObjectPrivate()
  : threadingModel(ObjectThreadingModel_SingleThread)
  {
  }

  void DynamicObject::setManageable(Manageable* m)
  {
    _p->methodMap.insert(Manageable::manageableMmethodMap().begin(),
      Manageable::manageableMmethodMap().end());
    _p->meta = MetaObject::merge(_p->meta, Manageable::manageableMetaObject());
    Manageable::SignalMap& smap = Manageable::manageableSignalMap();
    // need to convert signal getters to signal, we have the instance
    for (Manageable::SignalMap::iterator it = smap.begin(); it != smap.end(); ++it)
    {
      SignalBase* sb = it->second(m);
      _p->signalMap[it->first] = sb;
    }
  }

  DynamicObjectPrivate::~DynamicObjectPrivate()
  {
    for (SignalMap::iterator it = signalMap.begin(); it!= signalMap.end(); ++it)
    {
      if (it->first >= Manageable::endId)
        delete it->second;
    }
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

  class DynamicObjectTypeInterface: public ObjectTypeInterface, public DefaultTypeImplMethods<DynamicObject>
  {
  public:
    DynamicObjectTypeInterface() {}
    virtual const MetaObject& metaObject(void* instance);
    virtual qi::Future<AnyReference> metaCall(void* instance, Manageable* context, unsigned int method, const GenericFunctionParameters& params, MetaCallType callType = MetaCallType_Auto);
    virtual void metaPost(void* instance, Manageable* context, unsigned int signal, const GenericFunctionParameters& params);
    virtual qi::Future<SignalLink> connect(void* instance, Manageable* context, unsigned int event, const SignalSubscriber& subscriber);
    /// Disconnect an event link. Returns if disconnection was successful.
    virtual qi::Future<void> disconnect(void* instance, Manageable* context, SignalLink linkId);
    virtual const std::vector<std::pair<TypeInterface*, int> >& parentTypes();
    virtual qi::Future<AnyValue> property(void* instance, unsigned int id);
    virtual qi::Future<void> setProperty(void* instance, unsigned int id, AnyValue val);
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

  void DynamicObject::setMethod(unsigned int id, AnyFunction callable, MetaCallType threadingModel)
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

  AnyFunction DynamicObject::method(unsigned int id) const
  {
    DynamicObjectPrivate::MethodMap::iterator i = _p->methodMap.find(id);
    if (i == _p->methodMap.end())
      return AnyFunction();
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
      qi::Signature sig = p->signature();
      TypeInterface* type = TypeInterface::fromSignature(sig);
      if (!type)
        throw std::runtime_error("Unable to construct a type from " + sig.toString());
      PropertyBase* res = new GenericProperty(type);
      propertyMap[id] = res;
      return res;
    }
    else
      return i->second;
  }

  qi::Future<AnyReference> DynamicObject::metaCall(Manageable* context, unsigned int method, const GenericFunctionParameters& params, MetaCallType callType)
  {
    qi::Promise<AnyReference> out;
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
    if (method >= Manageable::startId && method < Manageable::endId)
      p.push_back(AnyReference(context));
    else
      p.push_back(AnyReference(this));
    p.insert(p.end(), params.begin(), params.end());
    return ::qi::metaCall(context->eventLoop(), _p->threadingModel,
      i->second.second, callType, context, method, i->second.first, p);
  }

  qi::Future<void> DynamicObject::metaSetProperty(unsigned int id, AnyValue val)
  {
    try
    {
      property(id)->setValue(val);
    }
    catch(const std::exception& e)
    {
      return qi::makeFutureError<void>(std::string("setProperty: ") + e.what());
    }
    return qi::Future<void>(0);
  }

  qi::Future<AnyValue> DynamicObject::metaProperty(unsigned int id)
  {
    return qi::Future<AnyValue>(property(id)->value());
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

  qi::Future<SignalLink> DynamicObject::metaConnect(unsigned int event, const SignalSubscriber& subscriber)
  {
    SignalBase * s = _p->createSignal(event);
    if (!s)
      return qi::makeFutureError<SignalLink>("Cannot find signal");
    SignalLink l = s->connect(subscriber);
    if (l == SignalBase::invalidSignalLink)
      return qi::Future<SignalLink>(l);
    SignalLink link = ((SignalLink)event << 32) + l;
    assert(link >> 32 == event);
    assert((link & 0xFFFFFFFF) == l);
    qiLogDebug() << "New subscriber " << link <<" to event " << event;
    return qi::Future<SignalLink>(link);
  }

  qi::Future<void> DynamicObject::metaDisconnect(SignalLink linkId)
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

  static AnyReference locked_call(AnyFunction& function,
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
  namespace {

    inline void call(qi::Promise<AnyReference>& out,
                      Manageable* context,
                      bool lock,
                      const GenericFunctionParameters& params,
                      unsigned int methodId,
                      AnyFunction& func
                      )
    {
      bool stats = context && context->isStatsEnabled();
      bool trace = context && context->isTraceEnabled();
      int tid = 0; // trace call id, reused for result sending
      if (trace)
      {
        tid = context->_nextTraceId();
        qi::os::timeval tv;
        qi::os::gettimeofday(&tv);
        std::vector<AnyValue> args;
        args.resize(params.size()-1);
        for (unsigned i=0; i<params.size()-1; ++i)
        {
          if (!params[i+1].type)
            args[i] = AnyValue::from("<??" ">");
          else
          {
            switch(params[i+1].type->kind())
            {
            case TypeKind_Int:
            case TypeKind_String:
            case TypeKind_Float:
            case TypeKind_List:
            case TypeKind_Map:
            case TypeKind_Tuple:
            case TypeKind_Dynamic:
              args[i] = params[i+1];
              break;
            default:
            args[i] = AnyValue::from("<??" ">");
            }
          }
        }
        context->traceObject(EventTrace(
          tid, EventTrace::Event_Call, methodId, AnyValue::from(args), tv));
      }
      qi::int64_t time = stats?qi::os::ustime():0;
      bool success = false;
      try
      {
        if (lock)
          out.setValue(locked_call(func, params, context->mutex()));
        else
          out.setValue(func.call(params));
        success = true;
      }
      catch(const std::exception& e)
      {
        out.setError(e.what());
      }
      catch(...)
      {
        out.setError("Unknown exception caught.");
      }
      if (stats)
        context->pushStats(methodId, (float)(qi::os::ustime() - time)/1e6f);
      if (trace)
      {
        qi::os::timeval tv;
        qi::os::gettimeofday(&tv);
        AnyValue val;
        if (success)
          val = out.future().value();
        else
          val = AnyValue::from(out.future().error());
        context->traceObject(EventTrace(tid,
          success?EventTrace::Event_Result:EventTrace::Event_Error,
          methodId, val, tv));
      }
    }
  }

  class MFunctorCall
  {
  public:
    MFunctorCall(AnyFunction& func, GenericFunctionParameters& params,
       qi::Promise<AnyReference>* out, bool noCloneFirst, Manageable* context, unsigned int methodId, bool lock)
    : noCloneFirst(noCloneFirst)
    {
      this->out = out;
      this->methodId = methodId;
      this->context = context;
      this->lock = lock;
      std::swap(this->func, func);
      std::swap((std::vector<AnyReference>&) params,
        (std::vector<AnyReference>&) this->params);
    }
    MFunctorCall(const MFunctorCall& b)
    {
      (*this) = b;
    }
    void operator = (const MFunctorCall& b)
    {
      // Implement move semantic on =
      std::swap( (std::vector<AnyReference>&) params,
        (std::vector<AnyReference>&) b.params);
      std::swap(func, const_cast<MFunctorCall&>(b).func);
      context = b.context;
      methodId = b.methodId;
      this->lock = b.lock;
      this->out = b.out;
      noCloneFirst = b.noCloneFirst;
    }
    void operator()()
    {
      call(*out, context, lock, params, methodId, func);
      params.destroy(noCloneFirst);
      delete out;
    }
    qi::Promise<AnyReference>* out;
    GenericFunctionParameters params;
    AnyFunction func;
    bool noCloneFirst;
    Manageable* context;
    bool lock;
    unsigned int methodId;
  };

  qi::Future<AnyReference> metaCall(EventLoop* el,
    ObjectThreadingModel objectThreadingModel,
    MetaCallType methodThreadingModel,
    MetaCallType callType,
    Manageable* context,
    unsigned int methodId,
    AnyFunction func, const GenericFunctionParameters& params, bool noCloneFirst)
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
    bool doLock = (context && objectThreadingModel == ObjectThreadingModel_SingleThread
        && methodThreadingModel == MetaCallType_Auto);
    if (sync)
    {
      qi::Promise<AnyReference> out;
      call(out, context, doLock, params, methodId, func);
      return out.future();
    }
    else
    {
      qi::Promise<AnyReference>* out = new qi::Promise<AnyReference>();
      GenericFunctionParameters pCopy = params.copy(noCloneFirst);
      qi::Future<AnyReference> result = out->future();
      el->post(MFunctorCall(func, pCopy, out, noCloneFirst, context, methodId, doLock));
      return result;
    }
  }

  //DynamicObjectTypeInterface implementation: just bounces everything to metaobject

  const MetaObject& DynamicObjectTypeInterface::metaObject(void* instance)
  {
    return reinterpret_cast<DynamicObject*>(instance)->metaObject();
  }

  qi::Future<AnyReference> DynamicObjectTypeInterface::metaCall(void* instance, Manageable* context, unsigned int method, const GenericFunctionParameters& params, MetaCallType callType)
  {
    return reinterpret_cast<DynamicObject*>(instance)
      ->metaCall(context, method, params, callType);
  }

  void DynamicObjectTypeInterface::metaPost(void* instance, Manageable* context, unsigned int signal, const GenericFunctionParameters& params)
  {
    reinterpret_cast<DynamicObject*>(instance)->metaPost(context, signal, params);
  }

  qi::Future<SignalLink> DynamicObjectTypeInterface::connect(void* instance, Manageable* context, unsigned int event, const SignalSubscriber& subscriber)
  {
    return reinterpret_cast<DynamicObject*>(instance)->metaConnect(event, subscriber);
  }

  qi::Future<void> DynamicObjectTypeInterface::disconnect(void* instance, Manageable* context, SignalLink linkId)
  {
    return reinterpret_cast<DynamicObject*>(instance)->metaDisconnect(linkId);
  }

  const std::vector<std::pair<TypeInterface*, int> >& DynamicObjectTypeInterface::parentTypes()
  {
    static std::vector<std::pair<TypeInterface*, int> > empty;
    return empty;
  }

  qi::Future<AnyValue> DynamicObjectTypeInterface::property(void* instance, unsigned int id)
  {
    return reinterpret_cast<DynamicObject*>(instance)
      ->metaProperty(id);
  }

  qi::Future<void> DynamicObjectTypeInterface::setProperty(void* instance, unsigned int id, AnyValue value)
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

  AnyObject     makeDynamicAnyObject(DynamicObject *obj, bool destroyObject,
    boost::function<void (GenericObject*)> onDelete)
  {
    AnyObject op;
    static DynamicObjectTypeInterface* type = new DynamicObjectTypeInterface();
    if (destroyObject || onDelete)
      op = AnyObject(new GenericObject(type, obj),
        boost::bind(&cleanupDynamicObject, _1, destroyObject, onDelete));
    else
      op = AnyObject(new GenericObject(type, obj));
    return op;
  }

}
