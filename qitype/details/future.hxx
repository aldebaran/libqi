#pragma once
/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/

#ifndef _QITYPE_DETAILS_FUTURE_HXX_
#define _QITYPE_DETAILS_FUTURE_HXX_

#include <vector>
#include <utility> // pair
#include <boost/thread/mutex.hpp>
#include <boost/bind.hpp>
#include <qitype/signal.hpp>

namespace qi {

  namespace detail {

    class FutureBasePrivate;
    class QITYPE_API FutureBase {
    public:
      FutureBase();
      ~FutureBase();
      bool wait(int msecs = 30000) const;
      bool isReady() const;
      bool hasError(int msecs = 30000) const;
      const std::string &error() const;
      void reset();
      boost::mutex& mutex();
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

      void setValue(qi::Future<T> future, const ValueType &value)
      {
        _value = value;
        reportReady();
        _onResult(future);
        notifyReady();
      }

      void setError(qi::Future<T> future, const std::string &message)
      {
        reportError(message);
        _onResult(future);
        notifyReady();
      }

      unsigned int connect(qi::Future<T> future, boost::function<void (qi::Future<T>)> fun, qi::EventLoop *evloop) {
        bool ready;
        unsigned int res;
        {
          boost::mutex::scoped_lock lock(mutex());
          res = _onResult.connect(fun, evloop);
          ready = isReady();
        }
        //result already ready, notify the callback
        if (ready) {
          if (evloop)
            evloop->asyncCall(1, boost::bind<void>(fun, future));
          else
            fun(future);
        }
        return res;
      }

      bool disconnect(unsigned int i) {
        return _onResult.disconnect(i);
      }


      const ValueType &value() const    { wait(); return _value; }
      ValueType &value()                { wait(); return _value; }

    private:
      qi::Signal<void (qi::Future<T>)>  _onResult;
      ValueType                         _value;
    };
  } // namespace detail

  template <typename T>
  qi::Future<T> makeFutureError(const std::string &error) {
    qi::Promise<T> prom;
    prom.setError(error);
    return prom.future();
  }
}

#endif  // _QITYPE_DETAILS_FUTURE_HXX_
