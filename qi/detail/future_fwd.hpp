#pragma once
/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/

#ifndef _QI_FUTURE_HPP_
# define _QI_FUTURE_HPP_

# include <qi/api.hpp>
# include <vector>
# include <qi/atomic.hpp>
# include <qi/config.hpp>
# include <qi/trackable.hpp>
# include <qi/clock.hpp>
# include <qi/detail/mpl.hpp>
# include <qi/anyvalue.hpp>

# include <boost/shared_ptr.hpp>
# include <boost/make_shared.hpp>
# include <boost/function.hpp>
# include <boost/bind.hpp>
# include <boost/thread/recursive_mutex.hpp>

# ifdef _MSC_VER
#   pragma warning( push )
#   pragma warning( disable: 4251 )
#   pragma warning( disable: 4275 ) //std::runtime_error: no dll interface
# endif

namespace qi {

  class AnyReference;

  namespace detail
  {
    template<typename T>
    struct FutureType
    {
      typedef T type;
      typedef T typecast;
    };

    struct FutureHasNoValue {};
    // Hold a void* for Future<void>
    template<>
    struct FutureType<void>
    {
      typedef void* type;
      typedef FutureHasNoValue typecast;
    };

    template <typename T>
    class AddUnwrap
    {};
  }

  class Actor;
  class Strand;

  template <typename T> class Future;
  template <typename T> class FutureSync;
  template <typename T> class Promise;

  namespace detail {
    template <typename T> class FutureBaseTyped;

    template<typename FT>
    void futureCancelAdapter(
                             boost::weak_ptr<detail::FutureBaseTyped<FT> > wf);
  }

  /** State of the future.
   */
  enum FutureState {
    FutureState_None,               ///< Future is not tied to a promise
    FutureState_Running,            ///< Operation pending
    FutureState_Canceled,           ///< The future has been canceled
    FutureState_FinishedWithError,  ///< The operation is finished with an error
    FutureState_FinishedWithValue,  ///< The operation is finished with a value
  };

  enum FutureCallbackType {
    FutureCallbackType_Sync  = 0,
    FutureCallbackType_Async = 1,
    FutureCallbackType_Auto  = 2
  };

  enum FutureTimeout {
    FutureTimeout_Infinite = ((int) 0x7fffffff),
    FutureTimeout_None     = 0,
  };

  typedef void* FutureUniqueId;

  /** base exception raised for all future error.
   */
  class QI_API FutureException : public std::runtime_error {
  public:
    enum ExceptionState {
      ExceptionState_FutureTimeout,       ///< No result ready
      ExceptionState_FutureCanceled,      ///< The future has been canceled
      ExceptionState_FutureNotCancelable, ///< The future is not cancelable
      ExceptionState_FutureHasNoError,    ///< asked for error, but there is no error
      ExceptionState_FutureUserError,     ///< real future error
      ExceptionState_PromiseAlreadySet,   ///< when the promise is already set.
      ExceptionState_FutureInvalid,       ///< the future is not associated to a promise
    };

    explicit FutureException(const ExceptionState &es, const std::string &str = std::string())
      : std::runtime_error(stateToString(es) + str)
      , _state(es)
    {}

    inline ExceptionState state() const { return _state; }

    std::string stateToString(const ExceptionState &es);

    virtual ~FutureException() throw()
    {}

  private:
    ExceptionState _state;
  };

  /** Exception inherited from FutureException
   *  This exception represent an error reported by the operation.
   */
  class QI_API FutureUserException : public FutureException {
  public:

    explicit FutureUserException(const std::string &str = std::string())
      : FutureException(ExceptionState_FutureUserError, str)
    {}

    virtual ~FutureUserException() throw()
    {}
  };

  /** Class that represents a value that will be set later in time.
   *
   * \includename{qi/future.hpp}
   */
  template <typename T>
  class Future : public detail::AddUnwrap<T> {
  public:
    typedef typename detail::FutureType<T>::type     ValueType;
    typedef typename detail::FutureType<T>::typecast ValueTypeCast;

