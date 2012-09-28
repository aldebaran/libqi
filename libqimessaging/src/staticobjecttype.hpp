#pragma once
/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/

#ifndef _SRC_STATICOBJECTTYPE_HPP_
#define _SRC_STATICOBJECTTYPE_HPP_

#include <qimessaging/api.hpp>
#include <qimessaging/genericvalue.hpp>
#include <qimessaging/type.hpp>
#include <qimessaging/metaobject.hpp>
#include <qimessaging/metafunction.hpp>
#include <qimessaging/genericobject.hpp>

namespace qi
{

  class SignalBase;
//type-erased methods and signals accessors for a given type
struct ObjectTypeData
{
  /* One might want this in the ObjectType virtuals, but that would
   * bypass ObjectType::metaCall which would have to be removed.
   * -> RemoteObject, ALBridge needs to be rewriten.
   */
  typedef boost::function<SignalBase* (void*)> SignalGetter;
  typedef std::map<unsigned int, SignalGetter> SignalGetterMap;
  SignalGetterMap signalGetterMap;

  typedef std::map<unsigned int, GenericMethod> MethodMap;
  MethodMap methodMap;

  boost::function<Manageable* (void*)> asManageable;
  Type* classType;
  std::vector<std::pair<Type*, int> > parentTypes;
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
  virtual const std::type_info& info();
  virtual const MetaObject& metaObject(void* instance);
  virtual qi::Future<MetaFunctionResult> metaCall(void* instance, unsigned int method, const MetaFunctionParameters& params, MetaCallType callType = MetaCallType_Auto);
  virtual void metaEmit(void* instance, unsigned int signal, const MetaFunctionParameters& params);
  virtual unsigned int connect(void* instance, unsigned int event, const SignalSubscriber& subscriber);
  /// Disconnect an event link. Returns if disconnection was successful.
  virtual bool disconnect(void* instance, unsigned int linkId);
  /// @return the manageable interface for this instance, or 0 if not available
  virtual Manageable* manageable(void* instance);
  virtual const std::vector<std::pair<Type*, int> >& parentTypes();
  virtual void* initializeStorage(void*);
  virtual void* ptrFromStorage(void**);
  virtual void* clone(void* inst);
  virtual void destroy(void*);
  virtual void  serialize(ODataStream& s, const void*);
  virtual void* deserialize(IDataStream& s);
  virtual std::string signature();
private:
  MetaObject     _metaObject;
  ObjectTypeData _data;
};

}
#endif  // _SRC_STATICOBJECTTYPE_HPP_
