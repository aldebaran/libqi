#pragma once
/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/

#ifndef _SRC_STATICOBJECTTYPE_HPP_
#define _SRC_STATICOBJECTTYPE_HPP_

#include <qitype/api.hpp>
#include <qitype/property.hpp>
#include <qitype/genericvalue.hpp>
#include <qitype/type.hpp>
#include <qitype/metaobject.hpp>
#include <qitype/genericobject.hpp>

namespace qi
{

  class SignalBase;
//type-erased methods and signals accessors for a given type
struct ObjectTypeData
{
  ObjectTypeData()
  : classType(0)
  , threadingModel(ObjectThreadingModel_SingleThread)
  {}

  /* One might want this in the ObjectType virtuals, but that would
   * bypass ObjectType::metaCall which would have to be removed.
   * -> RemoteObject, ALBridge needs to be rewriten.
   */
  typedef boost::function<SignalBase* (void*)> SignalGetter;
  typedef std::map<unsigned int, SignalGetter> SignalGetterMap;
  SignalGetterMap signalGetterMap;

  typedef boost::function<PropertyBase*(void*)> PropertyGetter;
  typedef std::map<unsigned int, PropertyGetter> PropertyGetterMap;
  PropertyGetterMap propertyGetterMap;

  typedef std::map<
    unsigned int,
    std::pair<GenericFunction, MetaCallType>
  > MethodMap;

  MethodMap methodMap;

  Type* classType;
  std::vector<std::pair<Type*, int> > parentTypes;
  ObjectThreadingModel threadingModel;
};


/** One instance of this class represents one C++ class
 *
 * To avoid exposing this class, which means not being templated by the class
 * type, we just store the two things we need in an erased way:
 * - Manageable accessor
 * - typeinfo
 */
class StaticObjectTypeBase: public ObjectType
{
public:
  void initialize(const MetaObject& mo, const ObjectTypeData& data);
  virtual const TypeInfo& info();
  virtual const MetaObject& metaObject(void* instance);
  virtual qi::Future<GenericValuePtr> metaCall(void* instance, Manageable* context, unsigned int method, const GenericFunctionParameters& params, MetaCallType callType = MetaCallType_Auto);
  virtual void metaPost(void* instance, Manageable* context, unsigned int signal, const GenericFunctionParameters& params);
  virtual qi::Future<Link> connect(void* instance, Manageable* context, unsigned int event, const SignalSubscriber& subscriber);
  /// Disconnect an event link. Returns if disconnection was successful.
  virtual qi::Future<void> disconnect(void* instance, Manageable* context, Link linkId);
  virtual qi::Future<GenericValue> property(void* instance, unsigned int id);
  virtual qi::Future<void> setProperty(void* instance, unsigned int id, GenericValue value);

  virtual const std::vector<std::pair<Type*, int> >& parentTypes();
  virtual void* initializeStorage(void*);
  virtual void* ptrFromStorage(void**);
  virtual void* clone(void* inst);
  virtual void destroy(void*);
  virtual bool less(void* a, void* b);
private:
  MetaObject     _metaObject;
  ObjectTypeData _data;
};

}
#endif  // _SRC_STATICOBJECTTYPE_HPP_
