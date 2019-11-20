#pragma once
/*
**  Copyright (C) 2013 Aldebaran Robotics
**  See COPYING for the license
*/

#ifndef _QI_SIGNAL_HPP_
#define _QI_SIGNAL_HPP_

#include <boost/function.hpp>
#include <boost/scoped_ptr.hpp>
#include <qi/atomic.hpp>
#include <qi/macro.hpp>

#include <qi/anyfunction.hpp>
#include <qi/type/typeobject.hpp>

#include <ka/macro.hpp>

#include <boost/thread/thread.hpp>
#include <boost/thread/mutex.hpp>

KA_WARNING_PUSH()
KA_WARNING_DISABLE(4251, )

// Macro to be more expressive when emitting a signal. Does nothing, it's normal.
#define QI_EMIT

namespace qi {

  class ManageablePrivate;
  class SignalSubscriber;

  class SignalBase;
  class SignalBasePrivate;

  using SignalLink = qi::uint64_t;

  namespace details_proxysignal
  {
    void setUpProxy(SignalBase&, AnyWeakObject, const std::string&);
  }

  /// SignalBase provides a signal subscription mechanism called "connection".
  /// Derived classes can customize the subscription step by setting
  /// an "onSubscribers" callback. @see onSubscriber.
  class QI_API SignalBase
  {
  public:
    using OnSubscribers = boost::function<Future<void>(bool)>;

    explicit SignalBase(const Signature &signature, OnSubscribers onSubscribers = OnSubscribers());
    SignalBase(const Signature &signature, ExecutionContext* execContext,
               OnSubscribers onSubscribers = OnSubscribers());
    SignalBase(OnSubscribers onSubscribers = OnSubscribers());
    SignalBase(ExecutionContext* execContext, OnSubscribers onSubscribers = OnSubscribers());

    /// SignalBase is not copyable, since subscriptions should not be duplicated.
    SignalBase(const SignalBase&) = delete;
    SignalBase& operator=(const SignalBase&) = delete;

    virtual ~SignalBase();

    virtual qi::Signature signature() const;

    template<typename F>
    SignalSubscriber connect(boost::function<F> func);
    SignalSubscriber connect(AnyObject object, const unsigned int slot);
    SignalSubscriber connect(AnyObject object, const std::string& slot);

    /// The following overloads are the lowest-level
    SignalSubscriber connect(const SignalSubscriber& s);

    /// Connect asynchronously. This is recommended since derived classes may
    /// provide asynchronous customizations for dealing with subscribers.
    /// The callbacks are guaranteed to be called only after the returned
    /// future is set.
    Future<SignalSubscriber> connectAsync(const SignalSubscriber&);

    /** Disconnect all callbacks from signal.
     *
     * This function will block until all callbacks are finished.
     * @return Returns true on success.
     */
    bool disconnectAll();
    /** Disconnect all callbacks from signal without waiting for them.
     *
     * This function does not block.
     * @return A future set to true on success.
     */
    Future<bool> disconnectAllAsync();

    QI_API_DEPRECATED_MSG("use disconnectAllAsync instead")
    bool asyncDisconnectAll()
    {
      disconnectAllAsync();
      return true;
    }

    /** Disconnect a SignalHandler.
     *
     * The associated callback will not be called anymore as soon as this
     * function returns.
     *
     * This method blocks until all the already running callbacks are
     * finished.
     * @return Returns true on success.
     * @throws `std::runtime_error` if the `onSubscribers` callback is set and it
     *         returned a future in error.
     */
    bool disconnect(const SignalLink& link);

    /** Disconnect a SignalHandler without waiting for it.
     *
     * Same as disconnect, but this method does not block.
     * Though this is async, you are guaranteed that once the function returns
     * the future, your callback will not be called anymore.
     * @return A future set to true on success, or set in error if the
     *         `OnSubscribers` callback is set and it returned a future in
     *         error.
     */
    Future<bool> disconnectAsync(const SignalLink& link);

