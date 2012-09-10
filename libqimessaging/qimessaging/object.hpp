/*
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2012 Aldebaran Robotics
*/

#pragma once
#ifndef _QIMESSAGING_OBJECT_HPP_
#define _QIMESSAGING_OBJECT_HPP_

#include <map>
#include <string>
#include <qimessaging/api.hpp>
#include <qimessaging/metavalue.hpp>
#include <qimessaging/metafunction.hpp>
#include <qimessaging/signature.hpp>
#include <qimessaging/future.hpp>
#include <qimessaging/metasignal.hpp>
#include <qimessaging/metamethod.hpp>
#include <qimessaging/metaobject.hpp>
#include <qimessaging/event_loop.hpp>
#include <qimessaging/objectbuilder.hpp>
#include <qimessaging/signal.hpp>


#include <boost/function_types/function_arity.hpp>
#include <boost/function_types/function_type.hpp>
#include <boost/function_types/result_type.hpp>
#include <boost/function_types/parameter_types.hpp>

namespace qi {

  class QIMESSAGING_API ObjectInterface {
  public:
    virtual ~ObjectInterface() = 0;
    virtual void onObjectDestroyed(Object *object, void *data) = 0;
  };

  class ObjectPrivate;
  class QIMESSAGING_API Object {
  public:
    Object();
    Object(qi::MetaObject metaobject);
    virtual ~Object();

    enum MetaCallType {
      MetaCallType_Auto   = 0,
      MetaCallType_Direct = 1,
      MetaCallType_Queued = 2,
    };

    bool isValid() const;

    void addCallbacks(ObjectInterface *callbacks, void *data = 0);
    void removeCallbacks(ObjectInterface *callbacks);

    MetaObject &metaObject();

    template <typename RETURN_TYPE> qi::FutureSync<RETURN_TYPE> call(const std::string& methodName,
                                                                 qi::AutoMetaValue p1 = qi::AutoMetaValue(),
                                                                 qi::AutoMetaValue p2 = qi::AutoMetaValue(),
                                                                 qi::AutoMetaValue p3 = qi::AutoMetaValue(),
                                                                 qi::AutoMetaValue p4 = qi::AutoMetaValue(),
                                                                 qi::AutoMetaValue p5 = qi::AutoMetaValue(),
                                                                 qi::AutoMetaValue p6 = qi::AutoMetaValue(),
                                                                 qi::AutoMetaValue p7 = qi::AutoMetaValue(),
                                                                 qi::AutoMetaValue p8 = qi::AutoMetaValue());

    virtual qi::Future<MetaFunctionResult> metaCall(unsigned int method, const MetaFunctionParameters& params, MetaCallType callType = MetaCallType_Auto);
    /// Resolve the method Id and bounces to metaCall
    qi::Future<MetaFunctionResult> xMetaCall(const std::string &retsig, const std::string &signature, const MetaFunctionParameters& params);


    void emitEvent(const std::string& eventName,
                   qi::AutoMetaValue p1 = qi::AutoMetaValue(),
                   qi::AutoMetaValue p2 = qi::AutoMetaValue(),
                   qi::AutoMetaValue p3 = qi::AutoMetaValue(),
                   qi::AutoMetaValue p4 = qi::AutoMetaValue(),
                   qi::AutoMetaValue p5 = qi::AutoMetaValue(),
                   qi::AutoMetaValue p6 = qi::AutoMetaValue(),
                   qi::AutoMetaValue p7 = qi::AutoMetaValue(),
                   qi::AutoMetaValue p8 = qi::AutoMetaValue());

    virtual void metaEmit(unsigned int event, const MetaFunctionParameters& params);
    //// Resolve and bounce to metaEmit
    bool xMetaEmit(const std::string &signature, const MetaFunctionParameters &in);

    /** Connect an event to an arbitrary callback.
     *
     * If you are within a service, it is recommended that you connect the
     * event to one of your Slots instead of using this method.
     */
    template <typename FUNCTOR_TYPE>
    unsigned int connect(const std::string& eventName, FUNCTOR_TYPE callback,
                         EventLoop* ctx = getDefaultObjectEventLoop());
    unsigned int xConnect(const std::string &signature, MetaFunction functor,
                          EventLoop* ctx = getDefaultObjectEventLoop());
    unsigned int connect(unsigned int event, MetaFunction Functor,
                         EventLoop* ctx = getDefaultObjectEventLoop());

