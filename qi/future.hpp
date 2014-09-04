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
# include <qi/actor.hpp>
# include <qi/detail/mpl.hpp>

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
  }

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
    FutureCallbackType_Async = 1
  };

  enum FutureTimeout {
    FutureTimeout_Infinite = ((int) 0x7fffffff),
    FutureTimeout_None     = 0,
  };

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
  class Future {
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

    /** Construct a Future that already contains a value.
     */
    explicit Future<T>(const ValueType& v, FutureCallbackType async = FutureCallbackType_Async)
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

    /** Wait for future to contain a value or an error
        @param timepoint Time until which we can wait
        @return a FutureState corresponding to the state of the future.
    */
    inline FutureState wait(qi::SteadyClock::time_point timepoint) const
    { return _p->wait(timepoint); }

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

  public: //Signals
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
                        FutureCallbackType type = FutureCallbackType_Async)
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
                 FutureCallbackType type = FutureCallbackType_Async);
#else
#define genCall(n, ATYPEDECL, ATYPES, ADECL, AUSE, comma)                 \
  template <typename AF, typename ARG0 comma ATYPEDECL>                   \
  inline void connect(const AF& fun, const ARG0& arg0 comma ADECL,        \
                      FutureCallbackType type = FutureCallbackType_Async) \
  {                                                                       \
    connectMaybeActor<ARG0, 0>(                                           \
        arg0, ::qi::bind<void(Future<T>)>(fun, arg0 comma AUSE), type);   \
  }
    QI_GEN(genCall)
