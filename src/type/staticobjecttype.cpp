/*
**  Copyright (C) 2012, 2013 Aldebaran Robotics
**  See COPYING for the license
*/
#include "staticobjecttype.hpp"
#include <qi/signal.hpp>
#include <qi/jsoncodec.hpp>

qiLogCategory("qitype.object");

namespace qi
{

void StaticObjectTypeBase::initialize(const MetaObject& mo, const ObjectTypeData& data)
{
  _metaObject = mo;
  _data = data;
}


const MetaObject&
StaticObjectTypeBase::metaObject(void* )
{
  return _metaObject;
}


qi::Future<AnyReference>
StaticObjectTypeBase::metaCall(void* instance, AnyObject context, unsigned int methodId,
                               const GenericFunctionParameters& params,
                               MetaCallType callType, Signature returnSignature)
{
  ObjectTypeData::MethodMap::iterator i;
  i = _data.methodMap.find(methodId);
  if (i == _data.methodMap.end())
  {
    return qi::makeFutureError<AnyReference>("No such method");
  }

  if (returnSignature.isValid())
  {
    const MetaMethod *mm = metaObject(instance).method(methodId);
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

  ExecutionContext* ec = context.executionContext();
  MetaCallType methodThreadingModel = i->second.second;

  AnyFunction method = i->second.first;
  AnyReference self;
  if (methodId >= Manageable::startId  && methodId < Manageable::endId)
  {
    self = AnyReference(qi::typeOf<Manageable>(), static_cast<Manageable*>(context.asGenericObject()));
  }
  else
  {
    self = AnyReference(this, instance);
  }
  GenericFunctionParameters p2;
  p2.reserve(params.size()+1);
  p2.push_back(self);
  p2.insert(p2.end(), params.begin(), params.end());

  return ::qi::metaCall(ec, _data.threadingModel, methodThreadingModel, callType, context, methodId, method, p2, true);
}


static PropertyBase* property(ObjectTypeData& data, void* instance, unsigned int signal)
{
  ObjectTypeData::PropertyGetterMap::iterator i;
  i = data.propertyGetterMap.find(signal);
  if (i == data.propertyGetterMap.end())
  {
    qiLogError() << "No such property " << signal;
    return 0;
  }
  PropertyBase* sig = i->second(instance);
  if (!sig)
  {
    qiLogError() << "Property getter returned NULL";
    return 0;
  }
  return sig;
}

static SignalBase* getSignal(ObjectTypeData& data, void* instance, unsigned int signal)
{
  ObjectTypeData::SignalGetterMap::iterator i;
  i = data.signalGetterMap.find(signal);
  if (i == data.signalGetterMap.end())
  {
    PropertyBase* prop = property(data, instance, signal);
    if (prop)
      return prop->signal();
    qiLogError() << "No such signal " << signal;
    return 0;
  }
  SignalBase* sig = i->second(instance);
  if (!sig)
  {
    qiLogError() << "Signal getter returned NULL";
    return 0;
  }
  return sig;
}

static void reportError(qi::Future<AnyReference> fut) {
  if (fut.hasError()) {
    qiLogError() << fut.error();
    return;
  }
  qi::AnyReference ref = fut.value();
  ref.destroy();
}

void StaticObjectTypeBase::metaPost(void* instance, AnyObject context, unsigned int signal,
                                    const GenericFunctionParameters& params)
{
  SignalBase* sb = getSignal(_data, instance, signal);
  if (sb)
  {
    sb->trigger(params);
  }
  else
  { // try method
    qi::Future<AnyReference> fut = metaCall(instance, context, signal, params, MetaCallType_Queued, Signature());
    fut.connect(&reportError);
  }
}

qi::Future<SignalLink> StaticObjectTypeBase::connect(void* instance, AnyObject context, unsigned int event,
                                                       const SignalSubscriber& subscriber)
{
  if (event >= Manageable::startId && event < Manageable::endId)
    instance = static_cast<Manageable*>(context.asGenericObject());
  SignalBase* sb = getSignal(_data, instance, event);
  if (!sb) {
    return qi::makeFutureError<SignalLink>("Cant find signal");
  }
  SignalLink id = sb->connect(subscriber);
  if (id == SignalBase::invalidSignalLink)
    return qi::Future<SignalLink>(id);
  SignalLink link = ((SignalLink)event << 32) + id;
  assert(link >> 32 == event);
  assert((link & 0xFFFFFFFF) == id);
  qiLogDebug() << "Connect " << event <<' ' << id << ' ' << link;
  return qi::Future<SignalLink>(link);
}

qi::Future<void> StaticObjectTypeBase::disconnect(void* instance, AnyObject context, SignalLink linkId)
{
  qiLogDebug() << "Disconnect " << linkId;
  unsigned int event = linkId >> 32;
  unsigned int link = linkId & 0xFFFFFFFF;
  if (event >= Manageable::startId && event < Manageable::endId)
    instance = static_cast<Manageable*>(context.asGenericObject());
  SignalBase* sb = getSignal(_data, instance, event);
  if (!sb)
    return qi::makeFutureError<void>("Cant find signal");
  bool b = sb->disconnect(link);
  if (!b)
    return qi::makeFutureError<void>("Cant unregister signal");
  return qi::Future<void>(0);
}

qi::Future<AnyValue> StaticObjectTypeBase::property(void* instance, unsigned int id)
{
  PropertyBase* p = ::qi::property(_data, instance, id);
  if (!p)
    return qi::makeFutureError<AnyValue>("Cant find event");
  return qi::Future<AnyValue>(p->value());
}

qi::Future<void> StaticObjectTypeBase::setProperty(void* instance, unsigned int id, AnyValue value)
{
  PropertyBase* p = ::qi::property(_data, instance, id);
  if (!p)
    return qi::makeFutureError<void>("Cant find event");
  qiLogDebug() << "SetProperty " << id << " " << encodeJSON(value);
  try
  {
    p->setValue(value.asReference());
  }
  catch(const std::exception& e)
  {
    return qi::makeFutureError<void>(std::string("setProperty: ") + e.what());
  }
  return qi::Future<void>(0);
}



const std::vector<std::pair<TypeInterface*, int> >& StaticObjectTypeBase::parentTypes()
{
  return _data.parentTypes;
}

const TypeInfo& StaticObjectTypeBase::info()
{
  return _data.classType->info();
}

void* StaticObjectTypeBase::initializeStorage(void* ptr)
{
  return _data.classType->initializeStorage(ptr);
}

void* StaticObjectTypeBase::ptrFromStorage(void** ptr)
{
  return _data.classType->ptrFromStorage(ptr);
}

void* StaticObjectTypeBase::clone(void* inst)
{
  return _data.classType->clone(inst);
}

void StaticObjectTypeBase::destroy(void* inst)
{
  _data.classType->destroy(inst);
}

bool StaticObjectTypeBase::less(void* a, void* b)
{
  return a<b;
}

}
