/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/
#include "staticobjecttype.hpp"
#include <qitype/signal.hpp>

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


qi::Future<GenericValuePtr>
StaticObjectTypeBase::metaCall(void* instance, Manageable* context, unsigned int methodId,
                               const GenericFunctionParameters& params,
                               MetaCallType callType)
{
  qi::Promise<GenericValuePtr> out;
  ObjectTypeData::MethodMap::iterator i;
  i = _data.methodMap.find(methodId);
  if (i == _data.methodMap.end())
  {
    out.setError("No such method");
    return out.future();
  }

  EventLoop* el = context->eventLoop();
  MetaCallType methodThreadingModel = i->second.second;

  GenericFunction method = i->second.first;
  GenericValuePtr self;
  if (methodId >= Manageable::startId  && methodId < Manageable::endId)
  {
    self.type = qi::typeOf<Manageable>();
    self.value = context;
  }
  else
  {
    self.type = this;
    self.value = instance;
  }
  GenericFunctionParameters p2;
  p2.reserve(params.size()+1);
  p2.push_back(self);
  p2.insert(p2.end(), params.begin(), params.end());

  return ::qi::metaCall(el, _data.threadingModel, methodThreadingModel, callType, context, methodId, method, p2, true);
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

void StaticObjectTypeBase::metaPost(void* instance, Manageable* context, unsigned int signal,
                                    const GenericFunctionParameters& params)
{
  SignalBase* sb = getSignal(_data, instance, signal);
  if (sb)
  {
    sb->trigger(params);
  }
  else
  { // try method
    metaCall(instance, context, signal, params, MetaCallType_Queued);
  }
}

qi::Future<Link> StaticObjectTypeBase::connect(void* instance, Manageable* context, unsigned int event,
                                                       const SignalSubscriber& subscriber)
{
  SignalBase* sb = getSignal(_data, instance, event);
  if (!sb) {
    return qi::makeFutureError<Link>("Cant find signal");
  }
  SignalBase::Link id = sb->connect(subscriber);
  Link link = ((Link)event << 32) + id;
  assert(link >> 32 == event);
  assert((link & 0xFFFFFFFF) == id);
  qiLogDebug() << "Connect " << event <<' ' << id << ' ' << link;
  return qi::Future<Link>(link);
}

qi::Future<void> StaticObjectTypeBase::disconnect(void* instance, Manageable* context, Link linkId)
{
  qiLogDebug() << "Disconnect " << linkId;
  unsigned int event = linkId >> 32;
  unsigned int link = linkId & 0xFFFFFFFF;
  SignalBase* sb = getSignal(_data, instance, event);
  if (!sb)
    return qi::makeFutureError<void>("Cant find signal");
  bool b = sb->disconnect(link);
  if (!b)
    return qi::makeFutureError<void>("Cant unregister signal");
  return qi::Future<void>(0);
}

qi::Future<GenericValue> StaticObjectTypeBase::property(void* instance, unsigned int id)
{
  PropertyBase* p = ::qi::property(_data, instance, id);
  if (!p)
    return qi::makeFutureError<GenericValue>("Cant find event");
  qi::Promise<GenericValue> res;
  res.setValue(p->value());
  return res.future();
}

qi::Future<void> StaticObjectTypeBase::setProperty(void* instance, unsigned int id, GenericValue value)
{
  PropertyBase* p = ::qi::property(_data, instance, id);
  if (!p)
    return qi::makeFutureError<void>("Cant find event");
  qiLogDebug() << "SetProperty " << id << " " << encodeJSON(value);
  try
  {
    p->setValue(value);
  }
  catch(const std::exception& e)
  {
    return qi::makeFutureError<void>(std::string("setProperty: ") + e.what());
  }
  qi::Promise<void> res;
  res.setValue(0);
  return res.future();
}



const std::vector<std::pair<Type*, int> >& StaticObjectTypeBase::parentTypes()
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