    QI_API_DEPRECATED_MSG("use disconnectAsync instead")
    bool asyncDisconnect(const SignalLink& link)
    {
      disconnectAsync(link);
      return true;
    }

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

    /// Set a function to call when the number of subscribers go between 1 and 0.
    /// You can use this to avoid computing if no one has subscribed to the signal.
    void setOnSubscribers(OnSubscribers onSubscribers);

    static const SignalLink invalidSignalLink;
    void _setSignature(const Signature &s);

  protected:
    void callSubscribers(const GenericFunctionParameters& params, MetaCallType callType = MetaCallType_Auto);
    using Trigger = boost::function<void(const GenericFunctionParameters& params, MetaCallType callType)>;
    void setTriggerOverride(Trigger trigger);
    void callOnSubscribe(bool v);
    void createNewTrackLink(int& id, SignalLink*& trackLink);
    static void disconnectTrackLink(SignalBasePrivate& p, int id);
    ExecutionContext* executionContext() const;
    void clearExecutionContext();

  protected:
    boost::shared_ptr<SignalBasePrivate> _p;
    friend class SignalBasePrivate;
    friend void details_proxysignal::setUpProxy(SignalBase&, AnyWeakObject, const std::string&);
  };

  inline bool isValidSignalLink(SignalLink l)
  {
    return l != SignalBase::invalidSignalLink;
  }

  /// Sets the link to an invalid value.
  /// @returns The previous value of the signal link.
  inline SignalLink exchangeInvalidSignalLink(SignalLink& l)
  {
    return ka::exchange(l, SignalBase::invalidSignalLink);
  }

  template <typename... P> class Signal;

  template<typename T>
  class SignalF: public SignalBase, private boost::function<T>
  {
    using BoostFunctionBase = boost::function<T>;

  public:
    using FunctionType = T;
    using typename BoostFunctionBase::result_type;

    /** Signal constructor
     * @param onSubscribers invoked each time number of subscribers switch
     * between 0 and 1, with argument '!subscribers.empty()'
     * Will not be called when destructor is invoked and all subscribers are removed
    */
    SignalF(OnSubscribers onSubscribers = OnSubscribers());
    SignalF(ExecutionContext* execContext, OnSubscribers onSubscribers);
    virtual qi::Signature signature() const;

    using BoostFunctionBase::operator();

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
    * @warning This function is synchronous and may block the current thread,
    * which may be in charge of dispatching results that could be waited for.
    * Please prefer connectAsync to be totally safe from subtle deadlocks in
    * your programs.
    *
    * @return a SignalSubscriber object. This object can be implicitly
    * converted to a SignalLink.
    * @throw runtime_error if the connection could not be made (because of invalid callback
    * arity or argument type)
    */
    SignalSubscriber connect(...);
#else
    template <typename F>
    SignalSubscriber connect(F c);
    SignalSubscriber connect(AnyFunction func);
    SignalSubscriber connect(const SignalSubscriber& sub);
    template <typename U>
    SignalSubscriber connect(SignalF<U>& signal);
    template <typename... P>
    SignalSubscriber connect(Signal<P...>& signal);

    template <typename F, typename Arg0, typename... Args>
    SignalSubscriber connect(F&& func, Arg0&& arg0, Args&&... args);

    SignalSubscriber connect(const AnyObject& obj, unsigned int slot);
    SignalSubscriber connect(const AnyObject& obj, const std::string& slot);
#endif

  private:
    template< class ForcedSignalType, class SignalType >
    SignalSubscriber connectSignal(SignalType& signal);
  };

  /** Class that represent an event to which function can subscribe.
   *
   * \includename{qi/signal.hpp}
   */
  template<typename... P>
  class Signal: public SignalF<void(P...)>
  {
  public:
    using ParentType = SignalF<void(P...)>;
    using typename ParentType::FunctionType;

    using OnSubscribers = typename ParentType::OnSubscribers;
    Signal(OnSubscribers onSubscribers = OnSubscribers())
      : ParentType(onSubscribers) {}
    explicit Signal(ExecutionContext* execContext, OnSubscribers onSubscribers = OnSubscribers())
      : ParentType(execContext, onSubscribers) {}