  public:
    Future()
      : _p(boost::make_shared<detail::FutureBaseTyped<T> >())
    {
    }

    Future(const Future<T>& b)
      : _p(b._p)
    {}

    bool operator==(const Future<T> &other)
    {
      return _p.get() == other._p.get();
    }

    inline Future<T>& operator=(const Future<T>& b)
    {
      _p = b._p;
      return *this;
    }

    bool operator < (const Future<T>& b) const
    {
      return _p.get() < b._p.get();
    }

    FutureUniqueId uniqueId() const
    {
      return _p.get();
    }

    /// @returns true if this future is associated to a promise, false otherwise.
    bool isValid() const
    {
      return _p->state() != FutureState_None;
    }

    /** Construct a Future that already contains a value.
     */
    explicit Future<T>(const ValueType& v, FutureCallbackType async = FutureCallbackType_Auto)
    {
      Promise<T> promise(async);
      promise.setValue(v);
      *this = promise.future();
    }

    /**
     * @brief Return the value associated to a Future.
     * @param msecs timeout
     * @return the value
     *
     * This function can throw for many reason:
     *   - wait timeout
     *   - user error
     *   - future canceled
     *
     * if an error is set, then value throw a FutureUserException, others errors are FutureException.
     */
    inline const ValueType &value(int msecs = FutureTimeout_Infinite) const
    { return _p->value(msecs); }

    /** same as value() with an infinite timeout.
     */
    inline operator const ValueTypeCast&() const
    { return _p->value(FutureTimeout_Infinite); }

    /** Wait for future to contain a value or an error
        @param msecs Maximum time to wait in milliseconds, 0 means return immediately.
        @return a FutureState corresponding to the state of the future.
    */
    inline FutureState wait(int msecs = FutureTimeout_Infinite) const
    { return _p->wait(msecs); }

    /** Wait for future to contain a value or an error
        @param duration Maximum time to wait
        @return a FutureState corresponding to the state of the future.
    */
    inline FutureState wait(qi::Duration duration) const
    { return _p->wait(duration); }

    inline FutureState waitFor(qi::Duration duration) const
    { return this->wait(duration); }

    /** Wait for future to contain a value or an error
        @param timepoint Time until which we can wait
        @return a FutureState corresponding to the state of the future.
    */
    inline FutureState wait(qi::SteadyClock::time_point timepoint) const
    { return _p->wait(timepoint); }

    inline FutureState waitUntil(qi::SteadyClock::time_point timepoint) const
    { return this->wait(timepoint); }

    /**
     * @return true if the future is finished
     * do not throw
     */
    inline bool isFinished() const
    { return _p->isFinished(); }

    /**
     * @return true if the future is running
     * do not throw
     */
    inline bool isRunning() const
    { return _p->isRunning(); }

    /**
     * @return true if the future has been canceled
     * This means that the future has been fully canceled, not that a cancel
     * was requested.
     * do not throw
     */
    inline bool isCanceled() const
    { return _p->isCanceled(); }

    /**
     * @param msecs timeout
     * @return true if the future has an error.
     * throw in the following case:
     *   - timeout
     */
    inline bool hasError(int msecs = FutureTimeout_Infinite) const
    { return _p->hasError(msecs); }

    /**
     * @param msecs timeout
     * @return
     * true if the future has a value.
     * throw in the following case:
     *   - timeout
     */
    inline bool hasValue(int msecs = FutureTimeout_Infinite) const
    { return _p->hasValue(msecs); }

    /**
     * @param msecs
     * @return the error
     * throw on timeout
     * throw if the future do not have an actual error.
     */
    inline const std::string &error(int msecs = FutureTimeout_Infinite) const
    { return _p->error(msecs); }

    /** Make the future sync
     * Should not be useful, use wait().
     */
    inline FutureSync<T> sync()
    {
      return FutureSync<T>(*this);
    };