#undef genCall
#endif

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

    template<typename FT, typename PT>
    friend void adaptFuture(const Future<FT>& f, Promise<PT>& p);
    template<typename FT, typename PT, typename CONV>
    friend void adaptFuture(const Future<FT>& f, Promise<PT>& p,
                            CONV converter);
    template<typename FT>
    friend void detail::futureCancelAdapter(
        boost::weak_ptr<detail::FutureBaseTyped<FT> > wf);

  private:
    static void binder(
        const boost::function<void(const boost::function<void()>&)>& poster,
        const boost::function<void(Future<T>)>& callback, Future<T> fut)
    {
      return poster(boost::bind(callback, fut));
    }

    template <
        typename ARG0,
        typename boost::enable_if<
            boost::is_base_of<Actor, typename detail::Unwrap<ARG0>::type>,
            int>::type>
    inline void connectMaybeActor(const ARG0& arg0,
                                  const boost::function<void(Future<T>)>& cb,
                                  FutureCallbackType type)
    {
      _p->connect(
          *this,
          qi::trackWithFallback(
            boost::function<void()>(),
            boost::function<void(const Future<T>&)>(
              boost::bind(
                  &Future<T>::binder,
                  boost::function<void(const boost::function<void()>&)>(
                      boost::bind(
                          &qi::Strand::post,
                          detail::Unwrap<ARG0>::unwrap(arg0)->strand(), _1)),
                  cb, _1)),
              arg0),
          FutureCallbackType_Sync);
    }
    template <
        typename ARG0,
        typename boost::disable_if<
            boost::is_base_of<Actor, typename detail::Unwrap<ARG0>::type>,
            int>::type>
    inline void connectMaybeActor(const ARG0& arg0,
                                  const boost::function<void(Future<T>)>& cb,
                                  FutureCallbackType type)
    {
      _p->connect(*this, cb, type);
    }
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

    const ValueType &value(int msecs = FutureTimeout_Infinite) const   { _sync = false; return _future.value(msecs); }
    operator const typename Future<T>::ValueTypeCast&() const          { _sync = false; return _future.value(); }
    FutureState wait(int msecs = FutureTimeout_Infinite) const         { _sync = false; return _future.wait(msecs); }
    FutureState wait(qi::Duration duration) const                      { _sync = false; return _future.wait(duration); }
    FutureState wait(qi::SteadyClock::time_point timepoint) const      { _sync = false; return _future.wait(timepoint); }
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
    explicit Promise(FutureCallbackType async = FutureCallbackType_Async) {
      _f._p->reportStart();
      _f._p->_async = async;
    }

    /** Create a canceleable promise. If Future<T>::cancel is invoked,
     * onCancel() will be called. It is expected to call setValue(),
     * setError() or setCanceled() as quickly as possible, but can do so
     * in an asynchronous way.
     */
    explicit Promise(boost::function<void (qi::Promise<T>)> cancelCallback,
        FutureCallbackType async = FutureCallbackType_Async)
    {
      setup(cancelCallback, async);
    }

    /** notify all future that a value has been set.
     *  throw if state != running
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
    bool isCancelRequested() {
      return _f._p->isCancelRequested();
    }

    /** reset the promise and the future
     * @deprecated reseting a promise removes connect() guaranties
     */
    void reset() {
      _f._p->reset();
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

  protected:
    void setup(boost::function<void (qi::Promise<T>)> cancelCallback, FutureCallbackType async = FutureCallbackType_Async)
    {
      this->_f._p->reportStart();
      this->_f._p->setOnCancel(cancelCallback);
      this->_f._p->_async = async;
    }
    explicit Promise(Future<T>& f) : _f(f) {}
    template<typename> friend class ::qi::detail::FutureBaseTyped;
    Future<T> _f;

    template<typename FT, typename PT>
    friend void adaptFuture(const Future<FT>& f, Promise<PT>& p);
    template<typename FT, typename PT, typename CONV>
    friend void adaptFuture(const Future<FT>& f, Promise<PT>& p,
                            CONV converter);
  };

  /**
   * \class qi::FutureBarrier
   * \includename{qi/future.hpp}
   * \brief This class helps waiting on multiple futures at the same point.
   *
   * \verbatim
   * This class helps waiting on multiple futures at the same point. If you want
   * to make several calls in a function and wait for all results at some point.
   * (:cpp:func:`qi::waitForAll(std::vector<Future<T>>&)` and
   * :cpp:func:`qi::waitForFirst(std::vector<Future<T>>&)` may help you
   * for simple cases).
   *
   * :cpp:class:`qi::FutureBarrier` is used like a builder. You must give it the
   * futures with :cpp:func:`qi::FutureBarrier<T>::addFuture(qi::Future<T>)`. On
   * first call to :cpp:func:`qi::FutureBarrier<T>::future()`, barrier will be closed
   * and won't except any more future. :cpp:func:`qi::FutureBarrier<T>::future()`
   * returns the vector of all the futures given to the barrier.
   *
   * With this code, you can easily write asynchronous map code.
   *
   * Simple example: waitForAll
   * **************************
   *
   * .. code-block:: cpp
   *
   *     void waitForAll(std::vector< Future<int> >& vect) {
   *         qi::FutureBarrier<int> barrier;
   *         std::vector< Future<int> >::iterator it;
   *
   *         for (it = vect.begin(); it != vect.end(); ++it) {
   *             barrier.addFuture(*it);
   *         }
   *         barrier.future().wait();
   *
   *         // [1]: Do something here with all the results.
   *     }
   *
   * This function is the simplest one you can write with FutureBarrier. Lets say
   * you have a vector of calls and you eant to wait on all of them before
   * executing something, this is typically the kind of code you would write.
   *
   * .. note::
   *
   *     This function is already provided with the API in ``qi`` namespace,
   *     as a templated implementation. Don't recode it.
   *
   * Complete example
   * ****************
   *
   * .. code-block:: cpp
   *
   *     qi::Future<int> returnAsynchronouslyNumber(int number);
   *     void mult42(qi::Promise<int> prom, qi::Future<int> number);
   *     void sumList(qi::Promise<int> prom,
   *                  qi::Future< std::vector< qi::Future<int> > > fut);
   *
   *     qi::Future<int> sum42ProductTable() {
   *         qi::FutureBarrier barrier;
   *
   *         // [1]:
   *         for (int it = 0; it < 10; ++it) {
   *             // [1.1]:
   *             qi::Future<int> fut = returnAsynchronouslyNumber(it);
   *
   *             qi::Promise<int> prom;
   *             fut.connect(boost::bind(&mult42, prom, _1));
   *             barrier.addFuture(prom.future());
   *
   *             // [1.2]
   *         }
   *
   *         // The following line would hang until the results are ready:
   *         // std::vector< qi::Future<int> > values = barrier.future();
   *         // Vector would then contain promises results, when they are all
   *         // ready, so [0, 42, 84, 126, 168, 210, 252, 294, 336, 378]
   *
   *         // [2]:
   *         qi::Promise<int> res;
   *         barrier.future().connect(boost::bind(&sumList, res, _1));
   *         return res.future();
   *     }
   *
   * This is a complete example of how to do a map. This is the standart usage
   * of futures but within a loop. If you look at *[1.1]* part, you have an
   * asynchronous call to returnAsynchronouslyNumber function, a treatment of this
   * result with function *mult42* to which we give a promise and we use the future
   * of the promise. Instead of returning it, we give it to the FutureBarrier.
   *
   * This is due to the fact that *[2]* needs *[1]* to be completely executed
   * before executing, including the callback *mult42*. FutureBarrier makes sure of
   * this synchronisation.
   *
   * Since it is returning a :cpp:class:`qi::Future`. You can connect to it using
   * the standard pattern again and execute a callback (*sunList*) when all the
   * results has been acquired. This what *[2]* does.
   *
   * To summaries, this function will: use an asynchronous call to the function
   * identity (just to have an asynchronous call), multiply all the results with
   * the number 42, and the sum the complete vector, to return it.
   *
   * .. note::
   *
   *     If you add any callback to the future after the call to
   *     :cpp:func:`qi::FutureBarrier<T>::addFuture(qi::Future<T>)`,
   *     replacing *[1.2]*, the callback on barrier's future will be executed
   *     asynchronously with it. If you are not sure, always call
   *     :cpp:func:`qi::FutureBarrier<T>::addFuture(qi::Future<T>)` in last.
   * \endverbatim
   */
  template<typename T>
  class FutureBarrier {
  public:
    /// FutureBarrier constructor taking no argument.
    FutureBarrier(FutureCallbackType async = FutureCallbackType_Async)
      : _closed(0)
      , _count(0)
      , _futures()
      , _promise(async)
    {}

    /**
     * \brief Adds the future to the barrier.
     * \return Whether the future could be added.
     *
     * \verbatim
     * This adds the future to the barrier. It means barrier's future won't return
     * until this one returns. It will also be added to the resulting vector.
     *
     * When :cpp:func:`qi::FutureBarrier::future()` has been called, this function
     * will have no effect and return false.
     * \endverbatim
     */
    bool addFuture(qi::Future<T> fut) {
      // Can't add future from closed qi::FutureBarrier.
      if (*this->_closed)
        return false;

      ++(this->_count);
      fut.connect(boost::bind<void>(&FutureBarrier::onFutureFinish, this));
      this->_futures.push_back(fut);
      return true;
    }


    /**
     * \brief Gets the future result for the barrier.
     *
     * \verbatim
     * Returns a future containing the vector of all the futures given to the barrier.
     *
     * .. warning::
     *
     *     Once called, you will not be able to add a new future to the barrier.
     * \endverbatim
     */
    Future< std::vector< Future<T> > > future() {
      this->close();
      return this->_promise.future();
    }

  protected:
    Atomic<int> _closed;
    Atomic<int> _count;
    std::vector< Future<T> > _futures;
    Promise< std::vector< Future<T> > > _promise;

  private:
    void onFutureFinish() {
      if (--(this->_count) == 0 && *this->_closed) {
        this->_promise.setValue(this->_futures);
      }
    }

    void close() {
      this->_closed = true;
      if (*(this->_count) == 0) {
        this->_promise.setValue(this->_futures);
      }
    }
  };

  /**
   * \brief Helper function to return a future with the error set.
   * \param error the error message.
   */
  template <typename T>
  qi::Future<T> makeFutureError(const std::string& error);

  /**
   * \brief Helper function to wait on a vector of futures.
   * \param vect The vector of futures to wait on.
   *
   * \verbatim
   * This function will wait on all the futures of the given vector and return
   * when they have all been set, either with an error or a valid value.
   * \endverbatim
   */
  template <typename T>
  void waitForAll(std::vector< Future<T> >& vect);

  /**
   * \brief Helper function to wait for the first valid future.
   * \param vect The vector of futures to wait on.
   * \return The first valid future, or an error.
   *
   * \verbatim
   * This function will wait on all the futures of the vector. It returns the
   * first valid future that returns. If no future is valid, a future set with
   * an error is returned.
   * \endverbatim
   */
  template <typename T>
  qi::FutureSync< qi::Future<T> > waitForFirst(std::vector< Future<T> >& vect);

  /// Helper function that does nothing on future cancellation
  template <typename T>
  void PromiseNoop(const qi::Promise<T>&)
  {
  }

  /// Specialize this struct to provide conversion between future values
  template<typename FT, typename PT>
  struct FutureValueConverter
  {
    void operator()(const FT& vIn, PT& vOut) { vOut = vIn;}
  };

  /**
   * \brief Feed a promise from a future of possibly different type.
   *
   * Will monitor \p f, and bounce its state to \p p.
   * Error and canceled state are bounced as is.
   * Valued state is bounced through FutureValueConverter<FT, PT>::convert()
   */
  template<typename FT, typename PT>
  void adaptFuture(const Future<FT>& f, Promise<PT>& p);

  /// Similar to adaptFuture(f, p) but with a custom converter
  template<typename FT, typename PT, typename CONV>
  void adaptFuture(const Future<FT>& f, Promise<PT>& p, CONV converter);
}

#ifdef _MSC_VER
#  pragma warning( pop )
#endif

#include <qi/detail/future.hxx>

#endif  // _QI_FUTURE_HPP_
