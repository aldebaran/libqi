#pragma once
/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/

#ifndef _QITYPE_SIGNAL_HPP_
#define _QITYPE_SIGNAL_HPP_

#include <qi/atomic.hpp>
#include <qi/eventloop.hpp>

#include <qitype/functiontype.hpp>
#include <qitype/functiontypefactory.hpp>
#include <qitype/typeobject.hpp>
#include <qitype/manageable.hpp>

#include <boost/thread/thread.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/enable_shared_from_this.hpp>

#ifdef _MSC_VER
#  pragma warning( push )
#  pragma warning( disable: 4251 )
#endif

namespace qi {

  class ManageablePrivate;
  class SignalSubscriber;
  class EventLoop;


  class GenericObject;
  typedef boost::shared_ptr<GenericObject> ObjectPtr;
  typedef boost::weak_ptr<GenericObject> ObjectWeakPtr;
  class SignalBasePrivate;

  class QITYPE_API SignalBase
  {
  public:
    explicit SignalBase(const std::string& signature);
    SignalBase();
    virtual ~SignalBase();
    SignalBase(const SignalBase& b);
    SignalBase& operator = (const SignalBase& b);
    virtual std::string signature() const;

    typedef unsigned int Link;

    template<typename FUNCTION_TYPE>
    SignalSubscriber& connect(FUNCTION_TYPE f, MetaCallType model=MetaCallType_Direct);

    SignalSubscriber& connect(qi::ObjectPtr target, unsigned int slot);
    SignalSubscriber& connect(GenericFunction callback, MetaCallType model=MetaCallType_Direct);
    SignalSubscriber& connect(const SignalSubscriber& s);

    bool disconnectAll();

    /** Disconnect a SignalHandler. The associated callback will not be called
     * anymore as soon as this function returns, but might be called in an
     * other thread while this function runs.
     */
    bool disconnect(const Link& link);

    /** Trigger the signal with given type-erased parameters.
    * @param params the signal arguments
    * @param callType specify how to invoke subscribers.
    *        Used in combination with each subscriber's MetaCallType to
    *        chose between synchronous and asynchronous call.
    *        The combination rule is to honor subscriber's override, then \p callType,
    *        and default to asynchronous
    */
    void trigger(const GenericFunctionParameters& params, MetaCallType callType = MetaCallType_Auto);
    /// Set the MetaCallType used by operator()().
    void setCallType(MetaCallType callType);
    /// Trigger the signal with given arguments, and call type set by setCallType()
    void operator()(
      qi::AutoGenericValuePtr p1 = qi::AutoGenericValuePtr(),
      qi::AutoGenericValuePtr p2 = qi::AutoGenericValuePtr(),
      qi::AutoGenericValuePtr p3 = qi::AutoGenericValuePtr(),
      qi::AutoGenericValuePtr p4 = qi::AutoGenericValuePtr(),
      qi::AutoGenericValuePtr p5 = qi::AutoGenericValuePtr(),
      qi::AutoGenericValuePtr p6 = qi::AutoGenericValuePtr(),
      qi::AutoGenericValuePtr p7 = qi::AutoGenericValuePtr(),
      qi::AutoGenericValuePtr p8 = qi::AutoGenericValuePtr());

    std::vector<SignalSubscriber> subscribers();
    static const SignalBase::Link invalidLink;
  public:
    // C4251
    boost::shared_ptr<SignalBasePrivate> _p;
  };

  template<typename FUNCTION_TYPE>
  inline SignalSubscriber& SignalBase::connect(FUNCTION_TYPE  callback, MetaCallType model)
  {
    return connect(makeGenericFunction(callback), model);
  }

  template<typename T>
  class Signal: public SignalBase, public boost::function<T>
  {
  public:
    Signal();
    Signal(const Signal<T>& b);
    Signal<T>& operator = (const Signal<T>& b);
    virtual std::string signature() const;
    using boost::function<T>::operator();

