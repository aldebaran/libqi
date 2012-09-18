/*
**
** Copyright (C) 2012 Aldebaran Robotics
*/

#include <qimessaging/staticobjecttype.hpp>
#include <qimessaging/signal.hpp>

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

Manageable* StaticObjectTypeBase::manageable(void*)
{
  qiLogDebug("qi.staticobject") << "Object not manageable";
  return 0;
}

qi::Future<MetaFunctionResult>
StaticObjectTypeBase::metaCall(void* instance, unsigned int methodId,
                               const MetaFunctionParameters& params,
                               MetaCallType callType)
{
  qi::Promise<MetaFunctionResult> out;
  ObjectTypeData::MethodMap::iterator i;
  i = _data.methodMap.find(methodId);
  if (i == _data.methodMap.end())
  {
    out.setError("No such method");
    return out.future();
  }

  Manageable* m = manageable(instance);
  EventLoop* el = m?m->eventLoop():0;
  GenericMethod method = i->second;
  GenericValue self;
  self.type = this;
  self.value = instance;
  return ::qi::metaCall(el, (MetaCallable)boost::bind(callMethod, method, self, _1),
    params, callType);
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
void StaticObjectTypeBase::metaEmit(void* instance, unsigned int signal,
                                    const MetaFunctionParameters& params)
{
  SignalBase* sb = getSignal(_data, instance, signal);
  if (!sb)
    return;
  sb->trigger(params);
}

unsigned int StaticObjectTypeBase::connect(void* instance, unsigned int event,
                                           const SignalSubscriber& subscriber)
{
  SignalBase* sb = getSignal(_data, instance, event);
  if (!sb)
    return -1;
  SignalBase::Link id = sb->connect(subscriber);
  if (id > 0xFFFF)
    qiLogError("meta") << "Signal link id too big";
  return (event << 16) + id;
}

bool StaticObjectTypeBase::disconnect(void* instance, unsigned int linkId)
{
  unsigned int event = linkId >> 16;
  unsigned int link = linkId & 0xFFFF;
  SignalBase* sb = getSignal(_data, instance, event);
  if (!sb)
    return false;
  return sb->disconnect(link);
}

}
