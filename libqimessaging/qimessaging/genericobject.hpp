#pragma once
/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/

#ifndef _QIMESSAGING_GENERICOBJECT_HPP_
#define _QIMESSAGING_GENERICOBJECT_HPP_

#include <map>
#include <string>
#include <qimessaging/api.hpp>
#include <qimessaging/genericvalue.hpp>
#include <qimessaging/signature.hpp>
#include <qimessaging/future.hpp>
#include <qimessaging/metasignal.hpp>
#include <qimessaging/metamethod.hpp>
#include <qimessaging/metaobject.hpp>
#include <qimessaging/eventloop.hpp>
#include <qimessaging/signal.hpp>


#include <boost/function_types/function_arity.hpp>
#include <boost/function_types/function_type.hpp>
#include <boost/function_types/result_type.hpp>
#include <boost/function_types/parameter_types.hpp>

namespace qi {

  struct SignalSubscriber;
  class QIMESSAGING_API ObjectInterface {
  public:
    virtual ~ObjectInterface() = 0;
    virtual void onObjectDestroyed(GenericObject *object, void *data) = 0;
  };

  class ManageablePrivate;

 /** User classes can inherit from Manageable to benefit from additional features:
  * - Automatic signal disconnection when the object is deleted
  * - Event loop management
  */
 class QIMESSAGING_API Manageable
 {
 public:
   Manageable();
   ~Manageable();
   Manageable(const Manageable& b);
   void operator = (const Manageable& b);

   void addCallbacks(ObjectInterface *callbacks, void *data = 0);
   void removeCallbacks(ObjectInterface *callbacks);

   // Remember than this is the target of subscriber
   void addRegistration(const SignalSubscriber& subscriber);
   // Notify that a registered subscriber got disconnected
   void removeRegistration(unsigned int linkId);

   EventLoop* eventLoop() const;
   void moveToEventLoop(EventLoop* eventLoop);

   ManageablePrivate* _p;
 };

 enum MetaCallType {
   MetaCallType_Auto   = 0,
   MetaCallType_Direct = 1,
   MetaCallType_Queued = 2,
 };

 /* We will have 3 implementations for 3 classes of C++ class:
 * - DynamicObject
 * - T where T inherits from Manageable
 * - T
 * If the class does not inherit from Manageable, some features will be lost:
 *   - Automatic signal disconnection on object destruction
 *   - EventLoop support. emit will use the default policy
 *
 *
 *  NOTE: no SignalBase accessor at this point, but the backend is such that it would be possible
 *   but if we do that, virtual emit/connect/disconnect must go away, as they could be bypassed
 *  ->RemoteObject, ALBridge will have to adapt
 *
 */

  class QIMESSAGING_API ObjectType: public Type
  {
  public:
    virtual const MetaObject& metaObject(void* instance) = 0;
    virtual qi::Future<GenericValue> metaCall(void* instance, unsigned int method, const GenericFunctionParameters& params, MetaCallType callType = MetaCallType_Auto)=0;
    virtual void metaEmit(void* instance, unsigned int signal, const GenericFunctionParameters& params)=0;
    virtual unsigned int connect(void* instance, unsigned int event, const SignalSubscriber& subscriber)=0;
    /// Disconnect an event link. Returns if disconnection was successful.
    virtual bool disconnect(void* instance, unsigned int linkId)=0;
    /// @return the manageable interface for this instance, or 0 if not available
    virtual Manageable* manageable(void* instance) = 0;
    /// @return parent types with associated poniter offset
    virtual const std::vector<std::pair<Type*, int> >& parentTypes() = 0;
    virtual Type::Kind kind() const { return Type::Object;}
    /// @return -1 if there is no inheritance, or the pointer offset
    int inherits(Type* other);
  };

  class GenericObject;
  typedef boost::shared_ptr<GenericObject> ObjectPtr;

  /* ObjectValue
  *  static version wrapping class C: Type<C>
  *  dynamic version: Type<DynamicObject>
  *
  * All the methods are convenience wrappers that bounce to the ObjectType
  */
  class QIMESSAGING_API GenericObject
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
    unsigned int connect(const std::string& eventName, FUNCTOR_TYPE callback,
                         EventLoop* ctx = getDefaultObjectEventLoop());


    unsigned int xConnect(const std::string &signature, const SignalSubscriber& functor);

    /// Calls given functor when event is fired. Takes ownership of functor.
    unsigned int connect(unsigned int event, const SignalSubscriber& subscriber);

    /// Disconnect an event link. Returns if disconnection was successful.
    bool disconnect(unsigned int linkId);
    /** Connect an event to a method.
     * Recommended use is when target is not a proxy.
     * If target is a proxy and this is server-side, the event will be
     *    registered localy and the call will be forwarded.
     * If target and this are proxies, the message will be routed through
     * the current process.
     */
    unsigned int connect(unsigned int signal, qi::ObjectPtr target, unsigned int slot);

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
 struct QIMESSAGING_API SignalSubscriber
 {
   SignalSubscriber()
     : eventLoop(0), target(), method(0)
   {}

   SignalSubscriber(GenericFunction func, EventLoop* ctx = getDefaultObjectEventLoop())
     : handler(func), eventLoop(ctx), target(), method(0)
   {}

   SignalSubscriber(qi::ObjectPtr target, unsigned int method)
     : eventLoop(0), target(target), method(method)
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
 };


  template <typename FUNCTION_TYPE>
  unsigned int GenericObject::connect(const std::string& eventName,
                               FUNCTION_TYPE callback,
                               EventLoop* ctx)
  {
    return xConnect(eventName + "::" + detail::FunctionSignature<FUNCTION_TYPE>::signature(),
      SignalSubscriber(makeGenericFunction(callback), ctx));
  }

 QIMESSAGING_API qi::Future<GenericValue> metaCall(EventLoop* el,
    GenericFunction func, const GenericFunctionParameters& params, MetaCallType callType, bool noCloneFirst=false);

};


#include <qimessaging/details/genericobject.hxx>
#endif  // _QIMESSAGING_GENERICOBJECT_HPP_
