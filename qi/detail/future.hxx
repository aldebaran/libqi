#pragma once
/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/

#ifndef _QI_DETAIL_FUTURE_HXX_
#define _QI_DETAIL_FUTURE_HXX_

#include <vector>
#include <utility> // pair
#define BOOST_BIND_GLOBAL_PLACEHOLDERS
#include <boost/bind.hpp>
#undef BOOST_BIND_GLOBAL_PLACEHOLDERS
#include <ka/errorhandling.hpp>
#include <qi/eventloop.hpp>
#include <qi/log.hpp>
#include <qi/strand.hpp>
#include <qi/type/detail/futureadapter.hpp>

namespace qi {

namespace detail {

  template <typename R, typename F>
  void setPromiseFromCall(Promise<R>& p, F&& f)
  {
    static_assert(
          std::is_convertible<typename std::result_of<F()>::type, R>::value,
          "function return type is incompatible with promise");
    p.setValue(f());
  }

  template <typename F>
  void setPromiseFromCall(Promise<void>& p, F&& f)
  {
    f();
    p.setValue(nullptr);
  }

  template <typename R, typename F>
  void setPromiseFromCallWithExceptionSupport(Promise<R>& p, F&& f)
  {
    try
    {
      setPromiseFromCall(p, std::forward<F>(f));
    }
    catch (std::exception& e)
    {
      p.setError(e.what());
    }
    catch (...)
    {
      p.setError("unknown exception");
    }
  }
} // namespace detail

  template <typename T>
  template <typename R, typename AF>
  inline Future<R> Future<T>::thenR(FutureCallbackType type, AF&& func)
  {
    return thenRImpl<R>(type, std::forward<AF>(func));
  }

  template <typename T>
  template <typename R, typename F>
  inline Future<R> Future<T>::thenRImpl(FutureCallbackType callbackType, F&& continuation)
  {
    boost::weak_ptr<detail::FutureBaseTyped<T> > weakp(_p);
    qi::Promise<R> promise([weakp](const qi::Promise<R>&){
          if (auto futureb = weakp.lock())
            Future<T>(futureb).cancel();
        });

    using isAsync = detail::IsAsyncBind<F>;
    if (callbackType == FutureCallbackType_Auto && isAsync::value)
      callbackType = FutureCallbackType_Sync;

    auto adaptedContinuation = [promise, continuation](const Future<T>& future) mutable
    {
      detail::setPromiseFromCallWithExceptionSupport(
            promise, [&continuation, &future]() mutable { return continuation(future); });
    };

    _p->connect(*this, adaptedContinuation, callbackType);
    return promise.future();
  }

  template <typename T>
  template <typename R, typename AF>
  inline Future<R> Future<T>::andThenR(FutureCallbackType type, AF&& func)
  {
    return andThenRImpl<R>(type, std::forward(func));
  }

  template <typename T>
  template <typename R, typename F>
  inline Future<R> Future<T>::andThenRImpl(FutureCallbackType callbackType, F&& continuation)
  {
    boost::weak_ptr<detail::FutureBaseTyped<T> > weakp(_p);
    qi::Promise<R> promise([weakp](const qi::Promise<R>&){
          if (auto futureb = weakp.lock())
            Future<T>(futureb).cancel();
        });

    using isAsync = detail::IsAsyncBind<F>;
    if (callbackType == FutureCallbackType_Auto && isAsync::value)
      callbackType = FutureCallbackType_Sync;

    auto adaptedContinuation = [promise, continuation](const Future<T>& future) mutable
    {
      if (future.isCanceled())
        promise.setCanceled();
      else if (future.hasError())
        promise.setError(future.error());
      else if (promise.isCancelRequested())
        promise.setCanceled();
      else
        detail::setPromiseFromCallWithExceptionSupport(
              promise, [&continuation, &future]() mutable { return continuation(future.value()); });
    };

    _p->connect(*this, adaptedContinuation, callbackType);
    return promise.future();
  }

  template <typename T>
  void Future<T>::connectWithStrand(qi::Strand* strand,
      const boost::function<void(const Future<T>&)>& cb)
  {
    connectWithStrand(*strand, cb);
  }

  template <typename T>
  void Future<T>::connectWithStrand(qi::Strand& strand,
      const boost::function<void(const Future<T>&)>& cb)
  {
    _p->connect(
        *this,
        strand.schedulerFor(cb),
        FutureCallbackType_Sync);
  }

