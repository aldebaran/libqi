#pragma once
/*
**  Copyright (C) 2013 Aldebaran Robotics
**  See COPYING for the license
*/

#ifndef _QI_TYPE_DYNAMICOBJECT_HPP_
#define _QI_TYPE_DYNAMICOBJECT_HPP_

#include <qi/anyobject.hpp>
#include <qi/property.hpp>
#include <qi/objectuid.hpp>
#include <boost/optional.hpp>

#ifdef _MSC_VER
#  pragma warning( push )
#  pragma warning( disable: 4251 )
#endif

namespace qi
{

  class DynamicObjectPrivate;

  /** A Dynamic object is an object that handles all signal/method operation
  * itself.
  *
  * Signal handling:
  * The default implementation is creating a SignalBase for each MetaSignal in
  * the MetaObject, and bounces metaPost(), connect() and disconnect() to it.
  *
  * Method handling:
  * The default implementation holds a method list that the user must populate
  * with setMethod()
  */
  class QI_API DynamicObject {
  public:
    DynamicObject();

    virtual ~DynamicObject();

    /// You *must* call DynamicObject::setMetaObject() if you overload this method.
    virtual void setMetaObject(const MetaObject& mo);


    MetaObject &metaObject();

    void setMethod(unsigned int id, AnyFunction callable, MetaCallType threadingModel = MetaCallType_Auto);
    void setSignal(unsigned int id, SignalBase* signal);
    void setProperty(unsigned int id, PropertyBase* property);

    const AnyFunction&   method(unsigned int id) const;
    SignalBase*   signal(unsigned int id) const;
    PropertyBase* property(unsigned int) const;

    boost::optional<ObjectUid> uid() const;
    void setUid(boost::optional<ObjectUid> newUid);

    virtual qi::Future<AnyReference> metaCall(AnyObject context, unsigned int method, const GenericFunctionParameters& params, MetaCallType callType = MetaCallType_Auto, Signature returnSignature=Signature());
    virtual void metaPost(AnyObject context, unsigned int event, const GenericFunctionParameters& params);
    /// Calls given functor when event is fired. Takes ownership of functor.
    virtual qi::Future<SignalLink> metaConnect(unsigned int event, const SignalSubscriber& subscriber);
    /// Disconnect an event link. Returns if disconnection was successful.
    virtual qi::Future<void> metaDisconnect(SignalLink linkId);
    virtual qi::Future<AnyValue> metaProperty(AnyObject context, unsigned int id);
    virtual qi::Future<void> metaSetProperty(AnyObject context, unsigned int id, AnyValue val);

    void setThreadingModel(ObjectThreadingModel model);
    ObjectThreadingModel threadingModel() const;
    // internal use, call once to update with Manageable methods and signals
    void setManageable(Manageable* m);
    // C4251
    boost::shared_ptr<DynamicObjectPrivate> _p;
  };

  /// Make an AnyObject of DynamicObject kind from a DynamicObject
  QI_API AnyObject     makeDynamicAnyObject(DynamicObject *obj,
    bool destroyObject = true,
    boost::optional<ObjectUid> uid = {},
    boost::function<void (GenericObject*)> onDelete = boost::function<void (GenericObject*)>());

  /// \deprecated This function is considered harmful. Use `makeDynamicAnyObject` instead and bind
  /// the lifetime of the `shared_ptr` to the deleter.
  //
  // WARNING: This function leaks and is harmful but seems difficult to fix right now. So we've
  // decided to deprecate it until it can get completely removed.
  QI_API_DEPRECATED_MSG(
    "This function is considered harmful. Use `makeDynamicAnyObject` instead and bind "
    "the lifetime of the `shared_ptr` to the deleter.")
  QI_API AnyObject makeDynamicSharedAnyObjectImpl(DynamicObject* obj,
                                                  boost::shared_ptr<Empty> other);

  /// Make an AnyObject that shares its ref counter with \p other.
  /// Note that \p obj will not be destroyed when the shared counter reaches 0.
  ///
  /// \deprecated This function is considered harmful. Use `makeDynamicAnyObject` instead and bind
  /// the lifetime of the `shared_ptr` to the deleter.
  template<typename T>
  QI_API_DEPRECATED_MSG(
    "This function is considered harmful. Use `makeDynamicAnyObject` instead and bind "
    "the lifetime of the `shared_ptr` to the deleter.")
  inline AnyObject makeDynamicSharedAnyObject(DynamicObject *obj, boost::shared_ptr<T> other)
  {
QI_WARNING_PUSH()
QI_WARNING_DISABLE(4996, deprecated-declarations)
    return makeDynamicSharedAnyObjectImpl(obj, boost::shared_ptr<Empty>(other, 0));
QI_WARNING_POP()
  }


  QI_API ObjectTypeInterface* getDynamicTypeInterface();
}

#ifdef _MSC_VER
#  pragma warning( pop )
#endif

#endif  // _QITYPE_DYNAMICOBJECT_HPP_
