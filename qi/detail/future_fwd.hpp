#pragma once
/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/

#ifndef _QI_FUTURE_HPP_
# define _QI_FUTURE_HPP_

# include <stdexcept>
# include <type_traits>
# include <vector>

# include <ka/functional.hpp>
# include <ka/errorhandling.hpp>
# include <ka/macro.hpp>
# include <qi/api.hpp>
# include <qi/assert.hpp>
# include <qi/atomic.hpp>
# include <qi/config.hpp>
# include <qi/clock.hpp>
# include <qi/detail/mpl.hpp>
# include <qi/either.hpp>
# include <qi/log.hpp>
# include <qi/os.hpp>
# include <qi/tag.hpp>

# include <boost/shared_ptr.hpp>
# include <boost/make_shared.hpp>
# include <boost/function.hpp>
#define BOOST_BIND_GLOBAL_PLACEHOLDERS
# include <boost/bind.hpp>
#undef BOOST_BIND_GLOBAL_PLACEHOLDERS
# include <boost/thread/recursive_mutex.hpp>
# include <boost/exception/diagnostic_information.hpp>

KA_WARNING_PUSH()
KA_WARNING_DISABLE(4251, )
KA_WARNING_DISABLE(4275, )  //std::runtime_error: no dll interface

namespace qi {

  class AnyReference;

  namespace detail
  {
    template<typename T>
    struct FutureType
    {
      using type = T;
      using typecast = T;
    };

    struct FutureHasNoValue {};
    // Hold a void* for Future<void>
    template<>
    struct FutureType<void>
    {
      using type = void*;
      using typecast = FutureHasNoValue;
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
    void futureCancelAdapter(boost::weak_ptr<detail::FutureBaseTyped<FT> > wf);
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

  inline std::ostream& operator<<(std::ostream& o, FutureState x)
  {
    switch (x)
    {
    case FutureState_None: return o << "FutureState_None";
    case FutureState_Running: return o << "FutureState_Running";
    case FutureState_Canceled: return o << "FutureState_Canceled";
    case FutureState_FinishedWithError: return o << "FutureState_FinishedWithError";
    case FutureState_FinishedWithValue: return o << "FutureState_FinishedWithValue";
    }
    throw std::runtime_error("Unknown FutureState value: " + os::to_string(x));
  }


  enum FutureCallbackType {
    FutureCallbackType_Sync  = 0,
    FutureCallbackType_Async = 1,
    FutureCallbackType_Auto  = 2
  };

  enum FutureTimeout {
    FutureTimeout_Infinite = INT_MAX, // TODO: replace by numeric_limits<int>::max() when we get
                                      // rid of VS2013
    FutureTimeout_None     = 0,
  };

  namespace detail
  {
    struct VisitTimeout : boost::static_visitor<MilliSeconds::rep>
    {
      MilliSeconds::rep operator()(MilliSeconds x) const {
        return x.count();
      }
      MilliSeconds::rep operator()(Infinity) const {
        return static_cast<MilliSeconds::rep>(FutureTimeout_Infinite);
      }
    };
  } // namespace detail

  enum AdaptFutureOption {
    AdaptFutureOption_None = 0,
    AdaptFutureOption_ForwardCancel = 1,
  };

  using FutureUniqueId = void*;

  /** base exception raised for all future error.
   */
  class QI_API FutureException : public std::runtime_error {
  public:
    enum ExceptionState {
      ExceptionState_FutureTimeout,       ///< No result ready
      ExceptionState_FutureCanceled,      ///< The future has been canceled
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
    static_assert(!std::is_const<T>::value, "can't create a future of const");
  public:
    using ValueType = typename detail::FutureType<T>::type;
    using ValueTypeCast = typename detail::FutureType<T>::typecast;
    using TemplateValue = T;

  public:
    Future()
      : _p(boost::make_shared<detail::FutureBaseTyped<T> >())
    {
    }

    Future(const Future<T>& b)
      : _p(b._p)
    {}

    bool operator==(const Future<T> &other) const
    {
      return _p.get() == other._p.get();
    }

    Future<T>& operator=(const Future<T>& b)
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
    inline const ValueType& value(int msecs = FutureTimeout_Infinite) const
    {
      return _p->value(msecs);
    }