  template <typename T>
  void Future<T>::_weakCancelCb(const boost::weak_ptr<detail::FutureBaseTyped<T> >& wfuture)
  {
    if (boost::shared_ptr<detail::FutureBaseTyped<T> > fbt = wfuture.lock())
    {
      Future<T> future(fbt);
      future.cancel();
    }
  }

  template <typename T>
  boost::function<void()> Future<T>::makeCanceler()
  {
    return boost::bind(&Future<T>::_weakCancelCb, boost::weak_ptr<detail::FutureBaseTyped<T> >(_p));
  }

  namespace detail {

    template <typename T>
    FutureBaseTyped<T>::FutureBaseTyped()
      : _value()
      , _async(FutureCallbackType_Auto)
    {
    }

    template <typename T>
    FutureBaseTyped<T>::~FutureBaseTyped()
    {
      boost::recursive_mutex::scoped_lock lock(mutex());
      if (_onDestroyed && state() == FutureState_FinishedWithValue)
        _onDestroyed(_value);
    }

    template <typename T>
    void FutureBaseTyped<T>::cancel(qi::Future<T>& future) noexcept
    {
      // optional necessary in case of error, see ka::invoke_catch() usage below
      auto cancelImpl = [&]() -> boost::optional<std::string> {
        CancelCallback onCancel;
        {
          boost::recursive_mutex::scoped_lock lock(mutex());
          if (isFinished())
            return {};
          requestCancel();
          std::swap(onCancel, _onCancel);
        }
        if (onCancel)
        {
          qi::Promise<T> prom(future);
          onCancel(prom);
        }

        return {};
      };

      if (auto const maybeError = ka::invoke_catch(ka::exception_message_t{}, cancelImpl))
      {
        qiLogError("qi.future") << "Future/Promise cancel handler threw an exception: " << maybeError.value();
      }
    }

    template <typename T>
    void FutureBaseTyped<T>::setOnCancel(const qi::Promise<T>& promise, CancelCallback onCancel)
    {
      bool doCancel = false;
      {
        boost::recursive_mutex::scoped_lock lock(mutex());
        _onCancel = onCancel;
        doCancel = isCancelRequested();
      }
      qi::Future<T> fut = promise.future();
      if (doCancel)
        cancel(fut);
    }

    template <typename T>
    void FutureBaseTyped<T>::executeCallbacks(bool defaultAsync, const Callbacks& callbacks, qi::Future<T>& future)
    {
      for (const auto& callback : callbacks)
      {
        const bool async = [&]{
          if (callback.callType != FutureCallbackType_Auto)
            return callback.callType != FutureCallbackType_Sync;
          else
            return defaultAsync != FutureCallbackType_Sync;
        }();

        if (async)
          getEventLoop()->post(boost::bind(callback.callback, future));
        else
          try
          {
            callback.callback(future);
          }
          catch (const qi::PointerLockException&)
          { // do nothing
          }
          catch (const std::exception& e)
          {
            qiLogError("qi.future") << "Exception caught in future callback " << e.what();
          }
          catch (...)
          {
            qiLogError("qi.future") << "Unknown exception caught in future callback";
          }
      }
    }

    template <typename T>
    template <typename F> // FunctionObject<R()> F (R unconstrained)
    void FutureBaseTyped<T>::finish(qi::Future<T>& future, F&& finishTask)
    {
      bool async;
      Callbacks onResult;
      {
        // report-ready + onResult() must be Atomic to avoid
        // missing callbacks/double calls in case connect() is invoked at
        // the same time
        boost::recursive_mutex::scoped_lock lock(mutex());
        if (!isRunning())
          throw FutureException(FutureException::ExceptionState_PromiseAlreadySet);
        finishTask();

        async = (_async != FutureCallbackType_Sync ? true : false);
        onResult = takeOutResultCallbacks();
        clearCancelCallback();

        // wake the waiting threads up
        notifyFinish();
      }
      // call the callbacks without the mutex
      executeCallbacks(async, onResult, future);
    }

    template <typename T>
    void FutureBaseTyped<T>::setValue(qi::Future<T>& future, const ValueType& value)
    {
      finish(future, [this, &value] {
        _value = value;
        reportValue();
      });
    }

    template <typename T>
    void FutureBaseTyped<T>::set(qi::Future<T>& future)
    {
      finish(future, [this] {
        reportValue();
      });
    }

    template <typename T>
    void FutureBaseTyped<T>::setError(qi::Future<T>& future, const std::string& message)
    {
      finish(future, [this, &message] {
        reportError(message);
      });
    }

