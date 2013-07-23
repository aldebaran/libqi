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

qiLogCategory("qi.future");

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
      FutureState               _state;
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
        _state(FutureState_None)
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

    FutureState FutureBase::state() const
    {
      return _p->_state;
    }

    FutureState FutureBase::wait(int msecs) const {
      static bool detectEventLoopWait = !os::getenv("QI_DETECT_FUTURE_WAIT_FROM_NETWORK_EVENTLOOP").empty();
      if (detectEventLoopWait && getDefaultNetworkEventLoop()->isInEventLoopThread())
        qiLogWarning() << "Future wait in network thread.";
      boost::recursive_mutex::scoped_lock lock(_p->_mutex);
      if (_p->_state != FutureState_Running)
        return _p->_state;
      if (msecs == FutureTimeout_Infinite)
        _p->_cond.wait(lock);
      else if (msecs > 0)
        _p->_cond.timed_wait(lock, boost::posix_time::milliseconds(msecs));
      // msecs <= 0 : do nothing just return the state
      return _p->_state;
    }

    void FutureBase::reportValue() {
      //always set by setValue
      //boost::recursive_mutex::scoped_lock lock(_p->_mutex);
      _p->_state = FutureState_FinishedWithValue;
    }

    void FutureBase::reportCanceled() {
      //always set by setCanceled
      //boost::recursive_mutex::scoped_lock lock(_p->_mutex);
      _p->_state = FutureState_Canceled;
    }

    void FutureBase::reportError(const std::string &message) {
      //always set by setError
      //boost::recursive_mutex::scoped_lock lock(_p->_mutex);
      _p->_state = FutureState_FinishedWithError;
      _p->_error = message;
    }

    void FutureBase::reportStart() {
      boost::recursive_mutex::scoped_lock lock(_p->_mutex);
      _p->_state = FutureState_Running;
    }

    void FutureBase::notifyFinish() {
      _p->_cond.notify_all();
    }

    bool FutureBase::isFinished() const {
      return _p->_state == FutureState_FinishedWithValue || _p->_state == FutureState_FinishedWithError || _p->_state == FutureState_Canceled;
    }

    bool FutureBase::isRunning() const {
      return _p->_state == FutureState_Running;
    }

    bool FutureBase::isCanceled() const {
      return _p->_state == FutureState_Canceled;
    }

    bool FutureBase::hasError(int msecs) const {
      if (wait(msecs) == FutureState_Running)
        throw FutureException(FutureException::ExceptionState_FutureTimeout);
      return _p->_state == FutureState_FinishedWithError;
    }

    bool FutureBase::hasValue(int msecs) const {
      if (wait(msecs) == FutureState_Running)
        throw FutureException(FutureException::ExceptionState_FutureTimeout);
      return _p->_state == FutureState_FinishedWithValue;
    }

    const std::string &FutureBase::error(int msecs) const {
      if (wait(msecs) == FutureState_Running)
        throw FutureException(FutureException::ExceptionState_FutureTimeout);
      if (_p->_state != FutureState_FinishedWithError)
        throw FutureException(FutureException::ExceptionState_FutureHasNoError);
      return _p->_error;
    }

    void FutureBase::reset() {
      boost::recursive_mutex::scoped_lock lock(_p->_mutex);
      //reset is called by the promise. So set the state to running.
      _p->_state = FutureState_Running;
      _p->_error = std::string();
    }

    boost::recursive_mutex& FutureBase::mutex()
    {
      return _p->_mutex;
    }
  }

  std::string FutureException::stateToString(const ExceptionState &es) {
    switch (es) {
    case ExceptionState_FutureTimeout:
      return "Future timeout.";
    case ExceptionState_FutureCanceled:
      return "Future canceled.";
    case ExceptionState_FutureNotCancelable:
      return "Future is not cancelable.";
    case ExceptionState_FutureHasNoError:
      return "Future has no error.";
    //use the specified string instead.
    case ExceptionState_FutureUserError:
      return "";
    case ExceptionState_PromiseAlreadySet:
      return "Future has already been set.";
    }
    return "";
  }

}

