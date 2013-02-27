#pragma once
/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/

#ifndef _QITYPE_DETAILS_FUTURE_HXX_
#define _QITYPE_DETAILS_FUTURE_HXX_

#include <vector>
#include <utility> // pair
#include <boost/thread/recursive_mutex.hpp>
#include <boost/bind.hpp>
#include <qi/api.hpp>

namespace qi {

  namespace detail {

    class FutureBasePrivate;
    class QI_API FutureBase {
    public:
      FutureBase();
      ~FutureBase();
      bool wait(int msecs = 0) const;
      bool isReady() const;
      bool hasError(int msecs = 0) const;
      const std::string &error() const;
      void reset();
      boost::recursive_mutex& mutex();
    protected:
      void reportReady();
      void reportError(const std::string &message);
      void notifyReady();

    public:
      FutureBasePrivate *_p;
    };

    //common state shared between a Promise and multiple Futures
    template <typename T>
    class FutureState : public FutureBase {
    public:
      typedef typename FutureType<T>::type ValueType;
      FutureState()
        : _value()
      {
      }

      bool isCanceleable() const
      {
        return _onCancel;
      }

      void cancel()
      {
        if (isReady())
          return;
        if (!_onCancel)
          throw std::runtime_error("Future not cancelleable");
        _onCancel();
      }

      void setOnCancel(boost::function<void ()> onCancel)
      {
        _onCancel = onCancel;
      }
      void setValue(qi::Future<T>& future, const ValueType &value)
      {
        if (wait(-1))
          throw std::runtime_error("Future value already set.");
        // report-ready + onResult() must be atomic to avoid
        // missing callbacks/double calls in case connect() is invoked at
        // the same time
        boost::recursive_mutex::scoped_lock lock(mutex());
        _value = value;
        reportReady();
        for(unsigned i=0; i<_onResult.size(); ++i)
          _onResult[i](future);
        notifyReady();
      }

      void setError(qi::Future<T>& future, const std::string &message)
      {
        boost::recursive_mutex::scoped_lock lock(mutex());
        reportError(message);
        for(unsigned i=0; i<_onResult.size(); ++i)
          _onResult[i](future);
        notifyReady();
      }

      void connect(qi::Future<T> future, const boost::function<void (qi::Future<T>)> &s)
      {
        bool ready;
        {
          boost::recursive_mutex::scoped_lock lock(mutex());
          _onResult.push_back(s);
          ready = isReady();
        }
        //result already ready, notify the callback
        if (ready) {
          s(future);
        }
      }

      const ValueType &value() const    { wait(); if (hasError()) throw std::runtime_error(error()); return _value; }

    private:
      typedef std::vector<boost::function<void (qi::Future<T>)> > Callbacks;
      Callbacks _onResult;
      ValueType                         _value;
      boost::function<void ()>          _onCancel;
    };

    template <typename T>
    void waitForFirstHelper(qi::Promise<bool>& bprom,
                            qi::Promise< qi::Future<T> >& prom,
                            qi::Future<T>& fut) {
      if (prom.future().isReady())
        return;
      if (!fut.hasError())
        prom.setValue(fut);
      bprom.setValue(true);
    }

    template <typename T>
    void waitForFirstHelperFailure(qi::Promise< qi::Future<T> >& prom) {
      if (prom.future().isReady())
        return;
      prom.setValue(makeFutureError<T>("No future returned successfully."));
    }
  } // namespace detail

  template <typename T>
  qi::Future<T> makeFutureError(const std::string &error) {
    qi::Promise<T> prom;
    prom.setError(error);
    return prom.future();
  }

  template<typename T>
  const typename Future<T>::ValueType& Future<T>::valueWithDefault(const ValueType& v) const
  {
    _p->wait();
    if (hasError())
      return v;
    return _p->value();
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
}

#endif  // _QITYPE_DETAILS_FUTURE_HXX_
