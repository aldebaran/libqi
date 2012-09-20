#pragma once
/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/

#ifndef _QIMESSAGING_DETAILS_FUTURE_HXX_
#define _QIMESSAGING_DETAILS_FUTURE_HXX_

#include <vector>
#include <utility> // pair
#include <boost/thread/mutex.hpp>

namespace qi {

  namespace detail {

    class FutureBasePrivate;
    class QIMESSAGING_API FutureBase {
    public:
      FutureBase();
      ~FutureBase();
      bool wait(int msecs = 30000) const;
      bool isReady() const;
      bool hasError() const;
      const std::string &error() const;
      void reset();

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
        _onResult.connect(fun, evloop);
        //result already ready, notify the callback
        //TODO: if evLoop use it
        if (isReady()) {
          if (evloop)
            evloop->asyncCall(1, boost::bind<void>(fun, future));
          else
            fun(future);
        }
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

}

#endif  // _QIMESSAGING_DETAILS_FUTURE_HXX_
