#pragma once
/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/

#ifndef _QITYPE_GENERICOBJECT_HPP_
#define _QITYPE_GENERICOBJECT_HPP_

#include <map>
#include <string>
#include <qi/atomic.hpp>
#include <qitype/api.hpp>
#include <qitype/manageable.hpp>
#include <qi/future.hpp>
#include <qitype/metasignal.hpp>
#include <qitype/metamethod.hpp>
#include <qitype/metaobject.hpp>
#include <qitype/functiontypefactory.hpp>
#include <qitype/signal.hpp>
#include <qi/eventloop.hpp>
#include <qitype/typeobject.hpp>

#ifdef _MSC_VER
#  pragma warning( push )
#  pragma warning( disable: 4251 )
#  pragma warning( disable: 4231 )
#endif

namespace qi {

  /* We need shared typeid on Future<GenericValuePtr>
   * If we do not export, typeids do not compare equals under some gcc-macos
   * Furthermore we get:
   * - macos: compiler warning and incorrect code if the template implementation is
   *   used before the extern declaration
   * - macos: compiler error if the extern is seen before a non-extern forced
   *  instanciation
   * - linux: linker error: if the symbol is marked hidden in some .o files,
   * and not hidden in others, hidden has precedence, and the extern prevents
   * the usage of the inlined code version.
   * - win32: if the whole template is exported, then no new instanciations
   * besides the one in the defining module can be created.
   */
#if !defined(qitype_EXPORTS) && !defined(__linux__)
  // CS4231 - 'identifier' before template explicit instantiation
  extern template class Future<GenericValuePtr>;
  // CS4231 - 'identifier' before template explicit instantiation
  extern template class Future<void>;
#endif
  class SignalSubscriber;
  class SignalBase;

  class GenericObject;
  typedef boost::shared_ptr<GenericObject> ObjectPtr;
  typedef boost::weak_ptr<GenericObject>   ObjectWeakPtr;

  /* ObjectValue
  *  static version wrapping class C: Type<C>
  *  dynamic version: Type<DynamicObject>
  *
  * All the methods are convenience wrappers that bounce to the ObjectType,
  * except Event Loop management
  * This class has pointer semantic. Do not use directly, use ObjectPtr,
  * obtained through Session, DynamicObjectBuilder or ObjectTypeBuilder.
  */
  class QITYPE_API GenericObject: public Manageable
  {
  public:
    GenericObject(ObjectType *type, void *value);
    ~GenericObject();
    const MetaObject &metaObject();
    template <typename RETURN_TYPE> qi::FutureSync<RETURN_TYPE> call(const std::string& methodName,
      qi::AutoGenericValuePtr p1 = qi::AutoGenericValuePtr(),
      qi::AutoGenericValuePtr p2 = qi::AutoGenericValuePtr(),
      qi::AutoGenericValuePtr p3 = qi::AutoGenericValuePtr(),
      qi::AutoGenericValuePtr p4 = qi::AutoGenericValuePtr(),
      qi::AutoGenericValuePtr p5 = qi::AutoGenericValuePtr(),
      qi::AutoGenericValuePtr p6 = qi::AutoGenericValuePtr(),
      qi::AutoGenericValuePtr p7 = qi::AutoGenericValuePtr(),
      qi::AutoGenericValuePtr p8 = qi::AutoGenericValuePtr());

    qi::Future<GenericValuePtr> metaCall(unsigned int method, const GenericFunctionParameters& params, MetaCallType callType = MetaCallType_Auto);
    /// Resolve the method Id and bounces to metaCall
    qi::Future<GenericValuePtr> metaCall(const std::string &signature, const GenericFunctionParameters& params, MetaCallType callType = MetaCallType_Auto);

    void emitEvent(const std::string& eventName,
                   qi::AutoGenericValuePtr p1 = qi::AutoGenericValuePtr(),
                   qi::AutoGenericValuePtr p2 = qi::AutoGenericValuePtr(),
                   qi::AutoGenericValuePtr p3 = qi::AutoGenericValuePtr(),
                   qi::AutoGenericValuePtr p4 = qi::AutoGenericValuePtr(),
                   qi::AutoGenericValuePtr p5 = qi::AutoGenericValuePtr(),
                   qi::AutoGenericValuePtr p6 = qi::AutoGenericValuePtr(),
                   qi::AutoGenericValuePtr p7 = qi::AutoGenericValuePtr(),
                   qi::AutoGenericValuePtr p8 = qi::AutoGenericValuePtr());

    void metaPost(unsigned int event, const GenericFunctionParameters& params);

    //protected
    bool xMetaPost(const std::string &signature, const GenericFunctionParameters &in);

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

    virtual EventLoop* eventLoop() const;
    virtual void moveToEventLoop(EventLoop* eventLoop);

    //bool isValid() { return type && value;}
    ObjectType*  type;
    void*        value;
  };

   // C4251
  template <typename FUNCTION_TYPE>
  qi::FutureSync<unsigned int> GenericObject::connect(const std::string& eventName,
                                                      FUNCTION_TYPE callback,
                                                      EventLoop* ctx)
  {
    return xConnect(eventName + "::" + detail::FunctionSignature<FUNCTION_TYPE>::signature(),
      SignalSubscriber(makeGenericFunction(callback), ctx));
  }


 QITYPE_API qi::Future<GenericValuePtr> metaCall(EventLoop* el,
    GenericFunction func, const GenericFunctionParameters& params, MetaCallType callType, bool noCloneFirst=false);

}

#include <qitype/details/genericobject.hxx>

#ifdef _MSC_VER
#  pragma warning( pop )
#endif

#endif  // _QITYPE_GENERICOBJECT_HPP_