    inline const ValueType& value(Either<MilliSeconds, Infinity> timeout) const
    {
      return _p->value(static_cast<int>(visit(detail::VisitTimeout{}, timeout)));
    }

    /**
     * @brief Return by copy the value associated to a Future.
     */
    ValueType valueCopy(Either<MilliSeconds, Infinity> timeout = Infinity{}) const
    {
      // Copy happens because of the return type.
      return value(static_cast<int>(visit(detail::VisitTimeout{}, timeout)));
    }

    /**
     * @brief Return a shared pointer to the value associated to a Future.
     */
    boost::shared_ptr<const T> valueSharedPtr(Either<MilliSeconds, Infinity> timeout = Infinity{}) const
    {
      // Make the returned pointer increment the shared state reference count.
      return boost::shared_ptr<const T>(_p, &_p->value(visit(detail::VisitTimeout{}, timeout)));
    }

    /**
     * @brief Return the value associated to a Future.
     * @note Equivalent to `valueCopy(FutureTimeout_Infinite)`.
     */
    inline ValueType operator*() const
    {
      return valueCopy();
    }

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
     */
    void cancel() noexcept
    {
      _p->cancel(*this);
    }

    /** @return always true
     *
     * @deprecated since 2.5
     */
    QI_API_DEPRECATED_MSG("Method implementation removed, always returns 'true'")
    bool isCancelable() const
    {
      return true;
    }

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
     *
     * @deprecated since 2.5 use then()
     */
    template <typename R, typename AF>
    QI_API_DEPRECATED_MSG(Use 'then' instead)
    Future<R> thenR(FutureCallbackType type, AF&& func);

    /**
     * @brief Same as thenR(), but with type defaulted to FutureCallbackType_Auto.
     *
     * @deprecated since 2.5 use then()
     */
    template <typename R, typename AF>
    QI_API_DEPRECATED_MSG(Use 'then' instead)
    Future<R> thenR(AF&& func)
    {
      return thenRImpl<R>(FutureCallbackType_Auto, std::forward<AF>(func));
    }

    /**
     * @deprecated since 2.5 use then()
     */
    template <typename R, typename AF, typename Arg0, typename... Args>
    QI_API_DEPRECATED_MSG(Use 'then' instead)
    Future<R> thenR(AF&& func, Arg0&& arg0, Args&&... args);

    /**
     * @deprecated since 2.5 use then()
     */
    template <typename R, typename AF, typename Arg0, typename... Args>
    QI_API_DEPRECATED_MSG(Use 'then' instead)
    Future<R> thenR(FutureCallbackType type, AF&& func, Arg0&& arg0, Args&&... args);

    /**
     * @brief Execute a callback when the future is finished.
     *
     * The callback will receive this future as argument and all other arguments passed to this function.
     *
     * If the first argument bound to this function is a weak_ptr it will be locked. If it is a Trackable, the callback
     * won't be called after the object's destruction. If it is an Actor, the call will be stranded.
     *
     * @return a future that will receive the value returned by the callback or an error if the callback threw.
     */
    template <typename F>
    auto then(FutureCallbackType type, F&& func)
        -> Future<typename std::result_of<F(Future<T>)>::type>
    {
      return thenRImpl<typename std::result_of<F(Future<T>)>::type, F>(type, std::forward<F>(func));
    }

    /**
     * @brief Same as then(), but with type defaulted to FutureCallbackType_Auto.
     */
    template <typename AF>
    auto then(AF&& func)
        -> Future<typename std::result_of<AF(Future<T>)>::type>
    {
      return this->then(FutureCallbackType_Auto, std::forward<AF>(func));
    }

    /**
     * @brief Same as thenR(), but the callback is called only if this future finishes with a value.
     *
     * The callback will receive the value of this future, as opposed to this future itself.
     * In the case of Future<void>, the callback shall accept a void* parameter.
     *
     * If this future finishes with an error or a cancel, the callback will not be called and the returned future will
     * finish in the same state.
     *
     * @deprecated since 2.5 use andThen()
     */
    template <typename R, typename AF>
    QI_API_DEPRECATED_MSG(Use 'andThen' instead)
    Future<R> andThenR(FutureCallbackType type, AF&& func);