    using ParentType::operator();
  };

  template <>
  class Signal<void> : public Signal<>
  {};

  struct SignalSubscriberPrivate;

  /** Event subscriber info.
   *
   * Only one of handler or target must be set.
   * This class is copyable but has entity semantics.
   *
   * \includename{qi/signal.hpp}
   */
QI_WARNING_PUSH()
QI_WARNING_DISABLE(4996, deprecated-declarations) // ignore linkId deprecation warnings
  class QI_API SignalSubscriber
  {
QI_WARNING_POP()
  public:
    friend class FunctorCall;
    friend class ManageablePrivate;
    friend class SignalBase;
    friend class SignalBasePrivate;

    SignalSubscriber();

    SignalSubscriber(AnyFunction func, MetaCallType callType = MetaCallType_Auto);
    SignalSubscriber(AnyFunction func, ExecutionContext* ec);
    SignalSubscriber(const AnyObject& target, unsigned int method);

    // This is copiable but not movable (never invalid).
    SignalSubscriber(const SignalSubscriber& other);
    SignalSubscriber& operator=(const SignalSubscriber& other);

    ~SignalSubscriber();

    /** Perform the call.
     *
     * Threading rules in order:
     * - Honor threadingModel if set (not auto)
     * - Honor callTypoe if set (not auto)
     * - Be asynchronous
     */
    void call(const GenericFunctionParameters& args, MetaCallType callType);

    void call(const std::shared_ptr<GenericFunctionParameters>& args,
      MetaCallType callType);

    SignalSubscriber setCallType(MetaCallType ct);

    /// @return the identifier of the subscription (aka link)
    SignalLink link() const;
    operator SignalLink() const;

    /** Try to extract exact signature of this subscriber.
    * @return the signature, or an invalid signature if extraction is impossible
    */
    Signature signature() const;

  private:
    std::shared_ptr<SignalSubscriberPrivate> _p;

    void callImpl(const GenericFunctionParameters& args);

    boost::optional<ExecutionContext*> executionContextFor(MetaCallType callType) const;

    // Call the subscriber with the given arguments, which can be passed by
    // value or by pointer (any `Readable` will do).
    //
    // (GenericFunctionParameters || Readable<GenericFunctionParameters>) Args
    template<typename Args>
    void callWithValueOrPtr(const Args& args, MetaCallType callType);

  public:
    QI_API_DEPRECATED_MSG("please use link() instead or cast to qi::SignalLink")
    SignalLink linkId;
  };

  using SignalSubscriberPtr = boost::shared_ptr<SignalSubscriber>;

  struct SignalSubscriberPrivate
  {
    SignalSubscriberPrivate();
    ~SignalSubscriberPrivate();

    // Non-copyable
    SignalSubscriberPrivate(const SignalSubscriberPrivate&) = delete;
    SignalSubscriberPrivate& operator=(const SignalSubscriberPrivate&) = delete;

    // Source information
    boost::weak_ptr<SignalBasePrivate> source;
    /// Uid that can be passed to GenericObject::disconnect()
    SignalLink  linkId = SignalBase::invalidSignalLink;

    // Target information, kept here to be able to introspect a Subscriber
    //   Mode 1: Direct functor call
    AnyFunction handler;
    MetaCallType threadingModel = MetaCallType_Direct;

    //   Mode 2: metaCall
    boost::scoped_ptr<AnyWeakObject> target;
    unsigned int method = 0;

    // If enabled is set to false, no more callback will trigger
    std::atomic<bool> enabled{true};

    // ExecutionContext on which to schedule the call
    std::atomic<ExecutionContext*> executionContext{nullptr};
  };
} // qi

KA_WARNING_POP()

#include <qi/type/detail/signal.hxx>

QI_NO_TYPE(qi::SignalBase)

#endif  // _QITYPE_SIGNAL_HPP_
