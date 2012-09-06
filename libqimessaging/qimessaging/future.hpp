/*
*  Author(s):
*  - Cedric Gestes <gestes@aldebaran-robotics.com>
*
*  Copyright (C) 2012 Aldebaran Robotics
*/


#pragma once
#ifndef _QIMESSAGING_FUTURE_HPP_
#define _QIMESSAGING_FUTURE_HPP_

#include <vector>
#include <qimessaging/api.hpp>
#include <boost/shared_ptr.hpp>

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
  template <typename T> class Promise;

  namespace detail {
    template <typename T> class FutureState;
  }



  template <typename T>
  class FutureInterface {
  public:
    typedef typename FutureType<T>::type ValueType;
    virtual ~FutureInterface() = 0;
    virtual void onFutureFinished(const ValueType &value, void *data) = 0;
    virtual void onFutureFailed(const std::string &error, void *data) = 0;
  };

  //pure virtual destructor need an implementation
  template <typename T>
  FutureInterface<T>::~FutureInterface()
  {}

  template <typename T>
  class Future {
  public:
    typedef typename FutureType<T>::type ValueType;
    Future()
      : _p(boost::shared_ptr< detail::FutureState<T> >())
    {
      _p = boost::shared_ptr< detail::FutureState<T> >(new detail::FutureState<T>(this));
    }

    const ValueType &value() const    { return _p->value(); }
    ValueType &value()                { return _p->value(); }
    operator const ValueType&() const { return _p->value(); }
    operator ValueType&()             { return _p->value(); }

    bool wait(int msecs = 30000) const         { return _p->wait(msecs); }
    bool isReady() const                       { return _p->isReady(); }
    bool hasError() const                      { return _p->hasError(); }

    const std::string &error() const           { return _p->error(); }

    void addCallbacks(FutureInterface<T> *p_interface, void *data = 0) {
      _p->addCallbacks(p_interface, data);
    }

    void removeCallbacks(FutureInterface<T> *p_interface) {
      _p->removeCallbacks(p_interface);
    }

    std::vector<std::pair<FutureInterface<T> *, void *> > callbacks()
    {
      return _p->callbacks();
    }

    friend class Promise<T>;
  private:
    boost::shared_ptr< detail::FutureState<T> > _p;
  };


  template <typename T>
  class Promise {
  public:
    typedef typename FutureType<T>::type ValueType;

    Promise() { }

    void setValue(const ValueType &value) {
      _f._p->setValue(value);
    }

    void setError(const std::string &msg) {
      _f._p->setError(msg);
    }

    void reset() {
      _f._p->reset();
    }

    Future<T> future() { return _f; }

  protected:
    Future<T> _f;
  };
};

#include <qimessaging/details/future.hxx>

#endif  // _QIMESSAGING_FUTURE_HPP_
