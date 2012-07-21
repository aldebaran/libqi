/*
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2012 Cedric GESTES
*/

#ifndef _QIMESSAGING_DETAILS_FUTURE_HPP_
#define _QIMESSAGING_DETAILS_FUTURE_HPP_

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
    public:
      FutureBasePrivate *_p;
    };

    //common state shared between a Promise and multiple Futures
    template <typename T>
    class FutureState : public FutureBase {
    public:
      typedef typename std::vector<std::pair<FutureInterface<T> *, void *> >::iterator futureInterfaceListIterator;

      FutureState(Future<T> *fut)
        : _self(fut),
          _value()
      {
      }

      void setValue(const T &value)
      {
        _value = value;
        futureInterfaceListIterator iter;
        boost::mutex::scoped_lock l(_mutexCallback);
        for (iter = _callback.begin(); iter != _callback.end(); ++iter) {
          (*iter).first->onFutureFinished(_value, (*iter).second);
        }
        reportReady();
      }

      void setError(const std::string &message)
      {
        futureInterfaceListIterator iter;
        boost::mutex::scoped_lock l(_mutexCallback);
        for (iter = _callback.begin(); iter != _callback.end(); ++iter) {
          (*iter).first->onFutureFailed(message, (*iter).second);
        }
        reportError(message);
      }

      void addCallbacks(FutureInterface<T> *p_interface, void *data) {
        std::pair<FutureInterface<T> *, void *> itf = std::make_pair(p_interface, data);
        boost::mutex::scoped_lock l(_mutexCallback);
        _callback.push_back(itf);
      }

      void removeCallbacks(FutureInterface<T> *p_interface) {
        boost::mutex::scoped_lock l(_mutexCallback);
        futureInterfaceListIterator iter;
        for (iter = _callback.begin(); iter != _callback.end(); ++iter) {
          if ((*iter).first == p_interface) {
            _callback.erase(iter);
            break;
          }
        }
      }

      std::vector<std::pair<FutureInterface<T> *, void *> > callbacks()
      {
        return _callback;
      }

      const T &value() const    { wait(); return _value; }
      T &value()                { wait(); return _value; }

    private:
      Future<T>                        *_self;
      T                                 _value;
      std::vector<std::pair<FutureInterface<T> *, void *> > _callback;
      boost::mutex                                          _mutexCallback;
    };
  } // namespace detail

  namespace detail {
    //void specialisation: do not hold anything only used for synchronisation
    template <>
    class FutureState<void> : public FutureBase {
    public:
      FutureState(Future<void> *fut)
        : _self(fut)
      {
      }

      void setValue(const void *QI_UNUSED(value))
      {
        boost::mutex::scoped_lock l(_mutexCallback);
        std::vector<std::pair<FutureInterface<void> *, void *> >::iterator iter;
        for (iter = _callback.begin(); iter != _callback.end(); ++iter) {
          (*iter).first->onFutureFinished((*iter).second);
        }
        reportReady();
      }

      void setError(const std::string &message)
      {
        boost::mutex::scoped_lock l(_mutexCallback);
        std::vector<std::pair<FutureInterface<void> *, void *> >::iterator iter;
        for (iter = _callback.begin(); iter != _callback.end(); ++iter) {
          (*iter).first->onFutureFailed(message, (*iter).second);
        }
        reportError(message);
      }

      void addCallbacks(FutureInterface<void> *p_interface, void *data) {
        std::pair<FutureInterface<void> *, void *> itf = std::make_pair(p_interface, data);
        boost::mutex::scoped_lock l(_mutexCallback);
        _callback.push_back(itf);
      }

      void removeCallbacks(FutureInterface<void> *p_interface) {
        boost::mutex::scoped_lock l(_mutexCallback);
        std::vector<std::pair<FutureInterface<void> *, void *> >::iterator iter;
        for (iter = _callback.begin(); iter != _callback.end(); ++iter) {
          if ((*iter).first == p_interface) {
            _callback.erase(iter);
            break;
          }
        }
      }

      std::vector<std::pair<FutureInterface<void> *, void *> > callbacks()
      {
        return _callback;
      }

      void value() const    { wait(); }

    private:
      Future<void>                        *_self;
      std::vector<std::pair<FutureInterface<void> *, void *> > _callback;
      boost::mutex                                             _mutexCallback;
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

    void addCallbacks(FutureInterface<void> *p_interface, void *data = 0) {
      _p->addCallbacks(p_interface, data);
    }

    void removeCallbacks(FutureInterface<void> *p_interface) {
      _p->removeCallbacks(p_interface);
    }

    std::vector<std::pair<FutureInterface<void> *, void *> > callbacks()
    {
      return _p->callbacks();
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

    void reset() {
      _f._p->reset();
    }

    Future<void> future() { return _f; }

  protected:
    Future<void> _f;
  };

};

#endif
