/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/
#include <qi/future.hpp>
#include <qi/log.hpp>
#include <qi/os.hpp>
#include <qi/eventloop.hpp>

#include <boost/thread.hpp>
#include <boost/pool/singleton_pool.hpp>

namespace qi {

  namespace detail {
    class FutureBasePrivate {
    public:
      void* operator new(size_t);
      void operator delete(void*);
      FutureBasePrivate();
      boost::condition_variable_any _cond;
      boost::recursive_mutex    _mutex;
      std::string               _error;
      bool                      _isReady;
      bool                      _hasError;
    };

    struct FutureBasePrivatePoolTag { };
    typedef boost::singleton_pool<FutureBasePrivatePoolTag, sizeof(FutureBasePrivate)> futurebase_pool;

    void* FutureBasePrivate::operator new(size_t sz)
    {
      return futurebase_pool::malloc();
    }

    void FutureBasePrivate::operator delete(void* ptr)
    {
      futurebase_pool::free(ptr);
    }

    FutureBasePrivate::FutureBasePrivate()
      : _cond(),
        _mutex(),
        _error(),
        _isReady(false),
        _hasError(false)
    {
    }

    FutureBase::FutureBase()
      : _p(new FutureBasePrivate())
    {
    }

    FutureBase::~FutureBase()
    {
      delete _p;
    };

    bool FutureBase::wait(int msecs) const {
      static bool detectEventLoopWait = !os::getenv("QI_DETECT_FUTURE_WAIT_FROM_NETWORK_EVENTLOOP").empty();
      if (detectEventLoopWait && getDefaultNetworkEventLoop()->isInEventLoopThread())
        qiLogWarning("qi.future") << "Future wait in network thread.";
      boost::recursive_mutex::scoped_lock lock(_p->_mutex);
      if (_p->_isReady || _p->_hasError)
        return true;
      if (msecs > 0)
        _p->_cond.timed_wait(lock, boost::posix_time::milliseconds(msecs));
      else if (msecs < 0)
        {} // Do nothing, just return state
      else
        _p->_cond.wait(lock);
      return _p->_isReady || _p->_hasError;
    }

    void FutureBase::reportReady() {
      boost::recursive_mutex::scoped_lock lock(_p->_mutex);
      _p->_isReady = true;
    }

    void FutureBase::notifyReady() {
      _p->_cond.notify_all();
    }

    void FutureBase::reportError(const std::string &message) {
      boost::recursive_mutex::scoped_lock lock(_p->_mutex);
      _p->_hasError = true;
      _p->_isReady = true;
      _p->_error = message;
    }

    bool FutureBase::isReady() const {
      return _p->_isReady;
    }

    bool FutureBase::hasError(int msecs) const {
      wait(msecs);
      return _p->_hasError;
    }

    const std::string &FutureBase::error() const {
      wait();
      return _p->_error;
    }


    void FutureBase::reset() {
      boost::recursive_mutex::scoped_lock lock(_p->_mutex);
      _p->_isReady = false;
      _p->_error = std::string();
      _p->_hasError = false;
    }

    boost::recursive_mutex& FutureBase::mutex()
    {
      return _p->_mutex;
    }
  }

}

