#pragma once
/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/

#ifndef _QI_DETAIL_FUTURE_HXX_
#define _QI_DETAIL_FUTURE_HXX_

#include <vector>
#include <utility> // pair
#include <boost/bind.hpp>
#include <qi/eventloop.hpp>
#include <qi/actor.hpp>
#include <qi/type/detail/futureadapter.hpp>
#include <qi/log.hpp>

namespace qi {

namespace detail {

  template <typename T, typename R>
  struct Caller
  {
    inline static R _callfunc(const T& arg,
        const boost::function<R(const T&)>& func)
    {
      return func(arg);
    }
  };

  template <typename T>
  struct Caller<T, void>
  {
    inline static void* _callfunc(const T& future,
        const boost::function<void(const T&)>& func)
    {
      func(future);
      return 0;
    }
  };

  template <typename T, typename R>
  void continueThen(const Future<T>& future,
      const boost::function<R(const Future<T>&)>& func,
      qi::Promise<R>& promise)
  {
    try
    {
      promise.setValue(Caller<Future<T>, R>::_callfunc(future, func));
    }
    catch (std::exception& e)
    {
      promise.setError(e.what());
    }
    catch (...)
    {
      promise.setError("unknown exception");
    }
  }

  template <typename T, typename R>
  void continueThenAsync(const Future<T>& future,
      const boost::function<qi::Future<R>(const Future<T>&)>& func,
      qi::Promise<R>& promise)
  {
    try
    {
      qi::Future<R> fut = func(future);
      qi::adaptFuture(fut, promise, AdaptFutureOption_None);
    }
    catch (std::exception& e)
    {
      promise.setError(e.what());
    }
    catch (...)
    {
      promise.setError("unknown exception");
    }
  }

  template <bool Async, typename T, typename R>
  struct ContinueThenMaybeAsync;

  template <typename T, typename R>
  struct ContinueThenMaybeAsync<true, T, R>
  {
    template <typename AF>
    static boost::function<void(const Future<T>&)> makeFunc(AF&& func, const qi::Promise<R>& promise)
    {
      return boost::bind(&detail::continueThenAsync<T, R>,
                         _1,
                         // this cast seems necessary :(
                         boost::function<qi::Future<R>(const Future<T>&)>(std::forward<AF>(func)),
                         promise);
    }
  };

  template <typename T, typename R>
  struct ContinueThenMaybeAsync<false, T, R>
  {
    template <typename AF>
    static boost::function<void(const Future<T>&)> makeFunc(AF&& func, const qi::Promise<R>& promise)
    {
      QI_ASSERT(false && "unreachable code");
      return {};
    }
  };

  template <typename T, typename R>
  void continueAndThen(const Future<T>& future,
      const boost::function<R(const typename Future<T>::ValueType&)>& func,
      qi::Promise<R>& promise)
  {
    if (future.isCanceled())
      promise.setCanceled();
    else if (future.hasError())
      promise.setError(future.error());
    else if (promise.isCancelRequested())
      promise.setCanceled();
    else
    {
      try
      {
        promise.setValue(Caller<typename Future<T>::ValueType, R>::_callfunc(future.value(), func));
      }
      catch (std::exception& e)
      {
        promise.setError(e.what());
      }
      catch (...)
      {
        promise.setError("unknown exception");
      }
    }
  }

  template <typename T, typename R>
  void continueAndThenAsync(const Future<T>& future,
      const boost::function<qi::Future<R>(const typename Future<T>::ValueType&)>& func,
      qi::Promise<R>& promise)
  {
    if (future.isCanceled())
      promise.setCanceled();
    else if (future.hasError())
      promise.setError(future.error());
    else if (promise.isCancelRequested())
      promise.setCanceled();
    else
    {
      try
      {
        qi::Future<R> fut = func(future.value());
        qi::adaptFuture(fut, promise);
      }
      catch (std::exception& e)
      {
        promise.setError(e.what());
      }
      catch (...)
      {
        promise.setError("unknown exception");
      }
    }
  }

  template <bool Async, typename T, typename R>
  struct ContinueAndThenMaybeAsync;

  template <typename T, typename R>
  struct ContinueAndThenMaybeAsync<true, T, R>
  {
    template <typename AF>
    static boost::function<void(const Future<T>&)> makeFunc(AF&& func, const qi::Promise<R>& promise)
    {
      return boost::bind(&detail::continueAndThenAsync<T, R>,
                         _1,
                         // this cast seems necessary :(
                         boost::function<qi::Future<R>(const typename Future<T>::ValueType&)>(std::forward<AF>(func)),
                         promise);
    }
  };

