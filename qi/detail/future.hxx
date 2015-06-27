#pragma once
/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/

#ifndef _QI_DETAILS_FUTURE_HXX_
#define _QI_DETAILS_FUTURE_HXX_

#include <vector>
#include <utility> // pair
#include <boost/bind.hpp>
#include <qi/eventloop.hpp>
#include <qi/actor.hpp>
#include <qi/anyvalue.hpp>
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
  void continuateThen(const Future<T>& future,
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
  void continuateAndThen(const Future<T>& future,
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
        promise.setValue(Caller<T, R>::_callfunc(future.value(), func));
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

  template <typename T>
  inline void forwardCancel(
      const boost::weak_ptr<FutureBaseTyped<T> >& wfutureb)
  {
    boost::shared_ptr<FutureBaseTyped<T> > futureb = wfutureb.lock();
    if (!futureb)
      return;
    Future<T>(futureb).cancel();
  }

} // namespace detail

  template <typename T>
  template <typename R>
  inline Future<R> Future<T>::thenR(
      FutureCallbackType type,
      const boost::function<R(const Future<T>&)>& func)
  {
    qi::Promise<R> promise(
        this->isCancelable()
        ? boost::bind(&detail::forwardCancel<T>,
            boost::weak_ptr<detail::FutureBaseTyped<T> >(_p))
        : boost::function<void(qi::Promise<R>)>());
    _p->connect(*this, boost::bind(&detail::continuateThen<T, R>, _1,
          func, promise), type);
    return promise.future();
  }

  template <typename T>
  template <typename R>
  inline Future<R> Future<T>::andThenR(
      FutureCallbackType type,
      const boost::function<R(const typename Future<T>::ValueType&)>& func)
  {
    qi::Promise<R> promise(
        this->isCancelable()
        ? boost::bind(&detail::forwardCancel<T>,
            boost::weak_ptr<detail::FutureBaseTyped<T> >(_p))
        // if the future is not cancelable, now it becomes cancelable because
        // continuateAndThen will abort if cancel is requested
        : boost::function<void(qi::Promise<T>&)>(&qi::PromiseNoop<R>));
    _p->connect(*this, boost::bind(&detail::continuateAndThen<T, R>, _1,
          func, promise), type);
    return promise.future();
  }

  template <typename T>
  template <typename Arg>
  inline void Future<T>::binder(
      const boost::function<void(const boost::function<void()>&)>& poster,
      const boost::function<void(const Arg&)>& callback, const Arg& fut)
  {
    return poster(boost::bind(callback, fut));
  }

  template <typename T>
  template <typename Arg>
  inline boost::function<void(const Arg&)>
      Future<T>::transformStrandedCallback(
          qi::Strand* strand,
          const boost::function<void(const Arg&)>& cb)
  {
    return boost::bind(
        &Future<T>::binder<Arg>,
        boost::function<void(const boost::function<void()>&)>(
          boost::bind(
            &qi::Strand::post,
            strand, _1)),
        cb, _1);
  }

  template <typename T>
  template <typename ARG0>
  inline typename boost::enable_if<
      boost::is_base_of<Actor, typename detail::Unwrap<ARG0>::type>,
      void>::type
      Future<T>::_connectMaybeActor(
          const ARG0& arg0, const boost::function<void(const Future<T>&)>& cb,
          FutureCallbackType type)
  {
    _p->connect(*this,
                qi::trackWithFallback(
                    boost::function<void()>(),
                    transformStrandedCallback<qi::Future<T> >(
                        detail::Unwrap<ARG0>::unwrap(arg0)->strand(), cb),
                    arg0),
                FutureCallbackType_Sync);
  }
  template <typename T>
  template <typename ARG0>
  inline typename boost::disable_if<
      boost::is_base_of<Actor, typename detail::Unwrap<ARG0>::type>,
      void>::type
      Future<T>::_connectMaybeActor(
          const ARG0& arg0, const boost::function<void(const Future<T>&)>& cb,
          FutureCallbackType type)
  {
    _p->connect(*this, cb, type);
  }

  template <typename T>
  template <typename R, typename ARG0, typename AF>
  typename boost::enable_if<
      boost::is_base_of<Actor, typename detail::Unwrap<ARG0>::type>,
      qi::Future<R> >::type
      Future<T>::_thenMaybeActor(const ARG0& arg0,
                         const AF& cb,
                         FutureCallbackType type)
  {
    return thenR(FutureCallbackType_Sync, qi::trackWithFallback(
          boost::function<void()>(),
          transformStrandedCallback<typename qi::Future<T>::ValueType>(
            detail::Unwrap<ARG0>::unwrap(arg0)->strand(), cb),
          arg0));
  }
  template <typename T>
  template <typename R, typename ARG0, typename AF>
  typename boost::disable_if<
      boost::is_base_of<Actor, typename detail::Unwrap<ARG0>::type>,
      qi::Future<R> >::type
      Future<T>::_thenMaybeActor(const ARG0& arg0,
                         const AF& cb,
                         FutureCallbackType type)
  {
    return thenR(type, cb);
  }

  template <typename T>
  void Future<T>::_weakCancelCb(const boost::weak_ptr<detail::FutureBaseTyped<T> >& wfuture)
  {
    if (boost::shared_ptr<detail::FutureBaseTyped<T> > fbt = wfuture.lock())
    {
      Future<T> future(fbt);
      assert(future.isCancelable());
      future.cancel();
    }
  }

  template <typename T>
  boost::function<void()> Future<T>::makeCanceler()
  {
    if (!isCancelable())
      throw std::runtime_error("future is not cancelable");
    return boost::bind(&Future<T>::_weakCancelCb, boost::weak_ptr<detail::FutureBaseTyped<T> >(_p));
  }

  namespace detail {

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
      void reset();

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
      typedef boost::function<void(Promise<T>)> CancelCallback;
      typedef typename FutureType<T>::type ValueType;
      FutureBaseTyped()
        : _value()
        , _async(FutureCallbackType_Async)
      {
      }

      ~FutureBaseTyped()
      {
        if (_onDestroyed && hasValue(0))
          _onDestroyed(_value);
      }

      bool isCancelable() const
      {
        return static_cast<bool>(_onCancel);
      }

      void cancel(qi::Future<T>& future)
      {
        CancelCallback onCancel;
        {
          boost::recursive_mutex::scoped_lock lock(mutex());
          if (isFinished())
            return;
          if (!_onCancel)
            throw FutureException(FutureException::ExceptionState_FutureNotCancelable);
          requestCancel();
          onCancel = _onCancel;
        }
        onCancel(Promise<T>(future));
      }

      void setOnCancel(qi::Promise<T>& promise,
        CancelCallback onCancel)
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

      void callCbNotify(qi::Future<T>& future)
      {
        for(unsigned i = 0; i<_onResult.size(); ++i)
        {
          try {
            if (_async == FutureCallbackType_Async)
              getEventLoop()->post(boost::bind(_onResult[i], future));
            else
              _onResult[i](future);
          } catch(const qi::PointerLockException&) { // do nothing
          } catch(const std::exception& e) {
            qiLogError("qi.future") << "Exception caught in future callback "
                                    << e.what();
          } catch (...) {
            qiLogError("qi.future")
                << "Unknown exception caught in future callback";
          }
        }
        notifyFinish();
        clearCallbacks();
      }

      void setValue(qi::Future<T>& future, const ValueType &value)
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

      /*
       * inplace api for promise
       */
      void set(qi::Future<T>& future)
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

      void setError(qi::Future<T>& future, const std::string &message)
      {
        boost::recursive_mutex::scoped_lock lock(mutex());
        if (!isRunning())
          throw FutureException(FutureException::ExceptionState_PromiseAlreadySet);

        reportError(message);
        callCbNotify(future);
      }

      void setBroken(qi::Future<T>& future)
      {
        boost::recursive_mutex::scoped_lock lock(mutex());
        assert(isRunning());

        reportError("Promise broken (all promises are destroyed)");
        callCbNotify(future);
      }

      void setCanceled(qi::Future<T>& future) {
        boost::recursive_mutex::scoped_lock lock(mutex());
        if (!isRunning())
          throw FutureException(FutureException::ExceptionState_PromiseAlreadySet);

        reportCanceled();
        callCbNotify(future);
      }

      void setOnDestroyed(boost::function<void (ValueType)> f)
      {
        _onDestroyed = f;
      }

      void connect(qi::Future<T> future,
          const boost::function<void (qi::Future<T>)> &s,
          FutureCallbackType type)
      {
        bool ready;
        {
          boost::recursive_mutex::scoped_lock lock(mutex());
          ready = isFinished();
          if (!ready)
          {
            _onResult.push_back(s);
          }
        }
        //result already ready, notify the callback
        if (ready) {
          if (type == FutureCallbackType_Async)
          {
            getEventLoop()->post(boost::bind(s, future));
          }
          else
          {
            try {
              s(future);
            } catch(const ::qi::PointerLockException&)
            {/*do nothing*/}
          }
        }
      }

      const ValueType &value(int msecs) const {
        FutureState state = wait(msecs);
        if (state == FutureState_Running)
          throw FutureException(FutureException::ExceptionState_FutureTimeout);
        if (state == FutureState_Canceled)
          throw FutureException(FutureException::ExceptionState_FutureCanceled);
        if (state == FutureState_FinishedWithError)
          throw FutureUserException(error(FutureTimeout_None));
        return _value;
      }

    private:
      friend class Promise<T>;
      typedef std::vector<boost::function<void (qi::Future<T>)> > Callbacks;
      Callbacks                _onResult;
      ValueType                _value;
      CancelCallback           _onCancel;
      boost::function<void (ValueType)> _onDestroyed;
      FutureCallbackType       _async;
      qi::Atomic<unsigned int> _promiseCount;

      void clearCallbacks()
      {
        _onResult.clear();
        if (_onCancel)
        {
          _onCancel = CancelCallback(PromiseNoop<T>);
        }
      }
    };

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
          if (future.isCancelable())
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
    void futureAdapter(Future<FT> f, Promise<PT> p, CONV converter)
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
    if (f.isCancelable())
      p.setup(boost::bind(&detail::futureCancelAdapter<AnyReference>,
            boost::weak_ptr<detail::FutureBaseTyped<AnyReference> >(f._p)));
    f.connect(boost::function<void(qi::Future<AnyReference>&)>(
          boost::bind(&detail::futureAdapter<R>, _1, p)),
        FutureCallbackType_Sync);
  }

  template<typename FT, typename PT>
  void adaptFuture(const Future<FT>& f, Promise<PT>& p)
  {
    if (f.isCancelable())
      p.setup(boost::bind(&detail::futureCancelAdapter<FT>,
            boost::weak_ptr<detail::FutureBaseTyped<FT> >(f._p)));
    const_cast<Future<FT>&>(f).connect(boost::bind(detail::futureAdapter<FT, PT, FutureValueConverter<FT, PT> >, _1, p,
      FutureValueConverter<FT, PT>()), FutureCallbackType_Sync);
  }

  template<typename FT, typename PT, typename CONV>
  void adaptFuture(const Future<FT>& f, Promise<PT>& p, CONV converter)
  {
    if (f.isCancelable())
      p.setup(boost::bind(&detail::futureCancelAdapter<FT>,
            boost::weak_ptr<detail::FutureBaseTyped<FT> >(f._p)));
    const_cast<Future<FT>&>(f).connect(boost::bind(detail::futureAdapter<FT, PT, CONV>, _1, p, converter), FutureCallbackType_Sync);
  }
}

#include <qi/detail/futurebarrier.hpp>

#endif  // _QI_DETAILS_FUTURE_HXX_
