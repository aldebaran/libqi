#pragma once
/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/

#ifndef _QI_TYPE_DETAIL_STATICOBJECTTYPE_HPP_
#define _QI_TYPE_DETAIL_STATICOBJECTTYPE_HPP_

#include <qi/api.hpp>
#include <qi/property.hpp>
#include <qi/anyvalue.hpp>
#include <qi/type/typeinterface.hpp>
#include <qi/type/metaobject.hpp>

namespace qi
{

class SignalBase;

namespace detail {

//type-erased methods and signals accessors for a given type
struct QI_API ObjectTypeData
{
  ObjectTypeData()
  : classType(0)
  , threadingModel(ObjectThreadingModel_Default)
  {}

  /* One might want this in the ObjectType virtuals, but that would
   * bypass ObjectTypeInterface::metaCall which would have to be removed.
   * -> RemoteObject, ALBridge needs to be rewriten.
   */
  using SignalGetter = boost::function<SignalBase* (void*)>;
  using SignalGetterMap = std::map<unsigned int, SignalGetter>;
  SignalGetterMap signalGetterMap;

  using PropertyGetter = boost::function<PropertyBase*(void*)>;
  using PropertyGetterMap = std::map<unsigned int, PropertyGetter>;
  PropertyGetterMap propertyGetterMap;

  using MethodMap = std::map<unsigned int, std::pair<AnyFunction, MetaCallType>>;

  MethodMap methodMap;

  TypeInterface* classType;
  std::vector<std::pair<TypeInterface*, int> > parentTypes;
  ObjectThreadingModel threadingModel;
  qi::AnyFunction strandAccessor;
};


/** One instance of this class represents one C++ class
 *
 * To avoid exposing this class, which means not being templated by the class
 * type, we just store the two things we need in an erased way:
 * - Manageable accessor
 * - typeinfo
 */
class QI_API StaticObjectTypeBase: public ObjectTypeInterface
{
public:
  void initialize(const MetaObject& mo, const ObjectTypeData& data);
  virtual const TypeInfo& info();
  virtual const MetaObject& metaObject(void* instance);
  virtual qi::Future<AnyReference> metaCall(void* instance, AnyObject context, unsigned int method, const GenericFunctionParameters& params, MetaCallType callType, Signature returnSignature);
  virtual void metaPost(void* instance, AnyObject context, unsigned int signal, const GenericFunctionParameters& params);
  virtual qi::Future<SignalLink> connect(void* instance, AnyObject context, unsigned int event, const SignalSubscriber& subscriber);
  /// Disconnect an event link. Returns if disconnection was successful.
  virtual qi::Future<void> disconnect(void* instance, AnyObject context, SignalLink linkId);
  virtual qi::Future<AnyValue> property(void* instance, AnyObject context, unsigned int id);
  virtual qi::Future<void> setProperty(void* instance, AnyObject context, unsigned int id, AnyValue value);

  virtual const std::vector<std::pair<TypeInterface*, int> >& parentTypes();
  virtual void* initializeStorage(void*);
  virtual void* ptrFromStorage(void**);
  virtual void* clone(void* inst);
  virtual void destroy(void*);
  virtual bool less(void* a, void* b);
private:
  MetaObject     _metaObject;
  ObjectTypeData _data;

  ExecutionContext* getExecutionContext(void* instance, qi::AnyObject context, MetaCallType methodThreadingModel = MetaCallType_Auto);
};

}

}

#endif