  template <typename T, typename R>
  struct ContinueAndThenMaybeAsync<false, T, R>
  {
    template <typename AF>
    static boost::function<void(const Future<T>&)> makeFunc(AF&& func, const qi::Promise<R>& promise)
    {
      QI_ASSERT(false && "unreachable code");
      return {};
    }
  };

} // namespace detail

  template <typename T>
  template <typename R, typename AF>
  inline Future<R> Future<T>::thenR(FutureCallbackType type, AF&& func)
  {
    return thenRImpl<R>(type, std::forward<AF>(func));
  }

  template <typename T>
  template <typename R, typename AF>
  inline Future<R> Future<T>::thenRImpl(FutureCallbackType type, AF&& func)
  {
    boost::weak_ptr<detail::FutureBaseTyped<T> > weakp(_p);
    qi::Promise<R> promise([weakp](const qi::Promise<R>&){
          if (auto futureb = weakp.lock())
            Future<T>(futureb).cancel();
        });

    if (type == FutureCallbackType_Auto && detail::IsAsyncBind<AF>::value)
    {
      type = FutureCallbackType_Sync;
      _p->connect(*this,
                  detail::ContinueThenMaybeAsync<detail::IsAsyncBind<AF>::value, T, R>
                      ::makeFunc(std::forward<AF>(func), promise),
                  type);
    }
    else
    {
      _p->connect(*this,
          boost::bind(
            &detail::continueThen<T, R>,
            _1,
            // this cast seems necessary :(
            boost::function<R(const Future<T>&)>(std::forward<AF>(func)),
            promise),
          type);
    }
    return promise.future();
  }

  template <typename T>
  template <typename R, typename AF>
  inline Future<R> Future<T>::andThenR(FutureCallbackType type, AF&& func)
  {
    return andThenRImpl<R>(type, std::forward(func));
  }

  template <typename T>
  template <typename R, typename AF>
  inline Future<R> Future<T>::andThenRImpl(FutureCallbackType type, AF&& func)
  {
    boost::weak_ptr<detail::FutureBaseTyped<T> > weakp(_p);
    qi::Promise<R> promise([weakp](const qi::Promise<R>&){
          if (auto futureb = weakp.lock())
            Future<T>(futureb).cancel();
        });

    if (type == FutureCallbackType_Auto && detail::IsAsyncBind<AF>::value)
    {
      type = FutureCallbackType_Sync;
      _p->connect(*this,
                  detail::ContinueAndThenMaybeAsync<detail::IsAsyncBind<AF>::value, T, R>
                      ::makeFunc(std::forward<AF>(func), promise),
                  type);
    }
    else
    {
      _p->connect(*this,
                  boost::bind(&detail::continueAndThen<T, R>,
                              _1,
                              // this cast seems necessary :(
                              boost::function<R(const typename Future<T>::ValueType&)>(std::forward<AF>(func)),
                              promise),
                  type);
    }
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
      if (_onDestroyed && hasValue(0))
        _onDestroyed(_value);
    }

    template <typename T>
    void FutureBaseTyped<T>::cancel(qi::Future<T>& future)
    {
      CancelCallback onCancel;
      {
        boost::recursive_mutex::scoped_lock lock(mutex());
        if (isFinished())
          return;
        requestCancel();
        onCancel = _onCancel;
      }
      if (onCancel)
      {
        qi::Promise<T> prom(future);
        onCancel(prom);
      }
    }