    /**
     * @brief Same as andThenR(), but with type defaulted to FutureCallbackType_Auto.
     *
     * @deprecated since 2.5 use andThen()
     */
    template <typename R, typename AF>
    QI_API_DEPRECATED_MSG(Use 'andThen' instead)
    Future<R> andThenR(AF&& func)
    {
      return andThenRImpl<R>(FutureCallbackType_Auto, std::forward<AF>(func));
    }

    /**
     * @brief Same as then(), but the callback is called only if this future finishes with a value.
     *
     * The callback will receive the value of this future, as opposed to this future itself.
     * In the case of Future<void>, the callback shall accept a void* parameter.
     *
     * If this future finishes with an error or a cancel, the callback will not be called and the returned future will
     * finish in the same state.
     */
    template <typename F>
    auto andThen(FutureCallbackType type, F&& func)
        -> Future<typename std::decay<typename std::result_of<F(ValueType)>::type>::type>
    {
      return this->andThenRImpl<typename std::decay<typename std::result_of<F(ValueType)>::type>::type, F>(type, std::forward<F>(func));
    }

    /**
     * @brief Same as andThen(), but with type defaulted to FutureCallbackType_Auto.
     */
    template <typename AF>
    auto andThen(AF&& func)
        -> Future<typename std::decay<typename std::result_of<AF(ValueType)>::type>::type>
    {
      return this->andThen(FutureCallbackType_Auto, std::forward<AF>(func));
    }

    /**
     * \brief Get a functor that will cancel the future.
     *
     * This functor will not keep the future alive, which is useful to avoid reference cycles. If the future does not
     * exist anymore, this is a no-op.
     *
     * \note This function should only be useful for bindings, you probably don't need it.
     */
    boost::function<void()> makeCanceler();

  public:
    using Connection = boost::function<void(Future<T>)>;

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
#define genCall(n, ATYPEDECL, ATYPES, ADECL, AUSE, comma)           \
    template <typename AF, typename ARG0 comma ATYPEDECL>           \
    QI_API_DEPRECATED_MSG(please use overload taking only a function and not argument instead)\
    void connect(const AF& fun, const ARG0& arg0 comma ADECL,       \
                 FutureCallbackType type = FutureCallbackType_Auto);
    QI_GEN(genCall)
#undef genCall
#endif

    /// @deprecated since 2.5 use the overload with Strand&
    QI_API_DEPRECATED_MSG(Use overload with 'Strand&' instead)
    void connectWithStrand(qi::Strand* strand,
        const boost::function<void(const Future<T>&)>& cb);
    void connectWithStrand(qi::Strand& strand,
        const boost::function<void(const Future<T>&)>& cb);

    // Our companion library libqitype requires a connect with same signature for all instantiations
    inline void _connect(const boost::function<void()>& s)
    {
      connect(boost::bind(s));
    }

    // KLUDGE: ExecutionContext uses the shared state of the future for gloomy reasons!
    friend class ExecutionContext;

  protected:
    /// An accessor to the shared state. TODO: remove it, it should not exist.
    boost::shared_ptr<detail::FutureBaseTyped<T> > impl() { return _p;}

    /// The constructor from the shared state.
    Future(boost::shared_ptr<detail::FutureBaseTyped<T> > p) :
      _p(p)
    {
      QI_ASSERT(_p);
    }

    // C4251 needs to have dll-interface to be used by clients of class 'qi::Future<T>'
    boost::shared_ptr< detail::FutureBaseTyped<T> > _p;
    friend class Promise<T>;
    friend class FutureSync<T>;

    template<typename R>
    friend void adaptFutureUnwrap(Future<AnyReference>& f, Promise<R>& p);
    template<typename FT, typename PT>
    friend void adaptFuture(const Future<FT>& f, Promise<PT>& p, AdaptFutureOption option);
    template<typename FT, typename PT, typename CONV>
    friend void adaptFuture(const Future<FT>& f, Promise<PT>& p,
                            CONV converter, AdaptFutureOption option);
    template<typename R>
    friend void adaptFuture(Future<AnyReference>& f, Promise<R>& p);

    template<typename FT>
    friend void detail::futureCancelAdapter(
        boost::weak_ptr<detail::FutureBaseTyped<FT> > wf);
    friend class detail::AddUnwrap<T>;

  private:
    friend class BoundObject;