    /** cancel() the asynchronous operation if possible
     * Exact effect is controlled by the cancel implementation, but it is
     * expected to set a value or an error to the Future as fast as possible.
     * Note that cancelation may be asynchronous.
     * @throw ExceptionState_FutureNotCancelable if isCancelable() is false.
     */
    void cancel()
    {
      _p->cancel(*this);
    }

    /** @return true if the future can be canceled. This does not mean that
     * cancel will succeed.
     */
    bool isCancelable() const
    {
      return _p->isCancelable();
    }

#ifdef DOXYGEN
    /**
     * @brief Execute a callback when the future is finished.
     *
     * The callback will receive this future as argument and all other arguments passed to this function.
     *
     * If the first argument bound to this function is a weak_ptr it will be locked. If it is a Trackable, the callback
     * won't be called after the object's destruction. If it is an Actor, the call will be stranded.
     *
     * @tparam R the return type of your callback as it is hard to deduce without C++11.
     *
     * @return a future that will receive the value returned by the callback or an error if the callback threw.
     */
    template <typename R>
    Future<R> thenR(
        FutureCallbackType type,
        const boost::function<R(const Future<T>&)>& func, ...);

    /**
     * @brief Same as thenR(), but with type defaulted to FutureCallbackType_Auto.
     */
    template <typename R>
    Future<R> thenR(
        const boost::function<R(const Future<T>&)>& func, ...)
    {
      return this->thenR(FutureCallbackType_Auto, func);
    }
#else
    template <typename R>
    Future<R> thenR(
        FutureCallbackType type,
        const boost::function<R(const Future<T>&)>& func);

    template <typename R>
    Future<R> thenR(
        const boost::function<R(const Future<T>&)>& func)
    {
      return this->thenR(FutureCallbackType_Auto, func);
    }

#define genCall(n, ATYPEDECL, ATYPES, ADECL, AUSE, comma)                    \
  template <typename R, typename AF, typename ARG0 comma ATYPEDECL>          \
  Future<R> thenR(const AF& func, const ARG0& arg0 comma ADECL)              \
  {                                                                          \
    return this->thenR<R>(FutureCallbackType_Auto, func, arg0 comma AUSE);  \
  }                                                                          \
  template <typename R, typename AF, typename ARG0 comma ATYPEDECL>          \
  Future<R> thenR(FutureCallbackType type, const AF& func,                   \
                  const ARG0& arg0 comma ADECL)                              \
  {                                                                          \
    return _thenMaybeActor<R, ARG0>(                                         \
        arg0, ::qi::bind<R(const Future<T>&)>(func, arg0 comma AUSE), type); \
  }
    QI_GEN(genCall)
#undef genCall
#endif

    /**
     * @brief Same as thenR(), but the callback is called only if this future finishes with a value.
     *
     * The callback will receive the value of this future, as opposed to this future itself.
     *
     * If this future finishes with an error or a cancel, the callback will not be called and the returned future will
     * finish in the same state.
     *
     * @remark Variadic variants of this function have not been implemented yet, waiting for C++11.
     */
    // TODO do variadic ones when we are C++11 TT_TT
    template <typename R>
    Future<R> andThenR(
        FutureCallbackType type,
        const boost::function<R(const typename Future<T>::ValueType&)>& func);

    /**
     * \brief Get a functor that will cancel the future.
     *
     * This functor will not keep the future alive, which is useful to avoid reference cycles. If the future does not
     * exist anymore, this is a no-op.
     *
     * \note This function should only be useful for bindings, you probably don't need it.
     */
    boost::function<void()> makeCanceler();

    /**
     * @brief Same as andThenR(), but with type defaulted to FutureCallbackType_Auto.
     */
    template <typename R>
    Future<R> andThenR(
        const boost::function<R(const typename Future<T>::ValueType&)>& func)
    {
      return this->andThenR(FutureCallbackType_Auto, func);
    }

  public:
    typedef boost::function<void (Future<T>) > Connection;

