#pragma once
/*
**  Copyright (C) 2013 Aldebaran Robotics
**  See COPYING for the license
*/

#ifndef _QITYPE_SIGNAL_HPP_
#define _QITYPE_SIGNAL_HPP_

#include <qi/atomic.hpp>
#include <qi/eventloop.hpp>

#include <qitype/anyfunction.hpp>
#include <qitype/typeobject.hpp>

#include <boost/thread/thread.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/enable_shared_from_this.hpp>

#ifdef _MSC_VER
#  pragma warning( push )
#  pragma warning( disable: 4251 )
#endif

// Macro to be more expressive when emitting a signal. Does nothing, it's normal.
#define QI_EMIT

namespace qi {

  class ManageablePrivate;
  class SignalSubscriber;
  class EventLoop;


  class GenericObject;
  typedef boost::shared_ptr<GenericObject> AnyObject;
  typedef boost::weak_ptr<GenericObject> ObjectWeakPtr;
  class SignalBasePrivate;

  typedef qi::uint64_t SignalLink;

  class QITYPE_API SignalBase
  {
  public:
    typedef boost::function<void(bool)> OnSubscribers;
    explicit SignalBase(const Signature &signature, OnSubscribers onSubscribers = OnSubscribers());
    SignalBase(OnSubscribers onSubscribers=OnSubscribers());
    virtual ~SignalBase();
    SignalBase(const SignalBase& b);
    SignalBase& operator = (const SignalBase& b);
    virtual qi::Signature signature() const;

    template<typename FUNCTION_TYPE>
    SignalSubscriber& connect(FUNCTION_TYPE f, MetaCallType model=MetaCallType_Auto);

    SignalSubscriber& connect(qi::AnyObject target, unsigned int slot);
    SignalSubscriber& connect(AnyFunction callback, MetaCallType model=MetaCallType_Auto);
    SignalSubscriber& connect(const SignalSubscriber& s);

    bool disconnectAll();

    /** Disconnect a SignalHandler. The associated callback will not be called
     * anymore as soon as this function returns, but might be called in an
     * other thread while this function runs.
     */
    bool disconnect(const SignalLink& link);

    /** Trigger the signal with given type-erased parameters.
    * @param params the signal arguments
    * @param callType specify how to invoke subscribers.
    *        Used in combination with each subscriber's MetaCallType to
    *        chose between synchronous and asynchronous call.
    *        The combination rule is to honor subscriber's override, then \p callType,
    *        and default to asynchronous
    */
    virtual void trigger(const GenericFunctionParameters& params, MetaCallType callType = MetaCallType_Auto);
    /// Set the MetaCallType used by operator()().
    void setCallType(MetaCallType callType);
    /// Trigger the signal with given arguments, and call type set by setCallType()
    void operator()(
      qi::AutoAnyReference p1 = qi::AutoAnyReference(),
      qi::AutoAnyReference p2 = qi::AutoAnyReference(),
      qi::AutoAnyReference p3 = qi::AutoAnyReference(),
      qi::AutoAnyReference p4 = qi::AutoAnyReference(),
      qi::AutoAnyReference p5 = qi::AutoAnyReference(),
      qi::AutoAnyReference p6 = qi::AutoAnyReference(),
      qi::AutoAnyReference p7 = qi::AutoAnyReference(),
      qi::AutoAnyReference p8 = qi::AutoAnyReference());

    std::vector<SignalSubscriber> subscribers();
    bool hasSubscribers();
    static const SignalLink invalidSignalLink;
  protected:
    typedef boost::function<void(const GenericFunctionParameters& params, MetaCallType callType)> Trigger;
    void callSubscribers(const GenericFunctionParameters& params, MetaCallType callType = MetaCallType_Auto);
    void setTriggerOverride(Trigger trigger);
    void setOnSubscribers(OnSubscribers onSubscribers);
    void callOnSubscribe(bool v);
  public:
    void _setSignature(const Signature &s);
    // C4251
    boost::shared_ptr<SignalBasePrivate> _p;
    friend class SignalBasePrivate;
  };

  template<typename FUNCTION_TYPE>
  inline SignalSubscriber& SignalBase::connect(FUNCTION_TYPE  callback, MetaCallType model)
  {
    return connect(AnyFunction::from(callback), model);
  }

  template<typename T>
  class SignalF: public SignalBase, public boost::function<T>
  {
  public:
    /** Signal constructor
     * @param onSubscribers invoked each time number of subscribers switch
     * between 0 and 1, with argument '!subscribers.empty()'
     * Will not be called when destructor is invoked and all subscribers are removed
    */
    SignalF(OnSubscribers onSubscribers = OnSubscribers());
    SignalF(const SignalF<T>& b);
    SignalF<T>& operator = (const SignalF<T>& b);
    typedef T FunctionType;
    virtual qi::Signature signature() const;
    using boost::function<T>::operator();