    // Private forward impl to then
    template <typename R, typename AF>
    Future<R> andThenRImpl(FutureCallbackType type, AF&& func);

    // Private forward impl to then
    template <typename R, typename AF>
    Future<R> thenRImpl(FutureCallbackType type, AF&& func);

    // Nuke this when C++03 ends
    void setOnDestroyed(boost::function<void(ValueType)> cb)
    {
      _p->setOnDestroyed(cb);
    }

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
    using ValueType = typename Future<T>::ValueType;
    using ValueTypeCast = typename Future<T>::ValueTypeCast;
    using Connection = typename Future<T>::Connection;
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
      {
        static const auto logKnownError = [](const char* message)
        {
          qiLogWarning("qi.FutureSync")
            << "Error in future on destruction: '" << message
            << "' - continuing stack unwinding...";
        };

        try
        {
          _future.value();
        }
        catch(const std::exception& err)
        {
          logKnownError(err.what());
          throw;
        }
        catch(const boost::exception& err)
        {
          logKnownError(boost::diagnostic_information(err).c_str());
          throw;
        }
        catch(...)
        {
          qiLogWarning("qi.FutureSync")
            << "Unknown error in future on destruction - continuing stack unwinding...";
          throw;
        }
      }
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

    const ValueType& value(int msecs = FutureTimeout_Infinite) const   { _sync = false; return _future.value(msecs); }
    ValueType valueCopy(int msecs = FutureTimeout_Infinite) const      { return value(msecs); }
    boost::shared_ptr<const T> valueSharedPtr(int msecs = FutureTimeout_Infinite) const {
      _sync = false; return _future.valueSharedPtr(msecs);
    }
    FutureState wait(int msecs = FutureTimeout_Infinite) const         { _sync = false; return _future.wait(msecs); }
    FutureState wait(qi::Duration duration) const                      { _sync = false; return _future.wait(duration); }
    FutureState waitFor(qi::Duration duration) const                   { _sync = false; return _future.waitFor(duration); }
    FutureState wait(qi::SteadyClock::time_point timepoint) const      { _sync = false; return _future.wait(timepoint); }
    FutureState waitUntil(qi::SteadyClock::time_point timepoint) const { _sync = false; return _future.waitUntil(timepoint); }
    bool isValid() const                                               { _sync = false; return _future.isValid(); }
    bool isRunning() const                                             { _sync = false; return _future.isRunning(); }
    bool isFinished() const                                            { _sync = false; return _future.isFinished(); }
    bool isCanceled() const                                            { _sync = false; return _future.isCanceled(); }
    bool hasError(int msecs = FutureTimeout_Infinite) const            { _sync = false; return _future.hasError(msecs); }
    bool hasValue(int msecs = FutureTimeout_Infinite) const            { _sync = false; return _future.hasValue(msecs); }
    const std::string &error(int msecs = FutureTimeout_Infinite) const { _sync = false; return _future.error(msecs); }
    void cancel() noexcept                                             { _sync = false; _future.cancel(); }
    bool isCancelable() const                                          { _sync = false; return true; }
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
#define genCall(n, ATYPEDECL, ATYPES, ADECL, AUSE, comma)     \
    template<typename AF, typename ARG0 comma ATYPEDECL>      \
  QI_API_DEPRECATED_MSG(please use overload taking only a function and not argument instead)\
    void connect(const AF& fun, const ARG0& arg0 comma ADECL);
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
    using ValueType = typename detail::FutureType<T>::type;

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
    template <typename FUNC,
              typename std::enable_if<!std::is_same<
                                        typename std::decay<FUNC>::type,
                                        typename std::decay<qi::Promise<T> >::type
                                        >::value
                                      >::type* = nullptr>
    explicit Promise(FUNC&& cancelCallback,
        FutureCallbackType async = FutureCallbackType_Auto)
    {
      setup(std::forward<FUNC>(cancelCallback), async);
      ++_f._p->_promiseCount;
    }

    explicit Promise(boost::function<void (qi::Promise<T>)> cancelCallback,
        FutureCallbackType async = FutureCallbackType_Auto)
    {
      setup([cancelCallback](qi::Promise<T>& p){ cancelCallback(p); }, async);
      ++_f._p->_promiseCount;
    }