    template <typename T>
    void FutureBaseTyped<T>::setBroken(qi::Future<T>& future)
    {
      finish(future, [this] {
        reportError("Promise broken (all promises are destroyed)");
      });
    }

    template <typename T>
    void FutureBaseTyped<T>::setCanceled(qi::Future<T>& future)
    {
      finish(future, [this] {
        reportCanceled();
      });
    }

    template <typename T>
    void FutureBaseTyped<T>::setOnDestroyed(boost::function<void(ValueType)> f)
    {
      boost::recursive_mutex::scoped_lock lock(mutex());
      _onDestroyed = f;
    }

    template <typename T>
    void FutureBaseTyped<T>::connect(qi::Future<T> future,
                                  const boost::function<void(qi::Future<T>)>& callback,
                                  FutureCallbackType type)
    {
      if (state() == FutureState_None)
        throw FutureException(FutureException::ExceptionState_FutureInvalid);

      bool ready;
      {
        boost::recursive_mutex::scoped_lock lock(mutex());
        ready = isFinished();
        if (!ready)
          _onResult.push_back(Callback(callback, type));
      }

      // result already ready, notify the callback
      if (ready)
      {
        const bool async = [&]{
          if (type != FutureCallbackType_Auto)
            return type != FutureCallbackType_Sync;
          else
            return _async != FutureCallbackType_Sync;
        }();

        auto soCalledEventLoop = getEventLoop();
        if (async && soCalledEventLoop)
        { // if no event loop was found (for example when exiting), force sync callbacks
          soCalledEventLoop->post(boost::bind(callback, future));
        }
        else
        {
          try
          {
            callback(future);
          }
          catch (const ::qi::PointerLockException&)
          { /*do nothing*/
          }
        }
      }
    }

    template <typename T>
    const typename FutureBaseTyped<T>::ValueType& FutureBaseTyped<T>::value(int msecs) const
    {
      FutureState state = wait(msecs);
      if (state == FutureState_None)
        throw FutureException(FutureException::ExceptionState_FutureInvalid);
      if (state == FutureState_Running)
        throw FutureException(FutureException::ExceptionState_FutureTimeout);
      if (state == FutureState_Canceled)
        throw FutureException(FutureException::ExceptionState_FutureCanceled);
      if (state == FutureState_FinishedWithError)
        throw FutureUserException(error(FutureTimeout_None));
      return _value;
    }

    template <typename T>
    auto FutureBaseTyped<T>::takeOutResultCallbacks() -> Callbacks
    {
      Callbacks onResult;
      using std::swap;
      swap(onResult, _onResult);
      return onResult;
    }

    template <typename T>
    void FutureBaseTyped<T>::clearCancelCallback()
    {
      _onCancel.clear();
    }

    template <typename T>
    void waitForFirstHelper(qi::Promise< qi::Future<T> >& prom,
                            qi::Future<T>& fut,
                            qi::Atomic<int>* count) {
      if (!prom.future().isFinished() && !fut.hasError())
      {
        // An other future can trigger at the same time.
        // Don't bother to lock, just catch the FutureAlreadySet exception
        try
        {
          prom.setValue(fut);
        }
        catch(const FutureException&)
        {}
      }
      if (! --*count)
      {
        // I'm the last
        if (!prom.future().isFinished())
        {
          // same 'race' as above. between two setError, not between a value and
          // an error.
          try
          {
            prom.setValue(makeFutureError<T>("No future returned successfully."));
          }
          catch(const FutureException&)
          {}
        }
        delete count;
      }
    }

    template <typename T>
    class AddUnwrap<Future<T> >
    {
    public:
      Future<T> unwrap()
      {
        Future<Future<T> >* self = static_cast<Future<Future<T> >*>(this);

        namespace ph = std::placeholders;
        Promise<T> promise(boost::bind(&AddUnwrap<Future<T> >::_cancel, ph::_1,
              boost::weak_ptr<FutureBaseTyped<Future<T> > >(self->_p)));

        self->connect(
            boost::bind(&AddUnwrap<Future<T> >::_forward, ph::_1, promise),
            FutureCallbackType_Sync);

        return promise.future();
      }

    private:
      static void _forward(const Future<Future<T> >& future,
          Promise<T>& promise)
      {
        if (future.isCanceled())
          promise.setCanceled();
        else if (future.hasError())
          promise.setError(future.error());
        else
          adaptFuture(future.value(), promise);
      }

      static void _cancel(Promise<T>& /*promise*/,
          const boost::weak_ptr<FutureBaseTyped<Future<T> > >& wfuture)
      {
        if (boost::shared_ptr<FutureBaseTyped<Future<T> > > fbt =
            wfuture.lock())
        {
          Future<Future<T> > future(fbt);
          future.cancel();
        }
      }
    };

  } // namespace detail

