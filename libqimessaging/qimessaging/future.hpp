/*
*  Author(s):
*  - Cedric Gestes <gestes@aldebaran-robotics.com>
*
*  Copyright (C) 2012 Aldebaran Robotics
*/


#pragma once
#ifndef _QIMESSAGING_FUTURE_HPP_
#define _QIMESSAGING_FUTURE_HPP_

namespace qi {

  template <typename T> class Future;
  template <typename T> class Promise;

  template <typename T>
  class FutureInterface {
  public:
    virtual void onFutureFinished(const Future<T> *future) = 0;
  };

  template <typename T>
  class Future  {
  public:
    Future()
      : _isReady(false),
        _delegate(0)
    {};

    const T &value()const { return _value; }

    bool waitForValue(int msecs = 30000);

    bool isReady() { return _isReady; }

    void callback(FutureInterface<T> *interface) { _delegate = interface; }

    friend class Promise<T>;

  protected:
    void setValue(const T &value) {
      _value = value;
      _isReady = true;
      if (_delegate)
        _delegate->onFutureFinished(this);
    }

    bool                _isReady;
    T                   _value;
    FutureInterface<T> *_delegate;
  };


  template <typename T>
  class Promise {
  public:
    Promise() { _f = new Future<T>(); }

    void setValue(const T &value) {
      _f->setValue(value);
    }

    Future<T> *future() { return _f; }

  protected:
    Future<T> *_f;
  };

};


#endif  // _QIMESSAGING_FUTURE_HPP_
