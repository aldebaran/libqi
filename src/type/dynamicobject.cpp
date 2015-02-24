/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/
#include <map>

#include <boost/make_shared.hpp>

#include <qi/api.hpp>
#include <qi/anyvalue.hpp>
#include <qi/type/typeinterface.hpp>
#include <qi/anyvalue.hpp>
#include <qi/anyobject.hpp>
#include <qi/anyfunction.hpp>
#include <qi/type/metaobject.hpp>
#include <qi/signal.hpp>
#include <qi/type/dynamicobject.hpp>
#include <qi/strand.hpp>

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
    typedef std::map<unsigned int, std::pair<SignalBase*, bool> > SignalMap;
    typedef std::map<unsigned int, std::pair<AnyFunction, MetaCallType> > MethodMap;
    SignalMap           signalMap;
    MethodMap           methodMap;
    MetaObject          meta;
    ObjectThreadingModel threadingModel;

    typedef std::map<unsigned int, std::pair<PropertyBase*, bool> > PropertyMap;
    PropertyMap propertyMap;

    ExecutionContext* getExecutionContext(
        qi::AnyObject context, MetaCallType methodThreadingModel);
  };

  DynamicObjectPrivate::DynamicObjectPrivate()
    : threadingModel(ObjectThreadingModel_Default)
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
      _p->signalMap[it->first] = std::make_pair(sb, false);
    }
  }

  DynamicObjectPrivate::~DynamicObjectPrivate()
  {
    //properties are also in signals, do not delete

    //only delete property we created
    for (PropertyMap::iterator it2 = propertyMap.begin(); it2 != propertyMap.end(); ++it2) {
      if (it2->second.second)
        delete it2->second.first;
    }

    //only delete signal we created
    for (SignalMap::iterator it = signalMap.begin(); it != signalMap.end(); ++it) {
      if (it->second.second)
        delete it->second.first;
    }
  }

  SignalBase* DynamicObjectPrivate::createSignal(unsigned int id)
  {

    SignalMap::iterator i = signalMap.find(id);
    if (i != signalMap.end())
      return i->second.first;
    if (meta.property(id))
    { // Replicate signal of prop in signalMap
      SignalBase* sb = property(id)->signal();
      signalMap[id] = std::make_pair(sb, false);
      return sb;
    }
    MetaSignal* sig = meta.signal(id);
    if (sig)
    {
      SignalBase* sb = new SignalBase(sig->parametersSignature());
      signalMap[id] = std::make_pair(sb, true);
      return sb;
    }
    return 0;
  }

  class DynamicObjectTypeInterface: public ObjectTypeInterface, public DefaultTypeImplMethods<DynamicObject>
  {
  public:
    DynamicObjectTypeInterface() {}
    virtual const MetaObject& metaObject(void* instance);
    virtual qi::Future<AnyReference> metaCall(void* instance, AnyObject context, unsigned int method, const GenericFunctionParameters& params, MetaCallType callType, Signature returnSignature);
    virtual void metaPost(void* instance, AnyObject context, unsigned int signal, const GenericFunctionParameters& params);
    virtual qi::Future<SignalLink> connect(void* instance, AnyObject context, unsigned int event, const SignalSubscriber& subscriber);
    /// Disconnect an event link. Returns if disconnection was successful.
    virtual qi::Future<void> disconnect(void* instance, AnyObject context, SignalLink linkId);
    virtual const std::vector<std::pair<TypeInterface*, int> >& parentTypes();
    virtual qi::Future<AnyValue> property(void* instance, AnyObject context, unsigned int id);
    virtual qi::Future<void> setProperty(void* instance, AnyObject context, unsigned int id, AnyValue val);
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
    _p->signalMap[id] = std::make_pair(signal, false);
  }


  void DynamicObject::setProperty(unsigned int id, PropertyBase* property)
  {
    _p->propertyMap[id] = std::make_pair(property, false);
  }

  const AnyFunction& DynamicObject::method(unsigned int id) const
  {
    static AnyFunction empty;
    DynamicObjectPrivate::MethodMap::iterator i = _p->methodMap.find(id);
    if (i == _p->methodMap.end())
      return empty;
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
      return i->second.first;
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
      propertyMap[id] = std::make_pair(res, true);
      return res;
    }
    else
      return i->second.first;
  }

  ExecutionContext* DynamicObjectPrivate::getExecutionContext(
      qi::AnyObject context, MetaCallType methodThreadingModel)
  {
    ExecutionContext* ec = context.executionContext();
    if (threadingModel == ObjectThreadingModel_SingleThread)
    {
      // execute queued methods on global eventloop if they are of queued type
      if (methodThreadingModel == MetaCallType_Queued)
        ec = 0;
      else if (!ec)
      {
        boost::shared_ptr<Manageable> manageable = context.managedObjectPtr();
        boost::mutex::scoped_lock l(manageable->initMutex());
        if (!manageable->executionContext())
          manageable->forceExecutionContext(boost::shared_ptr<qi::Strand>(
                new qi::Strand(*::qi::getEventLoop())));
        ec = context.executionContext();
      }
    }
    return ec;
  }

  qi::Future<AnyReference> DynamicObject::metaCall(AnyObject context, unsigned int method, const GenericFunctionParameters& params, MetaCallType callType, Signature returnSignature)
  {
    DynamicObjectPrivate::MethodMap::iterator i = _p->methodMap.find(method);
    if (i == _p->methodMap.end())
    {
      std::stringstream ss;
      ss << "Can't find methodID: " << method;
      return qi::makeFutureError<AnyReference>(ss.str());
    }
    if (returnSignature.isValid())
    {
      MetaMethod *mm = metaObject().method(method);
      if (!mm)
        return makeFutureError<AnyReference>("Unexpected error: MetaMethod not found");
      if (mm->returnSignature().isConvertibleTo(returnSignature) == 0)
      {
        if (returnSignature.isConvertibleTo(mm->returnSignature())==0)
          return makeFutureError<AnyReference>(
            "Call error: will not be able to convert return type from "
            + mm->returnSignature().toString()
            + " to " + returnSignature.toString());
        else
         qiLogWarning() << "Return signature might be incorrect depending on the value, from "
            + mm->returnSignature().toString()
            + " to " + returnSignature.toString();
      }
    }
    boost::shared_ptr<Manageable> m = context.managedObjectPtr();

    ExecutionContext* ec = _p->getExecutionContext(context, i->second.second);

    GenericFunctionParameters p;
    p.reserve(params.size()+1);
    if (method >= Manageable::startId && method < Manageable::endId)
      p.push_back(AnyReference::from(m));
    else
      p.push_back(AnyReference::from(this));
    p.insert(p.end(), params.begin(), params.end());
    return ::qi::metaCall(ec, _p->threadingModel,
      i->second.second, callType, context, method, i->second.first, p);
  }

  static void setPropertyValue(PropertyBase* property, AnyValue value)
  {
    property->setValue(value.asReference());
  }

  qi::Future<void> DynamicObject::metaSetProperty(AnyObject context, unsigned int id, AnyValue val)
  {
    ExecutionContext* ec = _p->getExecutionContext(context, MetaCallType_Auto);
    if (ec)
      return ec->async(boost::bind(&setPropertyValue, property(id), val));
    else
    {
      try
      {
        property(id)->setValue(val.asReference());
      }
      catch(const std::exception& e)
      {
        return qi::makeFutureError<void>(std::string("setProperty: ") + e.what());
      }
      return qi::Future<void>(0);
    }
  }

  qi::Future<AnyValue> DynamicObject::metaProperty(AnyObject context, unsigned int id)
  {
    PropertyBase* prop;
    try
    {
      prop = property(id);
    }
    catch (std::exception& e)
    {
      return qi::makeFutureError<AnyValue>(std::string("property: ") + e.what());
    }

    ExecutionContext* ec = _p->getExecutionContext(context, MetaCallType_Auto);
    if (ec)
      return ec->async<AnyValue>(boost::bind(&PropertyBase::value, prop));
    else
      return qi::Future<AnyValue>(prop->value());
  }

  static void reportError(qi::Future<AnyReference> fut) {
    if (fut.hasError()) {
      qiLogWarning() << "post on method failed: " << fut.error();
      return;
    }
    qi::AnyReference ref = fut.value();
    ref.destroy();
  }

  void DynamicObject::metaPost(AnyObject context, unsigned int event, const GenericFunctionParameters& params)
  {
    SignalBase * s = _p->createSignal(event);
    if (s)
    { // signal is declared, create if needed
      s->trigger(params);
      return;
    }

    // Allow emit on a method
    if (metaObject().method(event)) {
      qi::Future<AnyReference> fut = metaCall(context, event, params, MetaCallType_Queued);
      fut.connect(&reportError);
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

  //DynamicObjectTypeInterface implementation: just bounces everything to metaobject

  const MetaObject& DynamicObjectTypeInterface::metaObject(void* instance)
  {
    return reinterpret_cast<DynamicObject*>(instance)->metaObject();
  }

  qi::Future<AnyReference> DynamicObjectTypeInterface::metaCall(void* instance, AnyObject context, unsigned int method, const GenericFunctionParameters& params, MetaCallType callType, Signature returnSignature)
  {
    return reinterpret_cast<DynamicObject*>(instance)
      ->metaCall(context, method, params, callType, returnSignature);
  }

  void DynamicObjectTypeInterface::metaPost(void* instance, AnyObject context, unsigned int signal, const GenericFunctionParameters& params)
  {
    reinterpret_cast<DynamicObject*>(instance)->metaPost(context, signal, params);
  }

  qi::Future<SignalLink> DynamicObjectTypeInterface::connect(void* instance, AnyObject context, unsigned int event, const SignalSubscriber& subscriber)
  {
    return reinterpret_cast<DynamicObject*>(instance)->metaConnect(event, subscriber);
  }

  qi::Future<void> DynamicObjectTypeInterface::disconnect(void* instance, AnyObject context, SignalLink linkId)
  {
    return reinterpret_cast<DynamicObject*>(instance)->metaDisconnect(linkId);
  }

  const std::vector<std::pair<TypeInterface*, int> >& DynamicObjectTypeInterface::parentTypes()
  {
    static std::vector<std::pair<TypeInterface*, int> > empty;
    return empty;
  }

  qi::Future<AnyValue> DynamicObjectTypeInterface::property(void* instance, AnyObject context, unsigned int id)
  {
    return reinterpret_cast<DynamicObject*>(instance)
      ->metaProperty(context, id);
  }

  qi::Future<void> DynamicObjectTypeInterface::setProperty(void* instance, AnyObject context, unsigned int id, AnyValue value)
  {
    return reinterpret_cast<DynamicObject*>(instance)
      ->metaSetProperty(context, id, value);
  }

  static void cleanupDynamicObject(GenericObject *obj, bool destroyObject,
    boost::function<void (GenericObject*)> onDelete)
  {
    qiLogDebug() << "Cleaning up dynamic object " << obj << " delete=" << destroyObject
      << "  custom callback=" << !!onDelete;
    if (onDelete)
      onDelete(obj);
    if (destroyObject)
      delete reinterpret_cast<DynamicObject*>(obj->value);
    delete obj;
  }

  ObjectTypeInterface* getDynamicTypeInterface()
  {
    static DynamicObjectTypeInterface* type = 0;
    QI_THREADSAFE_NEW(type);
    return type;
  }

  AnyObject makeDynamicSharedAnyObjectImpl(DynamicObject* obj, boost::shared_ptr<Empty> other)
  {
    GenericObject* go = new GenericObject(getDynamicTypeInterface(), obj);
    return AnyObject(go, other);
  }

  AnyObject makeDynamicAnyObject(DynamicObject *obj, bool destroyObject,
    boost::function<void (GenericObject*)> onDelete)
  {
    ObjectTypeInterface* type = getDynamicTypeInterface();
    if (destroyObject || onDelete)
      return AnyObject(new GenericObject(type, obj),
        boost::bind(&cleanupDynamicObject, _1, destroyObject, onDelete));
    else
      return AnyObject(new GenericObject(type, obj), &AnyObject::deleteGenericObjectOnly);
  }

}