    /** Connect a callback function that will be called once when the Future
     * finishes (that is, switches from running to an other state).
     *
     * If type is sync, connect may block and call the callback synchronously
     * if the future is already set.
     *
     * It guaranteed that your callback will be called exactly once (unless the
     * promise is never set or the promise is reset, which is deprecated).
     */
    template<typename AF>
    inline void connect(const AF& fun,
                        FutureCallbackType type = FutureCallbackType_Auto)
    {
      _p->connect(*this, fun, type);
    }
#ifdef DOXYGEN
    /** Connect a callback with binding and tracking support.
     *
     * If the first argument is a weak_ptr or a pointer inheriting from
     * qi::Trackable, the callback will not be called if tracked object was
     * destroyed.
     */
    template<typename FUNCTYPE, typename ARG0>
    void connect(FUNCTYPE fun, ARG0 tracked, ...,
                 FutureCallbackType type = FutureCallbackType_Auto);
#else
#define genCall(n, ATYPEDECL, ATYPES, ADECL, AUSE, comma)                 \
  template <typename AF, typename ARG0 comma ATYPEDECL>                   \
  inline void connect(const AF& fun, const ARG0& arg0 comma ADECL,        \
                      FutureCallbackType type = FutureCallbackType_Auto) \
  {                                                                       \
    _connectMaybeActor<ARG0>(                                             \
        arg0, ::qi::bind<void(Future<T>)>(fun, arg0 comma AUSE), type);   \
  }
    QI_GEN(genCall)
#undef genCall
#endif

    inline void connectWithStrand(qi::Strand* strand,
        const boost::function<void(const Future<T>&)>& cb)
    {
      _p->connect(
          *this,
          transformStrandedCallback(strand, cb),
          FutureCallbackType_Sync);
    }

    // Our companion library libqitype requires a connect with same signature for all instantiations
    inline void _connect(const boost::function<void()>& s)
    {
      connect(boost::bind(s));
    }

    boost::shared_ptr<detail::FutureBaseTyped<T> > impl() { return _p;}
    Future(boost::shared_ptr<detail::FutureBaseTyped<T> > p) :
      _p(p)
    {
      assert(_p);
    }

  protected:
    // C4251 needs to have dll-interface to be used by clients of class 'qi::Future<T>'
    boost::shared_ptr< detail::FutureBaseTyped<T> > _p;
    friend class Promise<T>;
    friend class FutureSync<T>;

    template<typename R>
    friend void adaptFutureUnwrap(Future<AnyReference>& f, Promise<R>& p);
    template<typename FT, typename PT>
    friend void adaptFuture(const Future<FT>& f, Promise<PT>& p);
    template<typename FT, typename PT, typename CONV>
    friend void adaptFuture(const Future<FT>& f, Promise<PT>& p,
                            CONV converter);
    template<typename R>
    friend void adaptFuture(Future<AnyReference>& f, Promise<R>& p);

    template<typename FT>
    friend void detail::futureCancelAdapter(
        boost::weak_ptr<detail::FutureBaseTyped<FT> > wf);
    friend class detail::AddUnwrap<T>;

  private:
    friend class ServiceBoundObject;

    // Nuke this when C++03 ends
    void setOnDestroyed(boost::function<void(ValueType)> cb)
    {
      _p->setOnDestroyed(cb);
    }

    template <typename Arg, typename R>
    static qi::Future<R> binder(
        const boost::function<qi::Future<void>(const boost::function<void()>&)>& poster,
        const boost::function<R(const Arg&)>& callback, const Arg& fut);

    template <typename Arg, typename R>
    boost::function<qi::Future<R>(const Arg&)> transformStrandedCallback(
        qi::Strand* strand,
        const boost::function<R(const Arg&)>& cb);

    template <typename ARG0>
    typename boost::enable_if<
        boost::is_base_of<Actor, typename detail::Unwrap<ARG0>::type>,
        void>::type
        _connectMaybeActor(const ARG0& arg0,
                           const boost::function<void(const Future<T>&)>& cb,
                           FutureCallbackType type);
    template <typename ARG0>
    typename boost::disable_if<
        boost::is_base_of<Actor, typename detail::Unwrap<ARG0>::type>,
        void>::type
        _connectMaybeActor(const ARG0& arg0,
                           const boost::function<void(const Future<T>&)>& cb,
                           FutureCallbackType type);