    Promise(const qi::Promise<T>& rhs)
      : _f(rhs._f)
    {
      ++_f._p->_promiseCount;
    }

    ~Promise() {
      decRefcnt();
    }

    /** notify all future that a value has been set.
     *  throw if state != running
     *  If T is void \p value must be nullptr
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
     */
    void setOnCancel(boost::function<void (qi::Promise<T>&)> cancelCallback)
    {
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
    void setup(boost::function<void (qi::Promise<T>&)> cancelCallback, FutureCallbackType async = FutureCallbackType_Auto)
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
    friend void adaptFuture(const Future<FT>& f, Promise<PT>& p, AdaptFutureOption option);
    template<typename FT, typename PT, typename CONV>
    friend void adaptFuture(const Future<FT>& f, Promise<PT>& p,
                            CONV converter, AdaptFutureOption option);
    template<typename R>
    friend void adaptFuture(Future<AnyReference>& f, Promise<R>& p);

  private:
    void decRefcnt()
    {
      QI_ASSERT(_f._p->_promiseCount.load() > 0);
      // this is race-free because if we reach 0 it means that this is the last Promise pointing to a state and since it
      // is the last, no one could be trying to make a copy from it while destroying it. Also no one could be changing
      // the promise state (from running to finished or whatever) while destroying it.
      if (--_f._p->_promiseCount == 0 && _f._p.use_count() > 1 && _f.isRunning())
        _f._p->setBroken(_f);
    }
  };

  namespace detail
  {
    class FutureBasePrivate;
    class QI_API FutureBase {
    public:
      FutureBase();
      ~FutureBase();

      FutureState wait(int msecs) const;
      FutureState wait(qi::Duration duration) const;
      FutureState wait(qi::SteadyClock::time_point timepoint) const;
      FutureState state() const;
      bool isRunning() const;
      bool isFinished() const;
      bool isCanceled() const;
      bool isCancelRequested() const;
      bool hasError(int msecs) const;
      bool hasValue(int msecs) const;
      const std::string &error(int msecs) const;
      void reportStart();

    protected:
      void reportValue();
      void reportError(const std::string &message);
      void requestCancel();
      void reportCanceled();
      boost::recursive_mutex& mutex();
      void notifyFinish();

    public:
      FutureBasePrivate *_p;
    };


    //common state shared between a Promise and multiple Futures
    template <typename T>
    class FutureBaseTyped : public FutureBase {
    public:
      using CancelCallback = boost::function<void(Promise<T>&)>;
      using ValueType = typename FutureType<T>::type;
      FutureBaseTyped();
      ~FutureBaseTyped();

      void cancel(qi::Future<T>& future) noexcept;

      /*
       * inplace api for promise
       */
      void set(qi::Future<T>& future);
      void setValue(qi::Future<T>& future, const ValueType &value);
      void setError(qi::Future<T>& future, const std::string &message);
      void setBroken(qi::Future<T>& future);
      void setCanceled(qi::Future<T>& future);

      void setOnCancel(const qi::Promise<T>& promise, CancelCallback onCancel);
      void setOnDestroyed(boost::function<void (ValueType)> f);

      void connect(qi::Future<T> future,
          const boost::function<void (qi::Future<T>)> &callback,
          FutureCallbackType type);

      const ValueType& value(int msecs) const;

    private:
      friend class Promise<T>;
      using CallbackType = boost::function<void(qi::Future<T>)>;
      struct Callback
      {
        CallbackType callback;
        FutureCallbackType callType;

        Callback(CallbackType callback, FutureCallbackType callType)
          : callback(callback)
          , callType(callType)
        {}
      };
      using Callbacks = std::vector<Callback>;
      Callbacks                _onResult;
      ValueType                _value;
      CancelCallback           _onCancel;
      boost::function<void (ValueType)> _onDestroyed;
      std::atomic<FutureCallbackType> _async;
      qi::Atomic<unsigned int> _promiseCount;

      template <typename F> // FunctionObject<R()> F (R unconstrained)
      void finish(qi::Future<T>& future, F&& finishTask);

      /// Take the callbacks set for handling the result and leave the member empty. Not thread-safe.
      Callbacks takeOutResultCallbacks();

      /// Clear the callback set for handling cancellation. Not thread-safe.
      void clearCancelCallback();

