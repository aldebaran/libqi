#pragma once
/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/

#ifndef _QITYPE_SIGNAL_HPP_
#define _QITYPE_SIGNAL_HPP_

#include <qi/atomic.hpp>
#include <qi/eventloop.hpp>
#include <qitype/signature.hpp>
#include <qitype/functiontype.hpp>
#include <boost/thread/thread.hpp>
#include <boost/thread/mutex.hpp>

namespace qi {

  class ManageablePrivate;
  class SignalSubscriber;
  /** User classes can inherit from Manageable to benefit from  Event loop
  *  management
  */
  class QITYPE_API Manageable
  {
  public:
    Manageable();
    ~Manageable();
    Manageable(const Manageable& b);
    void operator = (const Manageable& b);

    EventLoop* eventLoop() const;
    void moveToEventLoop(EventLoop* eventLoop);

    ManageablePrivate* _p;
  };

  class GenericObject;
  typedef boost::shared_ptr<GenericObject> ObjectPtr;
  typedef boost::weak_ptr<GenericObject> ObjectWeakPtr;
  class SignalBasePrivate;

  class QITYPE_API SignalBase
  {
  public:
    explicit SignalBase(const std::string& signature);
    SignalBase();
    ~SignalBase();
    SignalBase(const SignalBase& b);
    SignalBase& operator = (const SignalBase& b);
    virtual std::string signature() const;

    typedef unsigned int Link;

    template<typename FUNCTION_TYPE>
    Link connect(FUNCTION_TYPE f, EventLoop* ctx = getDefaultObjectEventLoop());

    Link connect(qi::ObjectPtr target, unsigned int slot);
    Link connect(GenericFunction callback, EventLoop* ctx = getDefaultObjectEventLoop());
    Link connect(const SignalSubscriber& s);

    bool disconnectAll();

    /** Disconnect a SignalHandler. The associated callback will not be called
     * anymore as soon as this function returns, but might be called in an
     * other thread while this function runs.
     */
    bool disconnect(const Link& link);

    void trigger(const GenericFunctionParameters& params);
    void operator()(
      qi::AutoGenericValue p1 = qi::AutoGenericValue(),
      qi::AutoGenericValue p2 = qi::AutoGenericValue(),
      qi::AutoGenericValue p3 = qi::AutoGenericValue(),
      qi::AutoGenericValue p4 = qi::AutoGenericValue(),
      qi::AutoGenericValue p5 = qi::AutoGenericValue(),
      qi::AutoGenericValue p6 = qi::AutoGenericValue(),
      qi::AutoGenericValue p7 = qi::AutoGenericValue(),
      qi::AutoGenericValue p8 = qi::AutoGenericValue());

    std::vector<SignalSubscriber> subscribers();
    static const SignalBase::Link invalidLink;
  public:
    boost::shared_ptr<SignalBasePrivate> _p;
  };



  template<typename FUNCTION_TYPE>
  inline SignalBase::Link SignalBase::connect(FUNCTION_TYPE  callback, EventLoop* ctx)
  {
    return connect(makeGenericFunction(callback), ctx);
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
    inline SignalBase::Link connect(boost::function<T> f, EventLoop* ctx=getDefaultObjectEventLoop())
    {
      return SignalBase::connect(f, ctx);
    }
    inline SignalBase::Link connect(GenericFunction f, EventLoop* ctx=getDefaultObjectEventLoop())
    {
      return SignalBase::connect(f, ctx);
    }
    /// Auto-disconnects on target destruction
    inline SignalBase::Link connect(qi::ObjectPtr target, unsigned int slot)
    {
      return SignalBase::connect(target, slot);
    }
    /// IF O is a shared_ptr, will auto-disconnect if object is destroyed
    template<typename O, typename MF>
    inline SignalBase::Link connect(O* target, MF method, EventLoop* ctx=getDefaultObjectEventLoop());
    template<typename O, typename MF>
    inline SignalBase::Link connect(boost::shared_ptr<O> target, MF method, EventLoop* ctx=getDefaultObjectEventLoop());
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
     : weakLock(0), target(0), method(0), enabled(true)
   {}

   SignalSubscriber(GenericFunction func, EventLoop* ctx = getDefaultObjectEventLoop(), detail::WeakLock* lock = 0);

   SignalSubscriber(qi::ObjectPtr target, unsigned int method);

   template<typename O, typename MF>
   SignalSubscriber(O* ptr, MF function, EventLoop* ctx = getDefaultObjectEventLoop());

   template<typename O, typename MF>
   SignalSubscriber(boost::shared_ptr<O> ptr, MF function, EventLoop* ctx = getDefaultObjectEventLoop());

   SignalSubscriber(const SignalSubscriber& b);

   ~SignalSubscriber();

   void operator = (const SignalSubscriber& b);

   void call(const GenericFunctionParameters& args);

   //wait till all threads are inactive except the current thread.
   void waitForInactive();

   void addActive(bool acquireLock, boost::thread::id tid = boost::this_thread::get_id());
   void removeActive(bool acquireLock, boost::thread::id tid = boost::this_thread::get_id());
 public:
   // Source information
   SignalBase*        source;
   /// Uid that can be passed to GenericObject::disconnect()
   SignalBase::Link  linkId;

   // Target information, kept here to be able to introspect a Subscriber
   //   Mode 1: Direct functor call
   GenericFunction      handler;
   detail::WeakLock*    weakLock; // try to acquire weakLocker, disconnect if cant
   boost::function<EventLoop*(void)> eventLoopGetter;

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

#include <qitype/details/signal.hxx>

QI_NO_TYPE(qi::SignalBase)

#endif  // _QITYPE_SIGNAL_HPP_
