/*
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2012 Cedric GESTES
*/

#ifndef _QIMESSAGING_DETAILS_FUTURE_HPP_
#define _QIMESSAGING_DETAILS_FUTURE_HPP_

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

    protected:
      void reportReady();
      void reportError(const std::string &message);

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
          _callback(0),
          _data(0)
      {
      }

      void setValue(const T &value)
      {
        _value = value;
        reportReady();
        if (_callback) {
          _callback->onFutureFinished(*_self, _data);
        }
      }

      void setError(const std::string &message)
      {
        reportError(message);
        if (_callback) {
          _callback->onFutureFailed(*_self, _data);
        }
      }

      void setCallback(FutureInterface<T> *p_interface, void *data) {
        _callback = p_interface;
        _data     = data;
      }

      const T &value() const    { wait(); return _value; }
      T &value()                { wait(); return _value; }

    private:
      Future<T>          *_self;
      T                   _value;
      FutureInterface<T> *_callback;
      void               *_data;
    };


  } // namespace detail

  namespace detail {
    //void specialisation: do not hold anything only used for synchronisation
    template <>
    class FutureState<void> : public FutureBase {
    public:
      FutureState(Future<void> *fut)
        : _self(fut),
          _callback(0),
          _data(0)
      {
      }

      void setValue(const void *QI_UNUSED(value))
      {
        reportReady();
        if (_callback) {
            _callback->onFutureFinished(*_self, _data);
          }
      }

      void setError(const std::string &message)
      {
        reportError(message);
        if (_callback) {
          _callback->onFutureFailed(*_self, _data);
        }
      }

      void setCallback(FutureInterface<void> *p_interface, void *data) {
        _callback = p_interface;
        _data     = data;
      }

      void value() const    { wait(); }

    private:
      Future<void>          *_self;
      FutureInterface<void> *_callback;
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

    void value() const    { _p->wait(); }

    bool wait(int msecs = 30000) const         { return _p->wait(msecs); }
    bool isReady() const                       { return _p->isReady(); }

    bool hasError() const                      { return _p->hasError(); }
    const std::string &error() const           { return _p->error(); }

    void setCallback(FutureInterface<void> *p_interface, void *data = 0) {
      _p->setCallback(p_interface, data);
    }

    friend class Promise<void>;
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

    void setError(const std::string &message) {
      _f._p->setError(message);
    }

    Future<void> future() { return _f; }

  protected:
    Future<void> _f;
  };

};

#endif
