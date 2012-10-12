#pragma once
/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/

#ifndef _QIMESSAGING_GENERICOBJECT_HPP_
#define _QIMESSAGING_GENERICOBJECT_HPP_

#include <map>
#include <string>
#include <qi/atomic.hpp>
#include <qitype/api.hpp>
#include <qitype/signature.hpp>
#include <qitype/future.hpp>
#include <qitype/metasignal.hpp>
#include <qitype/metamethod.hpp>
#include <qitype/metaobject.hpp>
#include <qi/eventloop.hpp>
#include <qitype/signal.hpp>
#include <qitype/typeobject.hpp>


#include <boost/function_types/function_arity.hpp>
#include <boost/function_types/function_type.hpp>
#include <boost/function_types/result_type.hpp>
#include <boost/function_types/parameter_types.hpp>

namespace qi {

  struct SignalSubscriber;
  class QITYPE_API ObjectInterface {
  public:
    virtual ~ObjectInterface() = 0;
    virtual void onObjectDestroyed(GenericObject *object, void *data) = 0;
  };

  class GenericObject;
  typedef boost::shared_ptr<GenericObject> ObjectPtr;

  /* ObjectValue
  *  static version wrapping class C: Type<C>
  *  dynamic version: Type<DynamicObject>
  *
  * All the methods are convenience wrappers that bounce to the ObjectType
  */
  class QITYPE_API GenericObject
  {
  public:
    GenericObject(ObjectType *type, void *value);
    ~GenericObject();
    const MetaObject &metaObject();
    template <typename RETURN_TYPE> qi::FutureSync<RETURN_TYPE> call(const std::string& methodName,
      qi::AutoGenericValue p1 = qi::AutoGenericValue(),
      qi::AutoGenericValue p2 = qi::AutoGenericValue(),
      qi::AutoGenericValue p3 = qi::AutoGenericValue(),
      qi::AutoGenericValue p4 = qi::AutoGenericValue(),
      qi::AutoGenericValue p5 = qi::AutoGenericValue(),
      qi::AutoGenericValue p6 = qi::AutoGenericValue(),
      qi::AutoGenericValue p7 = qi::AutoGenericValue(),
      qi::AutoGenericValue p8 = qi::AutoGenericValue());

    qi::Future<GenericValue> metaCall(unsigned int method, const GenericFunctionParameters& params, MetaCallType callType = MetaCallType_Auto);
    /// Resolve the method Id and bounces to metaCall
    qi::Future<GenericValue> xMetaCall(const std::string &retsig, const std::string &signature, const GenericFunctionParameters& params);
    void emitEvent(const std::string& eventName,
                   qi::AutoGenericValue p1 = qi::AutoGenericValue(),
                   qi::AutoGenericValue p2 = qi::AutoGenericValue(),
                   qi::AutoGenericValue p3 = qi::AutoGenericValue(),
                   qi::AutoGenericValue p4 = qi::AutoGenericValue(),
                   qi::AutoGenericValue p5 = qi::AutoGenericValue(),
                   qi::AutoGenericValue p6 = qi::AutoGenericValue(),
                   qi::AutoGenericValue p7 = qi::AutoGenericValue(),
                   qi::AutoGenericValue p8 = qi::AutoGenericValue());
    void metaEmit(unsigned int event, const GenericFunctionParameters& params);
    bool xMetaEmit(const std::string &signature, const GenericFunctionParameters &in);
        /** Connect an event to an arbitrary callback.
     *
     * If you are within a service, it is recommended that you connect the
     * event to one of your Slots instead of using this method.
     */
    template <typename FUNCTOR_TYPE>
    qi::FutureSync<unsigned int> connect(const std::string& eventName, FUNCTOR_TYPE callback,
                         EventLoop* ctx = getDefaultObjectEventLoop());


    qi::FutureSync<unsigned int> xConnect(const std::string &signature, const SignalSubscriber& functor);

    /// Calls given functor when event is fired. Takes ownership of functor.
    qi::FutureSync<unsigned int> connect(unsigned int event, const SignalSubscriber& subscriber);

    /** Connect an event to a method.
     * Recommended use is when target is not a proxy.
     * If target is a proxy and this is server-side, the event will be
     *    registered localy and the call will be forwarded.
     * If target and this are proxies, the message will be routed through
     * the current process.
     */
    qi::FutureSync<unsigned int> connect(unsigned int signal, qi::ObjectPtr target, unsigned int slot);

    /// Disconnect an event link. Returns if disconnection was successful.
    qi::FutureSync<void> disconnect(unsigned int linkId);

    void moveToEventLoop(EventLoop* ctx);
    EventLoop* eventLoop();
    //bool isValid() { return type && value;}
    ObjectType*  type;
    void*        value;
  };

  template<typename T>
  GenericValue makeObjectValue(T* ptr);


    /** Event subscriber info.
  *
  * Only one of handler or target must be set.
  */
 struct QITYPE_API SignalSubscriber
 {
   SignalSubscriber()
     : eventLoop(0), target(), method(0), enabled(true), active(0)
   {}

   SignalSubscriber(GenericFunction func, EventLoop* ctx = getDefaultObjectEventLoop())
     : handler(func), eventLoop(ctx), target(), method(0), enabled(true), active(0)
   {}

   SignalSubscriber(qi::ObjectPtr target, unsigned int method)
     : eventLoop(0), target(target), method(method), enabled(true), active(0)
   {}

   void call(const GenericFunctionParameters& args);
   // Source information
   SignalBase*        source;
   /// Uid that can be passed to GenericObject::disconnect()
   SignalBase::Link  linkId;

   // Target information
   //   Mode 1: Direct functor call
   GenericFunction    handler;
   EventLoop*         eventLoop;
   //  Mode 2: metaCall
   ObjectPtr          target;
   unsigned int       method;
   bool               enabled; // call will do nothing if false
   qi::atomic<long>   active;  // true if a call is in progress
 };


  template <typename FUNCTION_TYPE>
  qi::FutureSync<unsigned int> GenericObject::connect(const std::string& eventName,
                                                      FUNCTION_TYPE callback,
                                                      EventLoop* ctx)
  {
    return xConnect(eventName + "::" + detail::FunctionSignature<FUNCTION_TYPE>::signature(),
      SignalSubscriber(makeGenericFunction(callback), ctx));
  }

 QITYPE_API qi::Future<GenericValue> metaCall(EventLoop* el,
    GenericFunction func, const GenericFunctionParameters& params, MetaCallType callType, bool noCloneFirst=false);

};


#include <qitype/details/genericobject.hxx>
#endif  // _QIMESSAGING_GENERICOBJECT_HPP_
