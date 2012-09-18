/*
** Copyright (C) 2012 Aldebaran Robotics
*/

#ifndef _QIMESSAGING_DYNAMICOBJECT_HPP_
#define _QIMESSAGING_DYNAMICOBJECT_HPP_

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
  class QIMESSAGING_API DynamicObject: public Manageable {
  public:
    DynamicObject();

    virtual ~DynamicObject();

    /// You *must* call DynamicObject::setMetaObject() if you overload this method.
    virtual void setMetaObject(const MetaObject& mo);


    MetaObject &metaObject();

    void setMethod(unsigned int id, MetaCallable callable);
    SignalBase* getSignal(unsigned int id);

    virtual qi::Future<MetaFunctionResult> metaCall(unsigned int method, const MetaFunctionParameters& params, MetaCallType callType = MetaCallType_Auto);
    virtual void metaEmit(unsigned int event, const MetaFunctionParameters& params);
    /// Calls given functor when event is fired. Takes ownership of functor.
    virtual unsigned int connect(unsigned int event, const SignalSubscriber& subscriber);
    /// Disconnect an event link. Returns if disconnection was successful.
    virtual bool disconnect(unsigned int linkId);


    boost::shared_ptr<DynamicObjectPrivate> _p;
  };

  //Make an GenericObject of DynamicObject kind from a DynamicObject
  QIMESSAGING_API GenericObject makeDynamicObject(DynamicObject* obj);

}
#endif
