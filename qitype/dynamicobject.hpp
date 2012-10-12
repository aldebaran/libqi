#pragma once
/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/

#ifndef _QIMESSAGING_DYNAMICOBJECT_HPP_
#define _QIMESSAGING_DYNAMICOBJECT_HPP_

#include <qitype/genericobject.hpp>

namespace qi
{

  class DynamicObjectPrivate;

  /** A Dynamic object is an object that handles all signal/method operation
  * itself.
  *
  * Signal handling:
  * The default implementation is creating a SignalBase for each MetaSignal in
  * the MetaObject, and bounces metaEmit(), connect() and disconnect() to it.
  *
  * Method handling:
  * The default implementation holds a method list that the user must populate
  * with setMethod()
  */
  class QITYPE_API DynamicObject: public Manageable {
  public:
    DynamicObject();

    virtual ~DynamicObject();

    /// You *must* call DynamicObject::setMetaObject() if you overload this method.
    virtual void setMetaObject(const MetaObject& mo);


    MetaObject &metaObject();

    void setMethod(unsigned int id, GenericFunction callable);
    GenericFunction method(unsigned int id);
    SignalBase* signalBase(unsigned int id) const;

    virtual qi::Future<GenericValue> metaCall(unsigned int method, const GenericFunctionParameters& params, MetaCallType callType = MetaCallType_Auto);
    virtual void metaEmit(unsigned int event, const GenericFunctionParameters& params);
    /// Calls given functor when event is fired. Takes ownership of functor.
    virtual qi::Future<unsigned int> connect(unsigned int event, const SignalSubscriber& subscriber);
    /// Disconnect an event link. Returns if disconnection was successful.
    virtual qi::Future<void> disconnect(unsigned int linkId);


    boost::shared_ptr<DynamicObjectPrivate> _p;
  };

  //Make an GenericObject of DynamicObject kind from a DynamicObject
  QITYPE_API ObjectPtr     makeDynamicObjectPtr(DynamicObject *obj);

}
#endif  // _QIMESSAGING_DYNAMICOBJECT_HPP_
