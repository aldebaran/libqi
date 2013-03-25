#pragma once
/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/

#ifndef _QITYPE_FUTURE_HPP_
#define _QITYPE_FUTURE_HPP_

#include <qi/api.hpp>
#include <vector>
#include <qi/atomic.hpp>
#include <qi/config.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>
#include <boost/function.hpp>
#include <boost/bind.hpp>
#include <boost/thread/recursive_mutex.hpp>

#ifdef _MSC_VER
#  pragma warning( push )
#  pragma warning( disable: 4251 )
#  pragma warning( disable: 4275 ) //std::runtime_error: no dll interface
#endif

namespace qi {

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

  template <typename T> class FutureInterface;
  template <typename T> class Future;
  template <typename T> class FutureSync;
  template <typename T> class Promise;

  namespace detail {
    template <typename T> class FutureBaseTyped;
  }

  /** State of the future.
   */
  enum FutureState {
    FutureState_None,               /// Future is not tied to a promise
    FutureState_Running,            /// Operation pending
    FutureState_Canceled,           /// The future has been canceled
    FutureState_FinishedWithError,  /// The operation is finished with an error
    FutureState_FinishedWithValue,  /// The operation is finished with a value
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
      //No result ready
      ExceptionState_FutureTimeout,
      //The future has been canceled
      ExceptionState_FutureCanceled,
      //The future is not cancelable
      ExceptionState_FutureNotCancelable,
      //asked for error, but there is no error
      ExceptionState_FutureHasNoError,
      //real future error
      ExceptionState_FutureUserError,
      //when the promise is already set.
      ExceptionState_PromiseAlreadySet,
    };

    explicit FutureException(const ExceptionState &es, const std::string &str = std::string())
      : std::runtime_error(stateToString(es) + str)
    {}

    inline ExceptionState state() { return _state; }

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

  template <typename T>
  class Future {
  public:
    typedef typename FutureType<T>::type     ValueType;
    typedef typename FutureType<T>::typecast ValueTypeCast;

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

    explicit Future<T>(const ValueType& v)
    {
      Promise<T> promise;
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
    inline const ValueType &value(int msecs = FutureTimeout_Infinite) const { return _p->value(msecs); }

    /** same as value() with an infinite timeout.
     */
    inline operator const ValueTypeCast&() const { return _p->value(FutureTimeout_Infinite); }

    /** Wait for future to contain a value or an error
     @param msecs: Maximum time to wait in milliseconds, 0 means forever and -1 means return immediately.
     @return true if future contains a value or an error, false if timeout was reached
     */
    inline FutureState wait(int msecs = FutureTimeout_Infinite) const          { return _p->wait(msecs); }

    /**
     * @brief isFinished
     * @return true if finished
     * do not throw
     */
    inline bool isFinished() const                                             { return _p->isFinished(); }

    /**
     * @brief isRunning
     * @return
     * do not throw
     */
    inline bool isRunning() const                                              { return _p->isRunning(); }

    /**
     * @brief isCanceled
     * @return
     * do not throw
     */
    inline bool isCanceled() const                                             { return _p->isCanceled(); }

    /**
     * @brief hasError
     * @param msecs timeout
     * @return true if the future has an error.
     * throw in the following case:
     *   - timeout
     */
    inline bool hasError(int msecs = FutureTimeout_Infinite) const             { return _p->hasError(msecs); }
    /**
     * @brief hasValue
     * @param msecs timeout
     * @return
     * true if the future has a value.
     * throw in the following case:
     *   - timeout
     */
    inline bool hasValue(int msecs = FutureTimeout_Infinite) const             { return _p->hasValue(msecs); }

    /**
     * @brief error
     * @param msecs
     * @return the error
     * throw on timeout
     * throw if the future do not have an actual error.
     */
    inline const std::string &error(int msecs = FutureTimeout_Infinite) const  { return _p->error(msecs); }


    inline FutureSync<T> sync()
    {
      return FutureSync<T>(*this);
    };

    /** cancel() the asynchronous operation if possible
    * Exact effect is controlled by the cancel implementation, but it is
    * expected to set a value or an error to the Future as fast as possible.
    * Note that cancelation may be asynchronous.
    * @throw ExceptionState_FutureNotCancelable if isCanceleable() is false.
    */
    void cancel()
    {
      _p->cancel();
    }

    /** return true if the future can be canceled. This does not mean that cancel will succeed.
     */
    bool isCanceleable() const
    {
      return _p->isCanceleable();
    }
  public: //Signals
    typedef boost::function<void (Future<T>) > Connection;
    inline void connect(const Connection& s) { _p->connect(*this, s);}
    //qi::Signal<void (qi::Future<T>)> &onResult() { return _p->_onResult; }

  protected:
    // C4251 needs to have dll-interface to be used by clients of class 'qi::Future<T>'
    boost::shared_ptr< detail::FutureBaseTyped<T> > _p;
    friend class Promise<T>;
    friend class FutureSync<T>;
  };



