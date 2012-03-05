/*
*  Author(s):
*  - Cedric Gestes <gestes@aldebaran-robotics.com>
*
*  Copyright (C) 2012 Aldebaran Robotics
*/


#pragma once
#ifndef _QIMESSAGING_FUTURE_HPP_
#define _QIMESSAGING_FUTURE_HPP_

#include <qimessaging/api.hpp>
#include <qimessaging/functor.hpp>
#include <boost/shared_ptr.hpp>

namespace qi {

  template <typename T> class FutureInterface;
  template <typename T> class Future;
  template <>           class Future<void>;
  template <typename T> class Promise;
  template <typename T> class FunctorReturnPromise;
  template <>           class FunctorReturnPromise<void>;

  namespace detail {
    template <typename T> class FutureState;
    template <>           class FutureState<void>;
  }

  namespace detail {

    class FutureBasePrivate;
    class QIMESSAGING_API FutureBase {
    public:
      FutureBase();
      bool waitForValue(int msecs = 30000) const;
      void setReady();
      bool isReady() const;

    public:
      FutureBasePrivate *_p;
    };

    //common state shared between a Promise and multiple Futures
    template <typename T>
    class FutureState : public FutureBase {
    public:
      FutureState(Future<T> *fut)
        : _self(fut),
          _value(),
          _delegate(0),
          _data(0)
      {
      }

      void setValue(const T &value)
      {
        _value = value;
        setReady();
        if (_delegate) {
            _delegate->onFutureFinished(*_self, _data);
          }
      }

      void setCallback(FutureInterface<T> *interface, void *data) {
        _delegate = interface;
        _data     = data;
      }

      const T &value() const    { waitForValue(); return _value; }
      T &value()                { waitForValue(); return _value; }

    private:
      Future<T>          *_self;
      T                   _value;
      FutureInterface<T> *_delegate;
      void               *_data;
    };


  } // namespace detail


  template <typename T>
  class FutureInterface {
  public:
    virtual ~FutureInterface() = 0;
    virtual void onFutureFinished(const Future<T> &future, void *data) = 0;
  };

  //pure virtual destructor need an implementation
  template <typename T>
  FutureInterface<T>::~FutureInterface()
  {}

  template <typename T>
  class Future {
  public:
    Future()
      : _p(boost::shared_ptr< detail::FutureState<T> >())
    {
      _p = boost::shared_ptr< detail::FutureState<T> >(new detail::FutureState<T>(this));
    }

    const T &value() const    { return _p->value(); }
    T &value()                { return _p->value(); }
    operator const T&() const { return _p->value(); }
    operator T&()             { return _p->value(); }

    bool waitForValue(int msecs = 30000) const { return _p->waitForValue(msecs); }
    bool isReady() const                       { return _p->isReady(); }

    void setCallback(FutureInterface<T> *interface, void *data = 0) {
      _p->setCallback(interface, data);
    }

    friend class Promise<T>;
    friend class FunctorReturnPromise<T>;
  private:
    boost::shared_ptr< detail::FutureState<T> > _p;
  };


  class QIMESSAGING_API FunctorResultPromiseBase {
  public:
    virtual ~FunctorResultPromiseBase()              = 0;
    virtual void setValue(qi::FunctorResult &result) = 0;
  };

  template <typename T>
  class Promise {
  public:
    Promise() { }

    void setValue(const T &value) {
      _f._p->setValue(value);
    }

    Future<T> future() { return _f; }

  protected:
    Future<T> _f;
  };

  template <typename T>
  class FunctorReturnPromise : public FunctorResultPromiseBase {
  public:
    FunctorReturnPromise() { }

    void setValue(const T &value) {
      _f._p->setValue(value);
    }

    virtual void setValue(qi::FunctorResult &result)
    {
      //TODO: remove the useless ref here
      T v;
      result.datastream() >> v;
      _f._p->setValue(v);
    }

    Future<T> future() { return _f; }

  protected:
    Future<T> _f;
  };



  namespace detail {
    //void specialisation: do not hold anything only used for synchronisation
    template <>
    class FutureState<void> : public FutureBase {
    public:
      FutureState(Future<void> *fut)
        : _self(fut),
          _delegate(0),
          _data(0)
      {
      }

      void setValue(const void *value)
      {
        setReady();
        if (_delegate) {
            _delegate->onFutureFinished(*_self, _data);
          }
      }

      void setCallback(FutureInterface<void> *interface, void *data) {
        _delegate = interface;
        _data     = data;
      }

      void value() const    { waitForValue(); }

    private:
      Future<void>          *_self;
      FutureInterface<void> *_delegate;
      void                  *_data;
    };
  }

  //void specialisation: do not hold anything only used for synchronisation
  template <>
  class Future<void> {
  public:
    Future()
      : _p(boost::shared_ptr< detail::FutureState<void> >())
    {
      _p = boost::shared_ptr< detail::FutureState<void> >(new detail::FutureState<void>(this));
    }

    void value() const    { _p->waitForValue(); }

    bool waitForValue(int msecs = 30000) const { return _p->waitForValue(msecs); }
    bool isReady() const                       { return _p->isReady(); }

    void setCallback(FutureInterface<void> *interface, void *data = 0) {
      _p->setCallback(interface, data);
    }

    friend class Promise<void>;
    friend class FunctorReturnPromise<void>;
  private:
    boost::shared_ptr< detail::FutureState<void> > _p;
  };

  template <>
  class Promise<void> {
  public:
    Promise() { }

    void setValue(const void *value) {
      _f._p->setValue(value);
    }

    Future<void> future() { return _f; }

  protected:
    Future<void> _f;
  };

  template <>
  class FunctorReturnPromise<void> : public FunctorResultPromiseBase {
  public:
    FunctorReturnPromise() { }

    void setValue(const void *value) {
      _f._p->setValue(value);
    }
    virtual void setValue(qi::FunctorResult &result)
    {
      //TODO: remove the useless ref here
      _f._p->setValue(0);
    }

    Future<void> future() { return _f; }

  protected:
    Future<void> _f;
  };

};


#endif  // _QIMESSAGING_FUTURE_HPP_
