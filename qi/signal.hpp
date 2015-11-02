#pragma once
/*
**  Copyright (C) 2013 Aldebaran Robotics
**  See COPYING for the license
*/

#ifndef _QI_SIGNAL_HPP_
#define _QI_SIGNAL_HPP_

#include <qi/atomic.hpp>

#include <qi/anyfunction.hpp>
#include <qi/type/typeobject.hpp>

#include <boost/thread/thread.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/condition.hpp>
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

  class SignalBasePrivate;

  using SignalLink = qi::uint64_t;

  //Signal are not copyable, they belong to a class.
  class QI_API SignalBase : boost::noncopyable
  {
  public:
    using OnSubscribers = boost::function<void(bool)>;
    explicit SignalBase(const Signature &signature, OnSubscribers onSubscribers = OnSubscribers());
    SignalBase(OnSubscribers onSubscribers=OnSubscribers());
    virtual ~SignalBase();
    virtual qi::Signature signature() const;
    template<typename F>
    SignalSubscriber& connect(boost::function<F> func);
    SignalSubscriber& connect(const SignalSubscriber& s);
    SignalSubscriber& connect(AnyObject object, const unsigned int slot);
    SignalSubscriber& connect(AnyObject object, const std::string& slot);

    /** Disconnect all callbacks from signal.
     *
     * This function will block until all callbacks are finished.
     */
    bool disconnectAll();
    /** Disconnect all callbacks from signal without waiting for them.
     *
     * This function does not block.
     */
    bool asyncDisconnectAll();

    /** Disconnect a SignalHandler.
     *
     * The associated callback will not be called anymore as soon as this
     * function returns.
     *
     * This method blocks until all the already running callbacks are
     * finished.
     */
    bool disconnect(const SignalLink& link);

    /** Disconnect a SignalHandler without waiting for it.
     *
     * Same as disconnect, but this method does not block.
     */
    bool asyncDisconnect(const SignalLink& link);

    /** Trigger the signal with given type-erased parameters.
    * @param params the signal arguments
    * @param callType specify how to invoke subscribers.
    *        Used in combination with each subscriber's MetaCallType to
    *        chose between synchronous and asynchronous call.
    *        The combination rule is to honor subscriber's override, then \p callType,
    *        then signal's callType and default to asynchronous
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
    using Trigger = boost::function<void(const GenericFunctionParameters& params, MetaCallType callType)>;
    void callSubscribers(const GenericFunctionParameters& params, MetaCallType callType = MetaCallType_Auto);
    void setTriggerOverride(Trigger trigger);
    void setOnSubscribers(OnSubscribers onSubscribers);
    void callOnSubscribe(bool v);
    void createNewTrackLink(int& id, SignalLink*& trackLink);
    void disconnectTrackLink(int id);
  public:
    void _setSignature(const Signature &s);
    // C4251
    boost::shared_ptr<SignalBasePrivate> _p;
    friend class SignalBasePrivate;
  };

  template <typename... P> class Signal;

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
    using FunctionType = T;
    virtual qi::Signature signature() const;
    using boost::function<T>::operator();

#ifdef DOXYGEN
    /** Connect a subscriber to this signal.
    *
    * Multiple forms can be used:
    * - connect(function, argOrPlaceholder1, argOrPlaceholder2, ...)
    *   Where function is a function or callable object (such as a boost::function).
    *   If the first argument is a weak ptr or inherits qi::Trackable, the slot
    *   will automatically disconnect if object is no longuer available.
    * - connect(AnyObject target, unsigned int slot)
    * - connect(AnyObject target, const std::string& slotName)
    * - connect(AnyFunction func)
    * - connect(const SignalSubscriber&)
    * - connect(qi::Signal<U>& otherSignal)
    *
    * @return a SignalSubscriber object. This object can be implicitly
    * converted to a SignalLink.
    * @throw runtime_error if the connection could not be made (because of invalid callback
    * arity or argument type)
    */
    SignalSubscriber& connect(...);
#else
    template <typename F>
    SignalSubscriber& connect(F c);
    SignalSubscriber& connect(AnyFunction func);
    SignalSubscriber& connect(const SignalSubscriber& sub);
    template <typename U>
    SignalSubscriber& connect(SignalF<U>& signal);
    template <typename... P>
    SignalSubscriber& connect(Signal<P...>& signal);

    template <typename F, typename Arg0, typename... Args>
    SignalSubscriber& connect(F&& func, Arg0&& arg0, Args&&... args);

    SignalSubscriber& connect(const AnyObject& obj, unsigned int slot);
    SignalSubscriber& connect(const AnyObject& obj, const std::string& slot);
#endif
  };

  /** Class that represent an event to which function can subscribe.
   *
   * \includename{qi/signal.hpp}
   */
  template<typename... P>
  class Signal: public SignalF<void(P...)>
  {
  public:
    typedef void(FunctionType)(P...); // FIXME: VS2013 fails if this is replaced by `using`
    using ParentType = SignalF<FunctionType>;
    using OnSubscribers = typename ParentType::OnSubscribers;
    Signal(OnSubscribers onSubscribers = OnSubscribers())
      : ParentType(onSubscribers) {}
    using boost::function<FunctionType>::operator();
  };

  template <>
  class Signal<void> : public Signal<>
  {};

  /** Event subscriber info.
   *
   * Only one of handler or target must be set.
   *
   * \includename{qi/signal.hpp}
   */
  class QI_API SignalSubscriber
  : public boost::enable_shared_from_this<SignalSubscriber>
  {
  public:
    SignalSubscriber();

    SignalSubscriber(AnyFunction func, MetaCallType callType = MetaCallType_Auto);
    SignalSubscriber(AnyFunction func, ExecutionContext* ec);
    SignalSubscriber(const AnyObject& target, unsigned int method);

    SignalSubscriber(const SignalSubscriber& b);

    ~SignalSubscriber();

    void operator = (const SignalSubscriber& b);

    /** Perform the call.
     *
     * Threading rules in order:
     * - Honor threadingModel if set (not auto)
     * - Honor callTypoe if set (not auto)
     * - Be asynchronous
     */
    void call(const GenericFunctionParameters& args, MetaCallType callType);

    SignalSubscriber& setCallType(MetaCallType ct);

    /// Wait until all threads are inactive except the current thread.
    void waitForInactive();

    void addActive(bool acquireLock, boost::thread::id tid = boost::this_thread::get_id());
    void removeActive(bool acquireLock, boost::thread::id tid = boost::this_thread::get_id());
    operator SignalLink() const
    {
      return linkId;
    }
    /** Try to extract exact signature of this subscriber.
    * @return the signature, or an invalid signature if extraction is impossible
    */
    Signature signature() const;
  public:
    // Source information
    SignalBase* source;
    /// Uid that can be passed to GenericObject::disconnect()
    SignalLink  linkId;

    // Target information, kept here to be able to introspect a Subscriber
    //   Mode 1: Direct functor call
    AnyFunction       handler;
    MetaCallType      threadingModel;

    //   Mode 2: metaCall
    boost::scoped_ptr<AnyWeakObject> target;
    unsigned int      method;

    boost::mutex      mutex;
    // Fields below are protected by lock

    // If enabled is set to false while lock is acquired,
    // No more callback will trigger (activeThreads will se no push-back)
    bool                         enabled;
    // Number of calls in progress.
    // Each entry there is a subscriber call that can no longuer be aborted
    std::vector<boost::thread::id> activeThreads; // order not preserved

    boost::condition               inactiveThread;

    // ExecutionContext on which to schedule the call
    ExecutionContext* executionContext;
  };
  using SignalSubscriberPtr = boost::shared_ptr<SignalSubscriber>;
}

#ifdef _MSC_VER
#  pragma warning( pop )
#endif

#include <qi/type/detail/signal.hxx>

QI_NO_TYPE(qi::SignalBase)

#endif  // _QITYPE_SIGNAL_HPP_
