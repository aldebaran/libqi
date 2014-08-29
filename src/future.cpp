/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/
#include <qi/future.hpp>
#include <qi/log.hpp>
#include <qi/os.hpp>

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
      qi::Atomic<int>           _state;
      qi::Atomic<int>           _cancelRequested;
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
        _error()
    {
      _state = FutureState_None;
      _cancelRequested = false;
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
      return FutureState(*_p->_state);
    }

    static bool waitFinished(FutureBasePrivate* p)
    {
      return *p->_state != FutureState_Running;
    }

    FutureState FutureBase::wait(int msecs) const {
      boost::recursive_mutex::scoped_lock lock(_p->_mutex);
      if (msecs == FutureTimeout_Infinite)
        _p->_cond.wait(lock, boost::bind(&waitFinished, _p));
      else if (msecs > 0)
        _p->_cond.wait_for(lock, qi::MilliSeconds(msecs),
            boost::bind(&waitFinished, _p));
      // msecs <= 0 : do nothing just return the state
      return FutureState(*_p->_state);
    }

    FutureState FutureBase::wait(qi::Duration duration) const {
      boost::recursive_mutex::scoped_lock lock(_p->_mutex);
      _p->_cond.wait_for(lock, duration, boost::bind(&waitFinished, _p));
      return FutureState(*_p->_state);
    }

    FutureState FutureBase::wait(qi::SteadyClock::time_point timepoint) const {
      boost::recursive_mutex::scoped_lock lock(_p->_mutex);
      if (*_p->_state != FutureState_Running)
        return FutureState(*_p->_state);
      _p->_cond.wait_until(lock, timepoint, boost::bind(&waitFinished, _p));
      return FutureState(*_p->_state);
    }

    void FutureBase::reportValue() {
      //always set by setValue
      //boost::recursive_mutex::scoped_lock lock(_p->_mutex);
      _p->_state = FutureState_FinishedWithValue;
    }

    void FutureBase::requestCancel() {
      _p->_cancelRequested = true;
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
      _p->_state.setIfEquals(FutureState_None, FutureState_Running);
    }

    void FutureBase::notifyFinish() {
      _p->_cond.notify_all();
    }

    bool FutureBase::isFinished() const {
      FutureState v = FutureState(*_p->_state);
      return v == FutureState_FinishedWithValue || v == FutureState_FinishedWithError || v == FutureState_Canceled;
    }

    bool FutureBase::isRunning() const {
      return *_p->_state == FutureState_Running;
    }

    bool FutureBase::isCanceled() const {
      return *_p->_state == FutureState_Canceled;
    }

    bool FutureBase::isCancelRequested() const {
      return *_p->_cancelRequested;
    }

    bool FutureBase::hasError(int msecs) const {
      if (wait(msecs) == FutureState_Running)
        throw FutureException(FutureException::ExceptionState_FutureTimeout);
      return *_p->_state == FutureState_FinishedWithError;
    }

    bool FutureBase::hasValue(int msecs) const {
      if (wait(msecs) == FutureState_Running)
        throw FutureException(FutureException::ExceptionState_FutureTimeout);
      return *_p->_state == FutureState_FinishedWithValue;
    }

    const std::string &FutureBase::error(int msecs) const {
      if (wait(msecs) == FutureState_Running)
        throw FutureException(FutureException::ExceptionState_FutureTimeout);
      if (*_p->_state != FutureState_FinishedWithError)
        throw FutureException(FutureException::ExceptionState_FutureHasNoError);
      return _p->_error;
    }

    void FutureBase::reset() {
      boost::recursive_mutex::scoped_lock lock(_p->_mutex);
      //reset is called by the promise. So set the state to running.
      _p->_state = FutureState_Running;
      _p->_error = std::string();
      _p->_cancelRequested = false;
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