    template <typename R>
    static void _continuate(const Future<T>& future,
        const boost::function<R(const Future<T>&)>& func,
        Promise<R>& promise);

    static void _cancelContinuation(const Future<T>& future,
        Promise<T>& promise);

    template <typename R, typename ARG0, typename AF>
    typename boost::enable_if<
        boost::is_base_of<Actor, typename detail::Unwrap<ARG0>::type>,
        qi::Future<R> >::type
        _thenMaybeActor(const ARG0& arg0,
            const AF& cb,
            FutureCallbackType type);
    template <typename R, typename ARG0, typename AF>
    typename boost::disable_if<
        boost::is_base_of<Actor, typename detail::Unwrap<ARG0>::type>,
        qi::Future<R> >::type
        _thenMaybeActor(const ARG0& arg0,
            const AF& cb,
            FutureCallbackType type);

    static void _weakCancelCb(const boost::weak_ptr<detail::FutureBaseTyped<T> >& wfuture);
  };

  /** This class allow throwing on error and being synchronous
   *  when the future is not handled by the client.
   *
   *  This class should only be used as return type. If you want to store it,
   *  use qi::Future.
   *
   *  \includename{qi/future.hpp}
   */
  template<typename T>
  class FutureSync
  {
  public:
    typedef typename Future<T>::ValueType ValueType;
    typedef typename Future<T>::ValueTypeCast ValueTypeCast;
    typedef typename Future<T>::Connection Connection;
    // This future cannot be set, so sync starts at false
    FutureSync() : _sync(false) {}

    FutureSync(const Future<T>& b)
      : _sync(true)
    {
      _future = b;
    }

    FutureSync(const FutureSync<T>& b)
      : _sync(true)
    {
      _future = b._future;
      b._sync = false;
    }

    explicit FutureSync<T>(const ValueType& v)
    : _sync(false)
    {
      Promise<T> promise;
      promise.setValue(v);
      _future = promise.future();
    }

    inline FutureSync<T>& operator=(const FutureSync<T>& b)
    {
      _future = b;
      _sync = true;
      b._sync = false;
      return *this;
    }

    inline FutureSync<T>& operator=(const Future<T>& b)
    {
      _future = b;
      _sync = true;
      return *this;
    }

    /** will block until the future returns if the future is kept synchronous
     * @warning this will throw if the future returns an error
     */
    ~FutureSync() QI_NOEXCEPT(false)
    {
      if (_sync)
        _future.value();
    }

    operator Future<T>()
    {
      return async();
    }

    bool operator < (const FutureSync<T>& b) const
    {
      return _future._p.get() < b._future._p.get();
    }

    FutureUniqueId uniqueId() const
    {
      return _future.uniqueId();
    }

