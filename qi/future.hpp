#pragma once
/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/

#ifndef _QITYPE_FUTURE_HPP_
#define _QITYPE_FUTURE_HPP_

#include <vector>
#include <qi/config.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/signals2.hpp>
#include <qi/eventloop.hpp>

#ifdef _MSC_VER
#  pragma warning( push )
#  pragma warning( disable: 4251 )
#endif

namespace qi {

  template<typename T> struct FutureType
  {
    typedef T type;
  };

  // Hold a void* for Future<void>
  template<> struct FutureType<void>
  {
    typedef void* type;
  };

  template <typename T> class FutureInterface;
  template <typename T> class Future;
  template <typename T> class FutureSync;
  template <typename T> class Promise;

  namespace detail {
    template <typename T> class FutureState;
  }

  template <typename T>
  class Future {
  public:
    typedef typename FutureType<T>::type ValueType;
    Future()
      : _p(new detail::FutureState<T>())
    {
    }

    Future(const FutureSync<T>& b)
    {
      *this = b;
    }

    inline Future<T>& operator = (const FutureSync<T>& b)
    {
      b._sync = false;
      _p = b._p;
      return *this;
    }

    explicit Future<T>(const ValueType& v)
    {
      Promise<T> promise;
      promise.setValue(v);
      *this = promise.future();
    }

    inline const ValueType &value() const    { return _p->value(); }

    /** Wait for future, and return a default value in case of error.
     * @param defaultVal the value to return in case of Future error
     * @return the future value, or \p defaultVal if hasError() is true.
     */
    inline const ValueType &valueWithDefault(const ValueType& defaultVal = ValueType()) const;

    inline operator const ValueType&() const { return _p->value(); }

    /** Wait for future to contain a value or an error
     @param msecs: Maximum time to wait in milliseconds, 0 means forever and -1 means return immediately.
     @return true if future contains a value or an error, false if timeout was reached
     */
    inline bool wait(int msecs = 30000) const         { return _p->wait(msecs); }
    inline bool isReady() const                       { return _p->isReady(); }
    inline bool hasError(int msecs=30000) const       { return _p->hasError(msecs); }

    inline const std::string &error() const           { return _p->error(); }


    inline FutureSync<T> sync()
    {
      return FutureSync<T>(*this);
    };

  public: //Signals
    typedef boost::signals2::connection Connection;
    typedef typename boost::signals2::signal<void (Future<T>)>::slot_type Slot;
    inline Connection connect(const Slot& s) { return _p->connect(*this, s);}
    inline bool disconnect(Connection i) { return _p->disconnect(i); }
    //qi::Signal<void (qi::Future<T>)> &onResult() { return _p->_onResult; }

  protected:
    // C4251 needs to have dll-interface to be used by clients of class 'qi::Future<T>'
    boost::shared_ptr< detail::FutureState<T> > _p;
    friend class Promise<T>;
    friend class FutureSync<T>;
  };

  template<typename T> class FutureSync: public Future<T>
  {
  public:
    // This future cannot be set, so sync starts at false
    FutureSync() : _sync(false) {}

    FutureSync(const Future<T>& b)
    : _sync(true)
    {
      *this = b;
      this->_p = b._p;
    }

    FutureSync(const FutureSync<T>& b)
    : _sync(true)
    {
      *this = b;
      this->_p = b._p;
      b._sync = false;
    }

    explicit FutureSync<T>(const typename Future<T>::ValueType& v)
    : _sync(false)
    {
      Promise<T> promise;
      promise.setValue(v);
      *this = promise.future();
    }

    inline FutureSync<T>& operator = (const FutureSync<T>& b)
    {
      this->_p = b._p;
      _sync = true;
      b._sync = false;
      return *this;
    }

    inline FutureSync<T>& operator = (const Future<T>& b)
    {
     this->_p = b._p;
      _sync = true;
      return *this;
    }

    ~FutureSync()
    {
      if (_sync)
        this->wait();
    }

    Future<T> async()
    {
      return *this;
    }

  private:
    mutable bool _sync;
    friend class Future<T>;
  };


  template <typename T>
  class Promise {
  public:
    typedef typename FutureType<T>::type ValueType;

    Promise() { }

    void setValue(const ValueType &value) {
      _f._p->setValue(_f, value);
    }

    void setError(const std::string &msg) {
      _f._p->setError(_f, msg);
    }

    void reset() {
      _f._p->reset();
    }

    Future<T> future() { return _f; }

  protected:
    Future<T> _f;
  };

  template <typename T>
  qi::Future<T> makeFutureError(const std::string &value);
};

#ifdef _MSC_VER
#  pragma warning( pop )
#endif

#include <qi/details/future.hxx>

#endif  // _QITYPE_FUTURE_HPP_