    inline SignalSubscriber& connect(boost::function<T> f, MetaCallType model=MetaCallType_Direct)
    {
      return SignalBase::connect(f, model);
    }
    inline SignalSubscriber& connect(GenericFunction f, MetaCallType model=MetaCallType_Direct)
    {
      return SignalBase::connect(f, model);
    }
    /// Auto-disconnects on target destruction
    inline SignalSubscriber& connect(qi::ObjectPtr target, unsigned int slot)
    {
      return SignalBase::connect(target, slot);
    }
    /// IF O is a shared_ptr, will auto-disconnect if object is destroyed
    template<typename O, typename MF>
    inline SignalSubscriber& connect(O* target, MF method, MetaCallType model=MetaCallType_Direct);
    template<typename O, typename MF>
    inline SignalSubscriber& connect(boost::shared_ptr<O> target, MF method, MetaCallType model=MetaCallType_Direct);
  };

  namespace detail
  {
    /// Interface for a weak-lock recursive mechanism: if lock fail, unregister callback
    class WeakLock
    {
    public:
      virtual ~WeakLock(){}
      virtual bool tryLock() = 0;
      virtual void unlock() = 0;
      virtual WeakLock* clone() = 0;
    };
  }

 /** Event subscriber info.
  *
  * Only one of handler or target must be set.
  */
 class QITYPE_API SignalSubscriber
 : public boost::enable_shared_from_this<SignalSubscriber>
 {
 public:
   SignalSubscriber()
     : source(0), linkId(SignalBase::invalidLink), weakLock(0), target(0), method(0), enabled(true)
   {}

   SignalSubscriber(GenericFunction func, MetaCallType model=MetaCallType_Direct, detail::WeakLock* lock = 0);

   SignalSubscriber(qi::ObjectPtr target, unsigned int method);

   template<typename O, typename MF>
   SignalSubscriber(O* ptr, MF function, MetaCallType model=MetaCallType_Direct);

   template<typename O, typename MF>
   SignalSubscriber(boost::shared_ptr<O> ptr, MF function, MetaCallType model=MetaCallType_Direct);

   SignalSubscriber(const SignalSubscriber& b);

   ~SignalSubscriber();

   void operator = (const SignalSubscriber& b);

   /* Perform the call.
    * Threading rules in order:
    * - Honor threadingModel if set (not auto)
    * - Honor callTypoe if set (not auto)
    * - Be asynchronous
    */
   void call(const GenericFunctionParameters& args, MetaCallType callType);

   //wait till all threads are inactive except the current thread.
   void waitForInactive();

   void addActive(bool acquireLock, boost::thread::id tid = boost::this_thread::get_id());
   void removeActive(bool acquireLock, boost::thread::id tid = boost::this_thread::get_id());
   operator SignalBase::Link() const
   {
     return linkId;
   }
 public:
   // Source information
   SignalBase*        source;
   /// Uid that can be passed to GenericObject::disconnect()
   SignalBase::Link  linkId;

   // Target information, kept here to be able to introspect a Subscriber
   //   Mode 1: Direct functor call
   GenericFunction      handler;
   detail::WeakLock*    weakLock; // try to acquire weakLocker, disconnect if cant
   MetaCallType threadingModel;

   //  Mode 2: metaCall
   ObjectWeakPtr*       target;
   unsigned int         method;

   boost::mutex                 mutex;
   // Fields below are protected by lock

   // If enabled is set to false while lock is acquired,
   // No more callback will trigger (activeThreads will se no push-back)
   bool                         enabled;
   // Number of calls in progress.
   // Each entry there is a subscriber call that can no longuer be aborted
   std::vector<boost::thread::id> activeThreads; // order not preserved
 };
 typedef boost::shared_ptr<SignalSubscriber> SignalSubscriberPtr;
}

#ifdef _MSC_VER
#  pragma warning( pop )
#endif

#include <qitype/details/signal.hxx>

QI_NO_TYPE(qi::SignalBase)

#endif  // _QITYPE_SIGNAL_HPP_