      static void executeCallbacks(bool defaultAsync, const Callbacks& callbacks, qi::Future<T>& future);
    };
  }

  /**
   * \brief Helper function to return a future with the error set.
   * \param error the error message.
   */
  template <typename T>
  qi::Future<T> makeFutureError(const std::string& error);

  /// Polymorphic function that converts an exception to a future in error.
  ///
  /// The future error message is the exception message.
  /// A transformation can be specified to change the error message before giving
  /// it to the future. This transformation is the identity by default.
  ///
  /// Example: Calling an asynchronous function and returning an error in case
  ///          of an exception.
  /// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  /// auto res = invoke_catch(
  ///   futureErrorFromException<int>,
  ///   []() -> Future<int> {
  ///     throw std::runtime_error("error");
  ///   }
  /// );
  /// assert(res.hasError());
  /// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  ///
  /// Transformation<std::string> F
  template<typename T, typename F = ka::id_transfo_t>
  auto futureErrorFromException(F errorMsgTransfo = {})
    // TODO: Remove this when we can use C++14
    -> decltype(ka::compose(ka::compose(makeFutureError<T>, errorMsgTransfo), ka::exception_message_t{}))
  {
    using ka::functional_ops::operator*; // Right-to-left function composition.
    return makeFutureError<T> * errorMsgTransfo * ka::exception_message_t{};
  }

  /// Helper function that does nothing on future cancelation
  ///
  /// @deprecated since 2.5:
  template <typename T>
  QI_API_DEPRECATED_MSG("you can remove this occurrence")
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
  void adaptFuture(const Future<FT>& f, Promise<PT>& p, AdaptFutureOption option = AdaptFutureOption_ForwardCancel);

  /// Similar to adaptFuture(f, p) but with a custom converter
  template<typename FT, typename PT, typename CONV>
  void adaptFuture(const Future<FT>& f, Promise<PT>& p, CONV converter,
      AdaptFutureOption option = AdaptFutureOption_ForwardCancel);

  /// \copydoc qi::Future<T>::makeCanceler
  template <typename T>
  inline boost::function<void()> makeCanceler(Future<T>& future)
  {
    return future.makeCanceler();
  }

  class AnyValue;

  template <typename T>
  Future<AnyValue> toAnyValueFuture(Future<T> future);

  struct SrcFuture;

  /// Polymorphic function object that creates a Future from a value, if provided.
  ///
  /// "unit" is traditionally the name of a transformation that sends a value into
  /// the monadic level, because it is a "unit" or "neutral element" for
  /// monadic composition.
  ///
  /// See the comment of `semiLift` for an explanation.
  ///
  /// Example:
  /// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  /// // `i` is an `int`
  /// UnitFuture unit;
  /// Future<int> fut0 = unit(i);
  /// Future<void> fut1 = unit();
  /// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  struct UnitFuture
  {
  // Regular:
    // Constructor added because of an error with Clang on Mac.
    // TODO: Remove this constructor when Clang is upgraded.
    UnitFuture()
    {
    }

    KA_GENERATE_FRIEND_REGULAR_OPS_0(UnitFuture)
  // PolymorphicFunction<Future<T> (T), Future<void> ()>:
    /// There is no constraint on T.
    template<typename T>
    Future<ka::Decay<T>> operator()(T&& t) const
    {
      return Future<ka::Decay<T>>{std::forward<T>(t)};
    }

    Future<void> operator()() const
    {
      return Future<void>{nullptr};
    }

  // Isomorphism:
    // TODO: Remove this when get rid of VS2013.
    using retract_type = SrcFuture;
  };

  /// Polymorphic function that gets (sources) the value of a `Future`.
  ///
  /// This function has an inverse: `UnitFuture` (i.e. composing with it in any
  /// order will produce a function doing nothing).
  ///
  /// See the concept `Isomorphism` in `concept.hpp` for a more formal explanation.
  struct SrcFuture
  {
  // Regular:
    KA_GENERATE_FRIEND_REGULAR_OPS_0(SrcFuture)

  // PolymorphicFunction<T (Future<T>)>:
    /// Note: The return type will be `void*` if `T` is `void`...
    /// There is no constraint on T.
    template<typename T>
    auto operator()(const Future<T>& x) const QI_NOEXCEPT_EXPR(*x) -> decltype(*x)
    {
      return *x;
    }