  template <typename T>
  qi::Future<T> makeFutureError(const std::string &error) {
    qi::Promise<T> prom;
    prom.setError(error);
    return prom.future();
  }

  namespace detail
  {
    template<typename FT, typename PT, typename CONV>
    void futureAdapter(const Future<FT>& f, Promise<PT> p, CONV converter)
    {
      if (f.hasError())
        p.setError(f.error());
      else if (f.isCanceled())
        p.setCanceled();
      else
      {
        try {
          converter(f.value(), p.value());
        }
        catch (const std::exception& e)
        {
          p.setError(std::string("futureAdapter conversion error: ") + e.what());
          return;
        }
        p.trigger();
      }
    }

    template<typename FT>
    void futureCancelAdapter(boost::weak_ptr<FutureBaseTyped<FT> > wf)
    {
      if (boost::shared_ptr<FutureBaseTyped<FT> > f = wf.lock())
        Future<FT>(f).cancel();
    }
  }

  template <>
  struct FutureValueConverter<void, void>
  {
    void operator()(void* /*in*/, void* /*out*/)
    {
    }
  };

  template <typename T>
  struct FutureValueConverter<T, void>
  {
    void operator()(const T& /*in*/, void* /*out*/)
    {
    }
  };

  template <typename T>
  struct FutureValueConverter<void, T>
  {
    void operator()(void* /*in*/, const T& /*out*/)
    {
    }
  };

  template<typename R>
  void adaptFutureUnwrap(Future<AnyReference>& f, Promise<R>& p)
  {
    p.setup(boost::bind(&detail::futureCancelAdapter<AnyReference>,
          boost::weak_ptr<detail::FutureBaseTyped<AnyReference> >(f._p)));
    f.connect(boost::function<void(const qi::Future<AnyReference>&)>(
          boost::bind(&detail::futureAdapter<R>, std::placeholders::_1, p)));
  }

  template<typename FT, typename PT>
  void adaptFuture(const Future<FT>& f, Promise<PT>& p, AdaptFutureOption option)
  {
    if (option == AdaptFutureOption_ForwardCancel)
      p.setup(boost::bind(&detail::futureCancelAdapter<FT>,
            boost::weak_ptr<detail::FutureBaseTyped<FT> >(f._p)));
    const_cast<Future<FT>&>(f).connect(boost::bind(detail::futureAdapter<FT, PT, FutureValueConverter<FT, PT> >, std::placeholders::_1, p,
      FutureValueConverter<FT, PT>()));
  }

  template<typename FT, typename PT, typename CONV>
  void adaptFuture(const Future<FT>& f, Promise<PT>& p, CONV converter, AdaptFutureOption option)
  {
    if (option == AdaptFutureOption_ForwardCancel)
      p.setup(boost::bind(&detail::futureCancelAdapter<FT>,
            boost::weak_ptr<detail::FutureBaseTyped<FT> >(f._p)));
    const_cast<Future<FT>&>(f).connect(boost::bind(detail::futureAdapter<FT, PT, CONV>, std::placeholders::_1, p, converter));
  }

  template <typename T>
  Future<AnyValue> toAnyValueFuture(Future<T> future)
  {
    return future.andThen([](const T& val) {
        return AnyValue::from(val);
    });
  }

  template <>
  inline Future<AnyValue> toAnyValueFuture(Future<void> future)
  {
    return future.andThen([](void *) {
      // create a void AnyValue
      return AnyValue(typeOf<void>());
    });
  }

  namespace detail
  {

    template <typename T>
    struct FutureWrapper
    {
      void operator,(const T& val)
      {
        future = Future<T>(val);
      }

      void operator,(const Future<T>& val)
      {
        future = val;
      }

      FutureWrapper<T>& operator()()
      {
        return *this;
      }

      qi::Future<T> future;
    };

    template <>
    struct FutureWrapper<void>
    {
      // initialize the future as operator, wont be called if the function returns void
      // if it returns a future, then this value will be overwritten anyway, so no problem
      FutureWrapper()
        : future(0)
      {}

      void operator,(const Future<void>& val)
      {
        future = val;
      }

      FutureWrapper<void>& operator()()
      {
        return *this;
      }

      qi::Future<void> future;
    };
  } // detail
} // qi

#include <qi/detail/futurebarrier.hpp>

#endif  // _QI_DETAIL_FUTURE_HXX_
