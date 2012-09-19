/*
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2012 Cedric GESTES
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
    public:
      FutureBasePrivate *_p;
    };

    //common state shared between a Promise and multiple Futures
    template <typename T>
    class FutureState : public FutureBase {
    public:
      typedef typename FutureType<T>::type ValueType;
      typedef typename std::vector<std::pair<FutureInterface<T> *, void *> >::iterator futureInterfaceListIterator;

      FutureState(Future<T> *fut)
        : _self(fut),
          _value()
      {
      }

      void setValue(const ValueType &value)
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
        if (isReady())
        {
          if (hasError())
            p_interface->onFutureFailed(error(), data);
          else
          p_interface->onFutureFinished(_value, data);
        }
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

      const ValueType &value() const    { wait(); return _value; }
      ValueType &value()                { wait(); return _value; }

    private:
      Future<T>                        *_self;
      ValueType                         _value;
      std::vector<std::pair<FutureInterface<T> *, void *> > _callback;
      boost::mutex                                          _mutexCallback;
    };
  } // namespace detail

}

#endif  // _QIMESSAGING_DETAILS_FUTURE_HXX_
