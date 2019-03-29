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
  std::vector<std::pair<TypeInterface*, std::ptrdiff_t> > parentTypes;
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
  const TypeInfo& info() override;
  const MetaObject& metaObject(void* instance) override;
  ObjectUid uid(void* instance) const override;
  qi::Future<AnyReference> metaCall(void* instance, AnyObject context, unsigned int method, const GenericFunctionParameters& params, MetaCallType callType, Signature returnSignature) override;
  void metaPost(void* instance, AnyObject context, unsigned int signal, const GenericFunctionParameters& params) override;
  qi::Future<SignalLink> connect(void* instance, AnyObject context, unsigned int event, const SignalSubscriber& subscriber) override;
  /// Disconnect an event link. Returns if disconnection was successful.
  qi::Future<void> disconnect(void* instance, AnyObject context, SignalLink linkId) override;
  qi::Future<AnyValue> property(void* instance, AnyObject context, unsigned int id) override;
  qi::Future<void> setProperty(void* instance, AnyObject context, unsigned int id, AnyValue value) override;

  const std::vector<std::pair<TypeInterface*, std::ptrdiff_t> >& parentTypes() override;
  void* initializeStorage(void*) override;
  void* ptrFromStorage(void**) override;
  void* clone(void* inst) override;
  void destroy(void*) override;
  bool less(void* a, void* b) override;
private:
  MetaObject     _metaObject;
  ObjectTypeData _data;

  ExecutionContext* getExecutionContext(void* instance, qi::AnyObject context, MetaCallType methodThreadingModel = MetaCallType_Auto);
};

}

}

#endif