    const ValueType &value(int msecs = FutureTimeout_Infinite) const   { _sync = false; return _future.value(msecs); }
    operator const typename Future<T>::ValueTypeCast&() const          { _sync = false; return _future.value(); }
    FutureState wait(int msecs = FutureTimeout_Infinite) const         { _sync = false; return _future.wait(msecs); }
    FutureState wait(qi::Duration duration) const                      { _sync = false; return _future.wait(duration); }
    FutureState waitFor(qi::Duration duration) const                   { _sync = false; return _future.waitFor(duration); }
    FutureState wait(qi::SteadyClock::time_point timepoint) const      { _sync = false; return _future.wait(timepoint); }
    FutureState waitUntil(qi::SteadyClock::time_point timepoint) const { _sync = false; return _future.waitUntil(timepoint); }
    bool isRunning() const                                             { _sync = false; return _future.isRunning(); }
    bool isFinished() const                                            { _sync = false; return _future.isFinished(); }
    bool isCanceled() const                                            { _sync = false; return _future.isCanceled(); }
    bool hasError(int msecs = FutureTimeout_Infinite) const            { _sync = false; return _future.hasError(msecs); }
    bool hasValue(int msecs = FutureTimeout_Infinite) const            { _sync = false; return _future.hasValue(msecs); }
    const std::string &error(int msecs = FutureTimeout_Infinite) const { _sync = false; return _future.error(msecs); }
    void cancel()                                                      { _sync = false; _future.cancel(); }
    bool isCancelable() const                                          { _sync = false; return _future.isCancelable(); }
    void connect(const Connection& s)                                  { _sync = false; _future.connect(s);}
    void _connect(const boost::function<void()>& s)                    { _sync = false; _future._connect(s);}

#ifdef DOXYGEN
    /** Connect a callback with binding and tracking support.
     *
     * If the first argument is a weak_ptr or a pointer inheriting from
     * qi::Trackable, the callback will not be called if tracked object was
     * destroyed.
     */
    template<typename FUNCTYPE, typename ARG0>
    void connect(FUNCTYPE fun, ARG0 tracked, ...);
#else
#define genCall(n, ATYPEDECL, ATYPES, ADECL, AUSE, comma)               \
    template<typename AF, typename ARG0 comma ATYPEDECL>                \
    inline void connect(const AF& fun, const ARG0& arg0 comma ADECL)    \
    {                                                                   \
      _sync = false;                                                    \
      connect(::qi::bind<void(FutureSync<T>)>(fun, arg0 comma AUSE));   \
    }
    QI_GEN(genCall)
#undef genCall
#endif

    Future<T> async()
    {
      _sync = false;
      return _future;
    }

  protected:
    mutable bool _sync;
    Future<T> _future;
    friend class Future<T>;
  };

  /** A Promise is used to create and satisfy a Future.
   *
   * \includename{qi/future.hpp}
   */
  template <typename T>
  class Promise {
  public:
    typedef typename detail::FutureType<T>::type ValueType;

    /** Create a standard promise.
     *  @param async specify how callbacks registered with Future::connect
     *         are called: synchronously from the Promise setter, or
     *         asynchronously from a thread pool.
     */
    explicit Promise(FutureCallbackType async = FutureCallbackType_Auto) {
      _f._p->reportStart();
      _f._p->_async = async;
      ++_f._p->_promiseCount;
    }

    /** Create a canceleable promise. If Future<T>::cancel is invoked,
     * onCancel() will be called. It is expected to call setValue(),
     * setError() or setCanceled() as quickly as possible, but can do so
     * in an asynchronous way.
     */
    explicit Promise(boost::function<void (qi::Promise<T>)> cancelCallback,
        FutureCallbackType async = FutureCallbackType_Auto)
    {
      setup(cancelCallback, async);
      ++_f._p->_promiseCount;
    }

    Promise(const qi::Promise<T>& rhs)
    {
      _f = rhs._f;
      ++_f._p->_promiseCount;
    }

    ~Promise() {
      decRefcnt();
    }

    /** notify all future that a value has been set.
     *  throw if state != running
     *  If T is void \p value must be 0
     */
    void setValue(const ValueType &value) {
      _f._p->setValue(_f, value);
    }

    /** set the error, and notify all futures
     *  throw if state != running
     */
    void setError(const std::string &msg) {
      _f._p->setError(_f, msg);
    }

    /** set the cancel state, and notify all futures
     *  throw if state != running
     */
    void setCanceled() {
      _f._p->setCanceled(_f);
    }

    /** return true if cancel has been called on the promise (even if the
     * cancel callback did not run yet).
     */
    bool isCancelRequested() const {
      return _f._p->isCancelRequested();
    }

    /// Get a future linked to this promise. Can be called multiple times.
    Future<T> future() const { return _f; }

    /** Gives access to the underlying value for in-place modification.
     *  trigger() must be called after the value is written to trigger the
     *  promise.
     */
    ValueType& value() { return _f._p->_value;}
    /** Trigger the promise with the current value.
     */
    void trigger() { _f._p->set(_f);}

