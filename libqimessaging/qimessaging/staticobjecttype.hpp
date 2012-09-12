/*

** Copyright (C) 2012 Aldebaran Robotics
*/

#ifndef _QIMESSAGING_STATICOBJECTTYPE_HPP_
#define _QIMESSAGING_STATICOBJECTTYPE_HPP_

#include <qimessaging/api.hpp>
#include <qimessaging/value.hpp>
#include <qimessaging/type.hpp>
#include <qimessaging/metaobject.hpp>
#include <qimessaging/metafunction.hpp>
#include <qimessaging/object.hpp>

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

  typedef std::map<unsigned int, MethodValue> MethodMap;
  MethodMap methodMap;
};


/** One instance of this class represents one C++ class
 *
 */
class StaticObjectTypeBase: public virtual ObjectType
{
public:
  void initialize(const MetaObject& mo, const ObjectTypeData& data);
  virtual const MetaObject& metaObject(void* instance);
  virtual qi::Future<MetaFunctionResult> metaCall(void* instance, unsigned int method, const MetaFunctionParameters& params, MetaCallType callType = MetaCallType_Auto);
  virtual void metaEmit(void* instance, unsigned int signal, const MetaFunctionParameters& params);
  virtual unsigned int connect(void* instance, unsigned int event, const SignalSubscriber& subscriber);
  /// Disconnect an event link. Returns if disconnection was successful.
  virtual bool disconnect(void* instance, unsigned int linkId);
  /// @return the manageable interface for this instance, or 0 if not available
  virtual Manageable* manageable(void* instance);

private:
  MetaObject     _metaObject;
  ObjectTypeData _data;
};

template<typename T>
class StaticManageableObjectTypeBase: public virtual StaticObjectTypeBase
{
public:
  virtual Manageable* manageable(void* instance)
  {
    return reinterpret_cast<T*>(instance);
  }
};

// FIXME: un-template by reimplementing Type and bouncing, so that this can
// go  in a private header.
template<typename T>
class StaticObject
:public virtual boost::mpl::if_<
         boost::is_base_of<Manageable, T>,
         StaticManageableObjectTypeBase<T>,
         StaticObjectTypeBase>::type
, public virtual TypeImpl<T*>
{};


}
#endif