    template <typename T>
    void FutureBaseTyped<T>::setOnCancel(qi::Promise<T>& promise, CancelCallback onCancel)
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
    void FutureBaseTyped<T>::callCbNotify(qi::Future<T>& future)
    {
      for (unsigned i = 0; i < _onResult.size(); ++i)
      {
        const bool async = [&]{
          if (_onResult[i].callType != FutureCallbackType_Auto)
            return _onResult[i].callType != FutureCallbackType_Sync;
          else
            return _async != FutureCallbackType_Sync;
        }();

        if (async)
          getEventLoop()->post(boost::bind(_onResult[i].callback, future));
        else
          try
          {
            _onResult[i].callback(future);
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
      notifyFinish();
      clearCallbacks();
    }

    template <typename T>
    void FutureBaseTyped<T>::setValue(qi::Future<T>& future, const ValueType& value)
    {
      // report-ready + onResult() must be Atomic to avoid
      // missing callbacks/double calls in case connect() is invoked at
      // the same time
      boost::recursive_mutex::scoped_lock lock(mutex());
      if (!isRunning())
        throw FutureException(FutureException::ExceptionState_PromiseAlreadySet);

      _value = value;
      reportValue();
      callCbNotify(future);
    }

    template <typename T>
    void FutureBaseTyped<T>::set(qi::Future<T>& future)
    {
      // report-ready + onResult() must be Atomic to avoid
      // missing callbacks/double calls in case connect() is invoked at
      // the same time
      boost::recursive_mutex::scoped_lock lock(mutex());
      if (!isRunning())
        throw FutureException(FutureException::ExceptionState_PromiseAlreadySet);

      reportValue();
      callCbNotify(future);
    }

    template <typename T>
    void FutureBaseTyped<T>::setError(qi::Future<T>& future, const std::string& message)
    {
      boost::recursive_mutex::scoped_lock lock(mutex());
      if (!isRunning())
        throw FutureException(FutureException::ExceptionState_PromiseAlreadySet);

      reportError(message);
      callCbNotify(future);
    }

    template <typename T>
    void FutureBaseTyped<T>::setBroken(qi::Future<T>& future)
    {
      boost::recursive_mutex::scoped_lock lock(mutex());
      QI_ASSERT(isRunning());

      reportError("Promise broken (all promises are destroyed)");
      callCbNotify(future);
    }

    template <typename T>
    void FutureBaseTyped<T>::setCanceled(qi::Future<T>& future)
    {
      boost::recursive_mutex::scoped_lock lock(mutex());
      if (!isRunning())
        throw FutureException(FutureException::ExceptionState_PromiseAlreadySet);

      reportCanceled();
      callCbNotify(future);
    }

    template <typename T>
    void FutureBaseTyped<T>::setOnDestroyed(boost::function<void(ValueType)> f)
    {
      _onDestroyed = f;
    }

    template <typename T>
    void FutureBaseTyped<T>::connect(qi::Future<T> future,
                                  const boost::function<void(qi::Future<T>)>& s,
                                  FutureCallbackType type)
    {
      if (state() == FutureState_None)
        throw FutureException(FutureException::ExceptionState_FutureInvalid);

      bool ready;
      {
        boost::recursive_mutex::scoped_lock lock(mutex());
        ready = isFinished();
        if (!ready)
          _onResult.push_back(Callback(s, type));
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

        if (async)
          getEventLoop()->post(boost::bind(s, future));
        else
        {
          try
          {
            s(future);
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
    void FutureBaseTyped<T>::clearCallbacks()
    {
      _onResult.clear();
      if (_onCancel)
      {
        _onCancel = CancelCallback(PromiseNoop<T>);
      }
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

        Promise<T> promise(boost::bind(&AddUnwrap<Future<T> >::_cancel, _1,
              boost::weak_ptr<FutureBaseTyped<Future<T> > >(self->_p)));

        self->connect(
            boost::bind(&AddUnwrap<Future<T> >::_forward, _1, promise),
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

      static void _cancel(Promise<T>& promise,
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
    void operator()(void* in, void* out)
    {
    }
  };

  template <typename T>
  struct FutureValueConverter<T, void>
  {
    void operator()(const T& in, void* out)
    {
    }
  };

  template <typename T>
  struct FutureValueConverter<void, T>
  {
    void operator()(void* in, const T& out)
    {
    }
  };

  template<typename R>
  void adaptFutureUnwrap(Future<AnyReference>& f, Promise<R>& p)
  {
    p.setup(boost::bind(&detail::futureCancelAdapter<AnyReference>,
          boost::weak_ptr<detail::FutureBaseTyped<AnyReference> >(f._p)));
    f.connect(boost::function<void(const qi::Future<AnyReference>&)>(
          boost::bind(&detail::futureAdapter<R>, _1, p)),
        FutureCallbackType_Sync);
  }

  template<typename FT, typename PT>
  void adaptFuture(const Future<FT>& f, Promise<PT>& p, AdaptFutureOption option)
  {
    if (option == AdaptFutureOption_ForwardCancel)
      p.setup(boost::bind(&detail::futureCancelAdapter<FT>,
            boost::weak_ptr<detail::FutureBaseTyped<FT> >(f._p)));
    const_cast<Future<FT>&>(f).connect(boost::bind(detail::futureAdapter<FT, PT, FutureValueConverter<FT, PT> >, _1, p,
      FutureValueConverter<FT, PT>()), FutureCallbackType_Sync);
  }

  template<typename FT, typename PT, typename CONV>
  void adaptFuture(const Future<FT>& f, Promise<PT>& p, CONV converter, AdaptFutureOption option)
  {
    if (option == AdaptFutureOption_ForwardCancel)
      p.setup(boost::bind(&detail::futureCancelAdapter<FT>,
            boost::weak_ptr<detail::FutureBaseTyped<FT> >(f._p)));
    const_cast<Future<FT>&>(f).connect(boost::bind(detail::futureAdapter<FT, PT, CONV>, _1, p, converter),
        FutureCallbackType_Sync);
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

  }

}

#include <qi/detail/futurebarrier.hpp>

#endif  // _QI_DETAIL_FUTURE_HXX_
