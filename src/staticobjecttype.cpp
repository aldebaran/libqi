/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/
#include "staticobjecttype.hpp"
#include <qitype/signal.hpp>

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
  GenericMethod method = i->second;
  GenericValuePtr self;
  self.type = this;
  self.value = instance;
  GenericFunctionParameters p2;
  p2.reserve(params.size()+1);
  p2.push_back(self);
  p2.insert(p2.end(), params.begin(), params.end());

  return ::qi::metaCall(el, method.toGenericFunction(), p2, callType, true);
}

static SignalBase* getSignal(ObjectTypeData& data, void* instance, unsigned int signal)
{
  ObjectTypeData::SignalGetterMap::iterator i;
  i = data.signalGetterMap.find(signal);
  if (i == data.signalGetterMap.end())
  {
    qiLogError("meta") << "No such signal " << signal;
    return 0;
  }
  SignalBase* sig = i->second(instance);
  if (!sig)
  {
    qiLogError("meta") << "Signal getter returned NULL";
    return 0;
  }
  return sig;
}
void StaticObjectTypeBase::metaPost(void* instance, Manageable* context, unsigned int signal,
                                    const GenericFunctionParameters& params)
{
  SignalBase* sb = getSignal(_data, instance, signal);
  if (!sb)
    return;
  sb->trigger(params);
}

qi::Future<unsigned int> StaticObjectTypeBase::connect(void* instance, Manageable* context, unsigned int event,
                                                       const SignalSubscriber& subscriber)
{
  SignalBase* sb = getSignal(_data, instance, event);
  if (!sb) {
    return qi::makeFutureError<unsigned int>("Cant find signal");
  }
  SignalBase::Link id = sb->connect(subscriber);
  if (id > 0xFFFF)
    qiLogError("meta") << "Signal link id too big";
  return qi::Future<unsigned int>((event << 16) + id);
}

qi::Future<void> StaticObjectTypeBase::disconnect(void* instance, Manageable* context, unsigned int linkId)
{
  unsigned int event = linkId >> 16;
  unsigned int link = linkId & 0xFFFF;
  SignalBase* sb = getSignal(_data, instance, event);
  if (!sb)
    return qi::makeFutureError<void>("Cant find signal");
  bool b = sb->disconnect(link);
  if (!b)
    return qi::makeFutureError<void>("Cant unregister signal");
  return qi::Future<void>(0);
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

}