  /** this class allow throwing on error and being synchronous
   *  when the future is not handled by the client.
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

    ~FutureSync()
    {
      if (_sync)
        _future.value();
    }

    operator Future<T>()
    {
      return async();
    }

    inline const ValueType &value() const                            { _sync = false; return _future.value(); }
    inline operator const typename Future<T>::ValueTypeCast&() const { _sync = false; return _future.value(); }
    inline FutureState wait(int msecs = FutureTimeout_Infinite) const { _sync = false; return _future.wait(msecs); }
    inline bool isRunning() const                                     { _sync = false; return _future.isRunning(); }
    inline bool isFinished() const                                    { _sync = false; return _future.isFinished(); }
    inline bool isCanceled() const                                    { _sync = false; return _future.isCanceled(); }
    inline bool hasError(int msecs = FutureTimeout_Infinite) const    { _sync = false; return _future.hasError(msecs); }
    inline bool hasValue(int msecs = FutureTimeout_Infinite) const    { _sync = false; return _future.hasValue(msecs); }

    inline const std::string &error() const                 { _sync = false; return _future.error(); }
    inline void cancel()                                    { _sync = false; _future.cancel(); }
    bool isCanceleable() const                              { _sync = false; return _future.isCanceleable(); }
    inline void connect(const Connection& s)                { _sync = false; _future.connect(s);}
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


  template <typename T>
  class Promise {
  public:
    typedef typename FutureType<T>::type ValueType;

    Promise() {
      _f._p->reportStart();
    }

    /** Create a canceleable promise. If Future<T>::cancel is invoked,
     * onCancel() will be called. It is expected to call setValue(),
     * setError() or setCanceled() as quickly as possible, but can do so
     * in an asynchronous way.
     */
    Promise(boost::function<void (qi::Promise<T>)> cancelCallback)
    {
      _f._p->reportStart();
      _f._p->setOnCancel(boost::bind<void>(cancelCallback, *this));
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

    /* reset the promise and the future */
    void reset() {
      _f._p->reset();
    }

    /* get the future from the promise, you can call this function many times. */
    Future<T> future() { return _f; }

    /** Gives access to the underlying value for in-place modification.
     *  trigger() must be called after the value is written to trigger the promise.
    */
    ValueType& value() { return _f._p->_value;}
    /** Trigger the promise with the current value.
    */
    void trigger() { _f._p->set(_f);}
  protected:
    Future<T> _f;
  };

  template<typename T>
  class FutureBarrier {
  public:
    /// FutureBarrier constructor taking no argument.
    FutureBarrier()
      : _closed(false)
      , _count(0)
      , _futures()
      , _promise()
    {}

    /// Adds the future to the barrier.
    bool addFuture(qi::Future<T> fut) {
      // Can't add future from closed qi::FutureBarrier.
      if (this->_closed)
        return false;

      ++(this->_count);
      fut.connect(boost::bind<void>(&FutureBarrier::onFutureFinish, this));
      this->_futures.push_back(fut);
      return true;
    }

    /// Gets the future result for the barrier.
    Future< std::vector< Future<T> > > future() {
      this->close();
      return this->_promise.future();
    }

  protected:
    bool _closed;
    Atomic<int> _count;
    std::vector< Future<T> > _futures;
    Promise< std::vector< Future<T> > > _promise;

  private:
    void onFutureFinish() {
      if (--(this->_count) == 0 && this->_closed) {
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

  template <typename T>
  qi::Future<T> makeFutureError(const std::string &value);

  /// Helper function to wait on a vector of futures.
  template <typename T>
  void waitForAll(std::vector< Future<T> >& vect);

  /// Helper function to wait for the first valid future.
  template <typename T>
  qi::FutureSync< qi::Future<T> > waitForFirst(std::vector< Future<T> >& vect);

  /// Specialize this struct to provide conversion between future values
  template<typename FT, typename PT> struct FutureValueConverter
  {
    void operator()(const FT& vIn, PT& vOut) { vOut = vIn;}
  };

  /** Feed a promise from a future of possibly different type.
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

#include <qi/details/future.hxx>

#endif  // _QITYPE_FUTURE_HPP_