    /** Set a cancel callback. If the cancel is requested, calls this callback
     * immediately.
     * \throws std::exception if the promise was not created as a cancelable
     * promise.
     */
    void setOnCancel(boost::function<void (qi::Promise<T>)> cancelCallback)
    {
      if (!this->_f._p->isCancelable())
        throw std::runtime_error("Promise was not created as a cancelable one");
      qi::Future<T> fut = this->future();
      this->_f._p->setOnCancel(*this, cancelCallback);
    }

    Promise<T>& operator=(const Promise<T>& rhs)
    {
      if (_f._p == rhs._f._p)
        return *this;

      decRefcnt();
      _f = rhs._f;
      ++_f._p->_promiseCount;
      return *this;
    }

  protected:
    void setup(boost::function<void (qi::Promise<T>)> cancelCallback, FutureCallbackType async = FutureCallbackType_Auto)
    {
      this->_f._p->reportStart();
      this->_f._p->setOnCancel(*this, cancelCallback);
      this->_f._p->_async = async;
    }
    explicit Promise(Future<T>& f) : _f(f) {
      ++_f._p->_promiseCount;
    }
    template<typename> friend class ::qi::detail::FutureBaseTyped;
    Future<T> _f;

    template<typename R>
    friend void adaptFutureUnwrap(Future<AnyReference>& f, Promise<R>& p);
    template<typename FT, typename PT>
    friend void adaptFuture(const Future<FT>& f, Promise<PT>& p);
    template<typename FT, typename PT, typename CONV>
    friend void adaptFuture(const Future<FT>& f, Promise<PT>& p,
                            CONV converter);
    template<typename R>
    friend void adaptFuture(Future<AnyReference>& f, Promise<R>& p);

  private:
    void decRefcnt()
    {
      assert(*_f._p->_promiseCount > 0);
      // this is race-free because if we reach 0 it means that this is the last Promise pointing to a state and since it
      // is the last, no one could be trying to make a copy from it while destroying it. Also no one could be changing
      // the promise state (from running to finished or whatever) while destroying it.
      if (--_f._p->_promiseCount == 0 && _f.isRunning())
        _f._p->setBroken(_f);
    }
  };

  /**
   * \brief Helper function to return a future with the error set.
   * \param error the error message.
   */
  template <typename T>
  qi::Future<T> makeFutureError(const std::string& error);

  /// Helper function that does nothing on future cancelation
  template <typename T>
  void PromiseNoop(qi::Promise<T>&)
  {
  }

  /// Specialize this struct to provide conversion between future values
  template<typename FT, typename PT>
  struct FutureValueConverter
  {
    void operator()(const FT& vIn, PT& vOut) { vOut = vIn;}
  };

  /**
   * \brief Feed a promise from a generic future which may be unwrapped if it contains itself a future.
   */
  template<typename R>
  void adaptFutureUnwrap(Future<AnyReference>& f, Promise<R>& p);

  /**
   * \brief Feed a promise from a future of possibly different type.
   *
   * Will monitor \p f, and bounce its state to \p p.
   * Error and canceled state are bounced as is.
   * Valued state is bounced through FutureValueConverter<FT, PT>::convert()
   */
  template<typename FT, typename PT>
  void adaptFuture(const Future<FT>& f, Promise<PT>& p);

  template<typename R>
  void adaptFuture(Future<AnyReference>& f, Promise<R>& p);

  /// Similar to adaptFuture(f, p) but with a custom converter
  template<typename FT, typename PT, typename CONV>
  void adaptFuture(const Future<FT>& f, Promise<PT>& p, CONV converter);

  /// \copydoc qi::Future<T>::makeCanceler
  template <typename T>
  inline boost::function<void()> makeCanceler(Future<T>& future)
  {
    return future.makeCanceler();
  }
}

#ifdef _MSC_VER
#  pragma warning( pop )
#endif

#endif  // _QI_FUTURE_HPP_
