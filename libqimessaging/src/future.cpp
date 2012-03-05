/*
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2012 Aldebaran Robotics
*/

#include <qimessaging/future.hpp>
#include <boost/thread.hpp>

namespace qi {

  namespace detail {

    class FutureBasePrivate {
    public:
      FutureBasePrivate();
      boost::condition_variable _cond;
      boost::mutex              _mutex;
      bool                      _isReady;
    };

    FutureBasePrivate::FutureBasePrivate()
      : _cond(),
        _mutex(),
        _isReady(false)
    {
    }

    FutureBase::FutureBase()
      : _p(new FutureBasePrivate())
    {
    };

    bool FutureBase::waitForValue(int msecs) const {
      boost::mutex::scoped_lock lock(_p->_mutex);
      if (_p->_isReady)
        return true;
      if (msecs > 0)
        _p->_cond.timed_wait(lock, boost::posix_time::milliseconds(msecs));
      else
        _p->_cond.wait(lock);
      return _p->_isReady;
    }

    void FutureBase::setReady() {
      boost::mutex::scoped_lock lock(_p->_mutex);
      _p->_isReady = true;
      _p->_cond.notify_all();
    }

    bool FutureBase::isReady() const {
      return _p->_isReady;
    }


  }
}