    /// Calls given functor when event is fired. Takes ownership of functor.
    virtual unsigned int connect(unsigned int event, const SignalSubscriber& subscriber);

    /// Disconnect an event link. Returns if disconnection was successful.
    virtual bool disconnect(unsigned int linkId);

   //return the list of all subscriber to an event
    std::vector<SignalSubscriber> subscribers(int eventId) const;
    /** Connect an event to a method.
     * Recommended use is when target is not a proxy.
     * If target is a proxy and this is server-side, the event will be
     *    registered localy and the call will be forwarded.
     * If target and this are proxies, the message will be routed through
     * the current process.
     */
    unsigned int connect(unsigned int signal, qi::Object target, unsigned int slot);
    /** Same as connect(signal, target, slot) but with reverse signature,
     * so that we can advertise this method.
     *
     */

    /// Trigger event handlers
    void trigger(unsigned int event, const MetaFunctionParameters& params);
    void moveToEventLoop(EventLoop* ctx);
    EventLoop* eventLoop();

    boost::shared_ptr<ObjectPrivate> _p;
  };

  /** Event subscriber info.
  *
  * Only one of handler or target must be set.
  */
 struct QIMESSAGING_API SignalSubscriber
 {
   SignalSubscriber()
     : handler(0), eventLoop(0), target(), method(0)
   {}

   SignalSubscriber(MetaFunction func, EventLoop* ctx)
     : handler(func), eventLoop(ctx), target(), method(0)
   {}

   SignalSubscriber(Object target, unsigned int method)
     : handler(0), eventLoop(0), target(target), method(method)
   {}

   void call(const MetaFunctionParameters& args);
   // Source information
   SignalBase*        source;
   /// Uid that can be passed to Object::disconnect()
   SignalBase::Link  linkId;

   // Target information
   //   Mode 1: Direct functor call
   MetaFunction       handler;
   EventLoop*         eventLoop;
   //  Mode 2: metaCall
   Object             target;
   unsigned int       method;
 };

  template <typename FUNCTION_TYPE>
  unsigned int Object::connect(const std::string& eventName,
                               FUNCTION_TYPE callback,
                               EventLoop* ctx)
  {
    std::stringstream   signature;
    signature << eventName << "::(";

    typedef typename boost::function_types::parameter_types<FUNCTION_TYPE>::type ArgsType;
    boost::mpl::for_each<
      boost::mpl::transform_view<ArgsType,
        boost::add_pointer<
        boost::remove_const<
        boost::remove_reference<boost::mpl::_1> > > > > (qi::detail::signature_function_arg_apply(&signature));

    signature << ")";

    return xConnect(signature.str(), makeFunctor(callback), ctx);
  }

};

QI_METATYPE_SERIALIZABLE(MetaObject);

/** Register struct with QI binding system.
 * Once called, your structure can be passed as argument to call(), and method
 * using it can be bound with advertiseMethod().
 * Usage: pass the name of the structure, and the list of members.
 */
#define QI_REGISTER_STRUCT(Cname, ...)        \
  QI_DATASTREAM_STRUCT(Cname, __VA_ARGS__)    \
  QI_SIGNATURE_STRUCT(Cname, __VA_ARGS__)

/// Only declare required functions for class registration
#define QI_REGISTER_STRUCT_DECLARE(Cname)    \
  QI_DATASTREAM_STRUCT_DECLARE(Cname)        \
  QI_SIGNATURE_STRUCT_DECLARE(Cname)

/// Implement functions required for class registration
#define QI_REGISTER_STRUCT_IMPLEMENT(Cname, ...)              \
  __QI_DATASTREAM_STRUCT_IMPLEMENT_(/**/, Cname, __VA_ARGS__) \
  __QI_SIGNATURE_STRUCT_IMPLEMENT_(/**/, Cname, __VA_ARGS__)

#define QI_REGISTER_STRUCT_PRIVATE_ACCESS(Cname)  \
  QI_DATASTREAM_STRUCT_PRIVATE_ACCESS(Cname)      \
  QI_SIGNATURE_STRUCT_PRIVATE_ACCESS(Cname)

#include <qimessaging/details/object.hxx>
#endif  // _QIMESSAGING_OBJECT_HPP_