    inline SignalSubscriber& connect(boost::function<T> f, MetaCallType model=MetaCallType_Auto)
    {
      return SignalBase::connect(f, model);
    }
    inline SignalSubscriber& connect(AnyFunction f, MetaCallType model=MetaCallType_Auto)
    {
      return SignalBase::connect(f, model);
    }
    /// Auto-disconnects on target destruction
    inline SignalSubscriber& connect(qi::AnyObject target, unsigned int slot)
    {
      return SignalBase::connect(target, slot);
    }
    template<typename U>
    inline SignalSubscriber& connect(SignalF<U>& signal);
    /// IF O is a shared_ptr, will auto-disconnect if object is destroyed
    template<typename O, typename MF>
    inline SignalSubscriber& connect(O* target, MF method, MetaCallType model=MetaCallType_Auto);
    template<typename O, typename MF>
    inline SignalSubscriber& connect(boost::shared_ptr<O> target, MF method, MetaCallType model=MetaCallType_Auto);
  };

namespace detail
{
  template<typename P0, typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7> struct VoidFunctionType                                           { typedef void(type)(P0, P1, P2, P3, P4, P5, P6, P7); };
  template<typename P0, typename P1, typename P2, typename P3, typename P4, typename P5, typename P6> struct VoidFunctionType<P0, P1, P2, P3, P4, P5, P6, void>                     { typedef void(type)(P0, P1, P2, P3, P4, P5, P6); };
  template<typename P0, typename P1, typename P2, typename P3, typename P4, typename P5             > struct VoidFunctionType<P0, P1, P2, P3, P4, P5, void, void>             { typedef void(type)(P0, P1, P2, P3, P4, P5); };
  template<typename P0, typename P1, typename P2, typename P3, typename P4                          > struct VoidFunctionType<P0, P1, P2, P3, P4, void, void, void>           { typedef void(type)(P0, P1, P2, P3, P4); };
  template<typename P0, typename P1, typename P2, typename P3                                       > struct VoidFunctionType<P0, P1, P2, P3, void, void, void, void>         { typedef void(type)(P0, P1, P2, P3); };
  template<typename P0, typename P1, typename P2                                                    > struct VoidFunctionType<P0, P1, P2, void, void, void, void, void>       { typedef void(type)(P0, P1, P2); };
  template<typename P0, typename P1                                                                 > struct VoidFunctionType<P0, P1, void, void, void, void, void, void>     { typedef void(type)(P0, P1); };
  template<typename P0                                                                              > struct VoidFunctionType<P0, void, void, void, void, void, void, void>   { typedef void(type)(P0); };
  template<                                                                                         > struct VoidFunctionType<void, void, void, void, void, void, void, void> { typedef void(type)(); };

}
template<
  typename P0 = void,
  typename P1 = void,
  typename P2 = void,
  typename P3 = void,
  typename P4 = void,
  typename P5 = void,
  typename P6 = void,
  typename P7 = void
  >
  class Signal: public SignalF<typename detail::VoidFunctionType<P0, P1, P2, P3, P4, P5, P6, P7>::type>
  {
  public:
    typedef typename detail::VoidFunctionType<P0, P1, P2, P3, P4, P5, P6, P7>::type FunctionType;
    typedef SignalF<FunctionType> ParentType;
    typedef typename ParentType::OnSubscribers OnSubscribers;
    Signal(OnSubscribers onSubscribers = OnSubscribers())
    : ParentType(onSubscribers) {}
    using boost::function<FunctionType>::operator();
  };
#define QI_SIGNAL_TEMPLATE_DECL typename P0, typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7
#define QI_SIGNAL_TEMPLATE P0,P1,P2,P3,P4,P5,P6,P7

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
     : source(0), linkId(SignalBase::invalidSignalLink), weakLock(0), target(0), method(0), enabled(true)
   {}

   SignalSubscriber(AnyFunction func, MetaCallType model=MetaCallType_Auto, detail::WeakLock* lock = 0);

   SignalSubscriber(qi::AnyObject target, unsigned int method);

   template<typename O, typename MF>
   SignalSubscriber(O* ptr, MF function, MetaCallType model=MetaCallType_Auto);

   template<typename O, typename MF>
   SignalSubscriber(boost::shared_ptr<O> ptr, MF function, MetaCallType model=MetaCallType_Auto);

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

   /// Auto-disconnect if \p ptr cannot be locked
   template<typename T>
   SignalSubscriber& track(boost::weak_ptr<T> ptr);

   //wait till all threads are inactive except the current thread.
   void waitForInactive();

   void addActive(bool acquireLock, boost::thread::id tid = boost::this_thread::get_id());
   void removeActive(bool acquireLock, boost::thread::id tid = boost::this_thread::get_id());
   operator SignalLink() const
   {
     return linkId;
   }
 public:
   // Source information
   SignalBase* source;
   /// Uid that can be passed to GenericObject::disconnect()
   SignalLink  linkId;

   // Target information, kept here to be able to introspect a Subscriber
   //   Mode 1: Direct functor call
   AnyFunction       handler;
   detail::WeakLock* weakLock; // try to acquire weakLocker, disconnect if cant
   MetaCallType      threadingModel;

   //  Mode 2: metaCall
   ObjectWeakPtr*    target;
   unsigned int      method;

   boost::mutex      mutex;
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