  // Isomorphism:
    // TODO: Remove this when get rid of VS2013.
    using retract_type = UnitFuture;

    /// Returns the retraction function, i.e. a function that undoes `SrcFuture`.
    /// As we model here the concept `Isomorphism`, it is also true that `SrcFuture`
    /// undoes `UnitFuture`.
    ///
    /// See the `Isomorphism` concept definition for more information.
    inline friend UnitFuture retract(SrcFuture)
    {
      return {};
    }
  };

  inline SrcFuture retract(UnitFuture)
  {
    return {};
  }

  /// Returns a future set with the given value. Passing no value results in a
  /// Future<void>.
  ///
  /// Example:
  /// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  /// Future<int> f = futurize(42);
  /// assert(f.hasValue() && f.value() == 42);
  /// Future<void> f2 = futurize();
  /// assert(f2.hasValue());
  /// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  ///
  /// TODO: Remove the trailing return type when get rid of C++11.
  template<typename... T>
  auto futurize(T&&... t)
    -> decltype(UnitFuture{}(ka::fwd<T>(t)...))
  {
    return UnitFuture{}(ka::fwd<T>(t)...);
  }

  /// Returns a new function similar to the given one except that it returns a
  /// future.
  ///
  /// This is a "semi-lifting" (see `semilift` for an explanation of the lifting notion)
  ///
  /// Example:
  /// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  /// // `heavyCompute` takes a `vector<int>` and returns an `int`.
  /// // `v` is a `vector<int>`.
  /// auto f = futurizeOutput(heavyCompute);
  /// Future<int> res = f(v);
  /// // ...
  /// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  ///
  /// TODO: Remove the trailing return type when get rid of C++11.
  ///
  /// Procedure Proc
  template<typename Proc>
  auto futurizeOutput(Proc&& p)
    -> decltype(ka::semilift(std::forward<Proc>(p), UnitFuture{}))
  {
    return ka::semilift(std::forward<Proc>(p), UnitFuture{});
  }
} // namespace qi

// KLUDGE: all definitions related to qi::bind or qi::Trackable are to be defined last,
// because qi::bind, found in trackable.hxx needs to know Future first!
// TODO: all functions below appear to be deprecated, they should be removed at some point.
#include <qi/trackable.hpp>

namespace qi
{
  template <typename T>
  template <typename R, typename AF, typename Arg0, typename... Args>
  Future<R> Future<T>::thenR(AF&& func, Arg0&& arg0, Args&&... args)
  {
    return thenRImpl<R>(
        FutureCallbackType_Auto,
        qi::bind(std::forward<AF>(func), std::forward<Arg0>(arg0), std::forward<Args>(args)...));
  }

  template <typename T>
  template <typename R, typename AF, typename Arg0, typename... Args>
  Future<R> Future<T>::thenR(FutureCallbackType type, AF&& func, Arg0&& arg0, Args&&... args)
  {
    return thenRImpl<R>(
        type,
        qi::bind(std::forward<AF>(func), arg0, std::forward<Args>(args)...));
  }

#ifndef DOXYGEN // skip those complicated macros in documentation
#define genCall(n, ATYPEDECL, ATYPES, ADECL, AUSE, comma)                   \
  template <typename T>                                                     \
  template <typename AF, typename ARG0 comma ATYPEDECL>                     \
  void Future<T>::connect(                                                  \
      const AF& fun, const ARG0& arg0 comma ADECL, FutureCallbackType type) \
  {                                                                         \
    this->then(type, qi::bind(fun, arg0 comma AUSE));                       \
  }
    QI_GEN(genCall)
#undef genCall

#define genCall(n, ATYPEDECL, ATYPES, ADECL, AUSE, comma)                           \
    template <typename T>                                                           \
    template <typename AF, typename ARG0 comma ATYPEDECL>                           \
    void FutureSync<T>::connect(const AF& fun, const ARG0& arg0 comma ADECL)        \
    {                                                                               \
      _sync = false;                                                                \
      connect(::qi::bind<void(FutureSync<T>)>(fun, arg0 comma AUSE));               \
    }
    QI_GEN(genCall)
#undef genCall
#endif
}

KA_WARNING_POP()

#endif  // _QI_FUTURE_HPP_
