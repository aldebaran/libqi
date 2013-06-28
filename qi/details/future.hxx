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

#include <qi/log.hpp>

namespace qi {

  namespace detail {

    class FutureBasePrivate;
    class QI_API FutureBase {
    public:
      FutureBase();
      ~FutureBase();

      FutureState wait(int msecs) const;
      FutureState state() const;
      bool isRunning() const;
      bool isFinished() const;
      bool isCanceled() const;
      bool hasError(int msecs) const;
      bool hasValue(int msecs) const;
      const std::string &error(int msecs) const;
      void reportStart();
      void reset();

    protected:
      void reportValue();
      void reportError(const std::string &message);
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
      typedef typename FutureType<T>::type ValueType;
      FutureBaseTyped()
        : _value()
        , _async(FutureCallbackType_Async)
      {
      }

      bool isCanceleable() const
      {
        return _onCancel;
      }

      void cancel()
      {
        if (isFinished())
          return;
        if (!_onCancel)
          throw FutureException(FutureException::ExceptionState_FutureNotCancelable);
        _onCancel();
      }

      void setOnCancel(boost::function<void ()> onCancel)
      {
        _onCancel = onCancel;
      }


      void callCbNotify(qi::Future<T>& future)
      {
        for(unsigned i = 0; i<_onResult.size(); ++i)
        {
          try {
            if (_async == FutureCallbackType_Async)
              getDefaultObjectEventLoop()->post(boost::bind(_onResult[i], future));
            else
              _onResult[i](future);
          } catch(const std::exception& e) {
            qiLogError("qi.future") << "Exception caught in future callback "
                                    << e.what();
          } catch (...) {
            qiLogError("qi.future")
                << "Unknown exception caught in future callback";
          }
        }
        notifyFinish();
      }

      void setValue(qi::Future<T>& future, const ValueType &value)
      {
        // report-ready + onResult() must be atomic to avoid
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
        // report-ready + onResult() must be atomic to avoid
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

      void setCanceled(qi::Future<T>& future) {
        boost::recursive_mutex::scoped_lock lock(mutex());
        if (!isRunning())
          throw FutureException(FutureException::ExceptionState_PromiseAlreadySet);

        reportCanceled();
        callCbNotify(future);
      }


      void connect(qi::Future<T> future, const boost::function<void (qi::Future<T>)> &s)
      {
        bool ready;
        {
          boost::recursive_mutex::scoped_lock lock(mutex());
          _onResult.push_back(s);
          ready = isFinished();
        }
        //result already ready, notify the callback
        if (ready) {
          if (_async == FutureCallbackType_Async)
            getDefaultObjectEventLoop()->post(boost::bind(s, future));
          else
            s(future);
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
      boost::function<void ()> _onCancel;
      FutureCallbackType       _async;
    };

    template <typename T>
    void waitForFirstHelper(qi::Promise<bool>& bprom,
                            qi::Promise< qi::Future<T> >& prom,
                            qi::Future<T>& fut) {
      if (prom.future().isFinished())
        return;
      if (!fut.hasError())
        prom.setValue(fut);
      bprom.setValue(true);
    }

    template <typename T>
    void waitForFirstHelperFailure(qi::Promise< qi::Future<T> >& prom) {
      if (prom.future().isFinished())
        return;
      prom.setValue(makeFutureError<T>("No future returned successfully."));
    }
  } // namespace detail

  template <typename T>
  qi::Future<T> makeFutureError(const std::string &error, FutureCallbackType async) {
    qi::Promise<T> prom(async);
    prom.setError(error);
    return prom.future();
  }

  template <typename T>
  void waitForAll(std::vector<Future<T> >& vect) {
    typename std::vector< Future<T> >::iterator it;
    qi::FutureBarrier<T> barrier;

    for (it = vect.begin(); it != vect.end(); ++it) {
      barrier.addFuture(*it);
    }
    barrier.future().wait();
  }

  template <typename T>
  qi::FutureSync< qi::Future<T> > waitForFirst(std::vector< Future<T> >& vect) {
    typename std::vector< Future<T> >::iterator it;
    qi::Promise< qi::Future<T> > prom;
    qi::FutureBarrier<bool> barrier;

    for (it = vect.begin(); it != vect.end(); ++it) {
      qi::Promise<bool> rprom;

      it->connect(boost::bind<void>(&detail::waitForFirstHelper<T>, rprom, prom, *it));
      barrier.addFuture(rprom.future());
    }

    // On failure, we set the promise to an error.
    barrier.future().connect(boost::bind<void>(&detail::waitForFirstHelperFailure<T>, prom));

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

  template<typename FT, typename PT>
  void adaptFuture(const Future<FT>& f, Promise<PT>& p)
  {
    const_cast<Future<FT>&>(f).connect(boost::bind(detail::futureAdapter<FT, PT, FutureValueConverter<FT, PT> >, _1, p,
      FutureValueConverter<FT, PT>()));
  }

  template<typename FT, typename PT, typename CONV>
  void adaptFuture(const Future<FT>& f, Promise<PT>& p, CONV converter)
  {
    const_cast<Future<FT>&>(f).connect(boost::bind(detail::futureAdapter<FT, PT, CONV>, _1, p, converter));
  }
}

#endif  // _QI_DETAILS_FUTURE_HXX_
