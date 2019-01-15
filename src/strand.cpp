/*
**  Copyright (C) 2018 Softbank Robotics Europe
**  See COPYING for the license
*/
#include <atomic>
#include <boost/atomic.hpp>

#include <ka/errorhandling.hpp>
#include <qi/strand.hpp>
#include <qi/log.hpp>
#include <qi/future.hpp>
#include <qi/getenv.hpp>

qiLogCategory("qi.strand");

namespace qi {

namespace
{
  // Executes the callback immediately. This function returns a Future for consistency with the
  // async functions and simplicity of use. The future will be in error if the callback throws
  // an exception.
  // Procedure<void()> Proc
  template<typename Proc>
  Future<void> execNow(Proc&& proc, ExecutionOptions)
  {
    Promise<void> p;
    detail::setPromiseFromCallWithExceptionSupport(p, std::forward<Proc>(proc));
    return p.future();
  }
}

enum class StrandPrivate::State
{
  None,
  Scheduled,
  Running,
  Canceled,
  // we don't care about finished state
};

struct StrandPrivate::Callback
{
  uint32_t id;
  State state;
  boost::function<void()> callback;
  qi::Promise<void> promise;
  qi::Future<void> asyncFuture;
  ExecutionOptions executionOptions;
};

boost::shared_ptr<StrandPrivate::Callback> StrandPrivate::createCallback(boost::function<void()> cb, ExecutionOptions options)
{
  ++_aliveCount;
  boost::shared_ptr<Callback> cbStruct = boost::make_shared<Callback>();
  cbStruct->id = ++_curId;
  cbStruct->state = State::None;
  cbStruct->callback = std::move(cb);
  cbStruct->executionOptions = options;
  return cbStruct;
}

Future<void> StrandPrivate::asyncAtImpl(boost::function<void()> cb, qi::SteadyClockTimePoint tp, ExecutionOptions options)
{
  const auto now = SteadyClock::now();
  if (tp <= now && isInThisContext())
    return execNow(std::move(cb), options);
  return deferImpl(std::move(cb), tp - now, options);
}

Future<void> StrandPrivate::asyncDelayImpl(boost::function<void()> cb, qi::Duration delay, ExecutionOptions options)
{
  if (delay == qi::Duration::zero() && isInThisContext())
    return execNow(std::move(cb), options);
  return deferImpl(std::move(cb), delay, options);
}

Future<void> StrandPrivate::deferImpl(boost::function<void()> cb, qi::Duration delay, ExecutionOptions options)
{
  boost::shared_ptr<Callback> cbStruct = createCallback(std::move(cb), options);
  cbStruct->promise = qi::Promise<void>(boost::bind(&StrandPrivate::cancel, this, cbStruct));

  qiLogDebug() << "Deferring job id " << cbStruct->id << " in " << qi::to_string(delay);
  if (delay.count())
    cbStruct->asyncFuture = _executor.asyncDelay(boost::bind( &StrandPrivate::enqueue, this, cbStruct, options),
                                                  delay, options);
  else
    enqueue(cbStruct, options);
  return cbStruct->promise.future();
}

void StrandPrivate::enqueue(boost::shared_ptr<Callback> cbStruct, ExecutionOptions options)
{
  const bool shouldschedule = [&]()
  {
    boost::recursive_mutex::scoped_lock lock(_mutex);
    qiLogDebug() << "Enqueueing job id " << cbStruct->id;

    auto scheduleCallback = [&] {
      _queue.push_back(cbStruct);
      cbStruct->state = State::Scheduled;
    };

    // the callback may have been canceled
    if (cbStruct->state == State::None)
    {
      if (_dying)
      {
        cbStruct->promise.setError("the strand is dying");
        qiLogDebug() << "Strand is dying on job id " << cbStruct->id;
        return false;
      }

      qiLogDebug() << "Strand callback state is None on job id " << cbStruct->id;
      scheduleCallback();
    }
    else
    {
      QI_ASSERT(cbStruct->state == State::Canceled);
      if (options.onCancelRequested == CancelOption::NeverSkipExecution)
      {
        qiLogDebug() << "Job was canceled but is specified as never skipped - will execute";
        scheduleCallback();
      }
      else
      {
        qiLogDebug() << "Job was canceled, dropping";
        return false;
      }
    }
    // if process was not scheduled yet, do it, there is work to do
    if (!_processing)
    {
      qiLogDebug() << "Schedule process on job id " << cbStruct->id;
      _processing = true;
      return true;
    }

    return false;
  }();

  if (shouldschedule)
  {
    qiLogDebug() << "StrandPrivate::process was not scheduled, doing it";
    _executor.async(boost::bind(&StrandPrivate::process, shared_from_this()), options);
  }
}

void StrandPrivate::stopProcess(boost::recursive_mutex::scoped_lock& lock,
                                bool finished)
{
  // if we still have work
  if (!finished && !_dying)
  {
    qiLogDebug() << "Strand quantum expired, rescheduling";
    lock.unlock();
    _executor.async(boost::bind(&StrandPrivate::process, shared_from_this()));
  }
  else
  {
    _processing = false;
    _processFinished.notify_all();
  }
}

void StrandPrivate::process()
{
  static const unsigned int QI_STRAND_QUANTUM_US =
    qi::os::getEnvDefault<unsigned int>("QI_STRAND_QUANTUM_US", 5000);

  qiLogDebug() << "StrandPrivate::process started";

  _processingThread = qi::os::gettid();

  qi::SteadyClockTimePoint start = qi::SteadyClock::now();

  do
  {
    boost::shared_ptr<Callback> cbStruct;
    {
      boost::recursive_mutex::scoped_lock lock(_mutex);
      if (_dying)
      {
        qiLogDebug() << this << " strand is dying, stopping process";
        break;
      }

      QI_ASSERT(_processing);
      if (_queue.empty())
      {
        qiLogDebug() << "Queue empty, stopping";
        stopProcess(lock, true);
        _processingThread = 0;
        return;
      }
      cbStruct = _queue.front();
      _queue.pop_front();
      if (cbStruct->state == State::Scheduled
      || (cbStruct->state == State::Canceled && cbStruct->executionOptions.onCancelRequested == CancelOption::NeverSkipExecution))
      {
        --_aliveCount;
        cbStruct->state = State::Running;
      }
      else
      {
        // Job was canceled, cancel() already has done --_aliveCount
        qiLogDebug() << "Abandoning job id " << cbStruct->id
          << ", state: " << static_cast<int>(cbStruct->state);
        continue;
      }
    }
    qiLogDebug() << "Executing job id " << cbStruct->id;
    try {
      cbStruct->callback();
      cbStruct->promise.setValue(0);
    }
    catch (std::exception& e) {
      cbStruct->promise.setError(e.what());
    }
    catch (...) {
      cbStruct->promise.setError("callback has thrown in strand");
    }
    qiLogDebug() << "Finished job id " << cbStruct->id;
  } while (qi::SteadyClock::now() - start < qi::MicroSeconds(QI_STRAND_QUANTUM_US));

  _processingThread = 0;

  {
    boost::recursive_mutex::scoped_lock lock(_mutex);
    stopProcess(lock, false);
  }
}

void StrandPrivate::cancel(boost::shared_ptr<Callback> cbStruct)
{
  boost::recursive_mutex::scoped_lock lock(_mutex);

  switch (cbStruct->state)
  {
    case State::None:
      qiLogDebug() << "Not scheduled yet, canceling future";
      cbStruct->asyncFuture.cancel();
      cbStruct->state = State::Canceled;
      if(cbStruct->executionOptions.onCancelRequested != CancelOption::NeverSkipExecution)
      {
        --_aliveCount;
        cbStruct->promise.setCanceled();
      }
      break;
    case State::Scheduled:
      qiLogDebug() << "Was scheduled, removing it from queue";
      if (cbStruct->executionOptions.onCancelRequested != CancelOption::NeverSkipExecution)
      {
        bool erased = false;
        for (Queue::iterator iter = _queue.begin(); iter != _queue.end(); ++iter)
          if ((*iter)->id == cbStruct->id)
          {
            _queue.erase(iter);
            erased = true;
            break;
          }
        // state was scheduled, so the callback must be there
        QI_ASSERT(erased);
        // Silence compile warning unused erased
        (void)erased;

        --_aliveCount;
        cbStruct->promise.setCanceled();
      }
      else
      {
        cbStruct->state = State::Canceled;
      }
      break;
    default:
      qiLogDebug() << "State is " << static_cast<int>(cbStruct->state)
        << ", too late for canceling";
      break;
  }
}

bool StrandPrivate::isInThisContext() const
{
  return _processingThread == qi::os::gettid();
}

Strand::Strand()
  : _p(new StrandPrivate(*qi::getEventLoop()))
{
  qiLogDebug() << this << " new strand";
}

Strand::Strand(qi::ExecutionContext& eventloop)
  : _p(new StrandPrivate(eventloop))
{
}

Strand::~Strand()
{
  if (const auto error = join(std::nothrow))
  {
    qiLogWarning() << "Error while joining tasks in Strand destruction. "
                      "Detail: " << *error;
  }
}

namespace
{
  static const auto dyingStrandMessage = "the strand is dying";
}

void Strand::join()
{
  if (!_p)
  {
    qiLogDebug() << this << " already joined";
    return;
  }

  // keep it alive until we unlock the mutex
  boost::shared_ptr<StrandPrivate> prv;

  {
    boost::unique_lock<boost::recursive_mutex> lock(_p->_mutex);
    qiLogVerbose() << this << " joining (processing: " << _p->_processing
      << ", size: " << _p->_aliveCount << ")";

    _p->_dying = true;

    if (isInThisContext())
    {
      qiLogVerbose() << this << " joining from inside the context";
      // don't wait if we are joining the strand from within the strand
      return;
    }

    boost::atomic_exchange(&prv, _p);

    prv->_processFinished.wait(lock, [&]{ return !prv->_processing; });
    while (!prv->_queue.empty())
    {
      auto task = std::move(prv->_queue.front());
      prv->_queue.pop_front();
      if (task->state == StrandPrivate::State::Canceled)
        continue;
      QI_ASSERT(task->state == StrandPrivate::State::Scheduled);
      task->promise.setError(dyingStrandMessage);
      --prv->_aliveCount;
    }

    qiLogVerbose() << this << " joined, remaining tasks: " << prv->_aliveCount;
  }
}

/// Join all tasks, returning an error message if necessary instead of throwing
/// an exception.
///
/// Note: under extreme circumstances such as system memory exhaustion, this
///   method could still throw a `std::bad_alloc` exception, thus causing a call
///   to `std::terminate` because of the `noexcept` specifier. This behavior is
///   considered acceptable.
Strand::OptionalErrorMessage Strand::join(std::nothrow_t) QI_NOEXCEPT(true)
{
  // Catch any exception and return its message.
  return ka::invoke_catch(ka::exception_message{}, [this]() {join(); return OptionalErrorMessage{};});
}

Future<void> Strand::async(const boost::function<void()>& cb,
    qi::SteadyClockTimePoint tp)
{
  auto prv = boost::atomic_load(&_p);
  if (prv)
    return prv->asyncAtImpl(cb, tp);
  else
    return makeFutureError<void>(dyingStrandMessage);
}

Future<void> Strand::async(const boost::function<void()>& cb,
    qi::Duration delay)
{
  auto prv = boost::atomic_load(&_p);
  if (prv)
    return prv->asyncDelayImpl(cb, delay);
  else
    return makeFutureError<void>(dyingStrandMessage);
}

Future<void> Strand::asyncAtImpl(boost::function<void()> cb, qi::SteadyClockTimePoint tp, ExecutionOptions options)
{
  auto prv = boost::atomic_load(&_p);
  if (prv)
    return prv->asyncAtImpl(std::move(cb), tp, options);
  else
    return makeFutureError<void>(dyingStrandMessage);
}

Future<void> Strand::asyncDelayImpl(boost::function<void()> cb, qi::Duration delay, ExecutionOptions options)
{
  auto prv = boost::atomic_load(&_p);
  if (prv)
    return prv->asyncDelayImpl(std::move(cb), delay, options);
  else
    return makeFutureError<void>(dyingStrandMessage);
}

void Strand::postImpl(boost::function<void()> callback, ExecutionOptions options)
{
  auto prv = boost::atomic_load(&_p);
  if (prv)
  {
    // As no future will be returned, we need to at least log the user if a problem occured.
    prv->enqueue(prv->createCallback([=] {
      auto errorLogger = ka::compose([](const std::string& msg) {
        qiLogWarning() << "Uncaught error in task posted in a strand: " << msg;
      }, ka::exception_message{});

      ka::invoke_catch(std::move(errorLogger), callback);
    }, options), options);
  }
}

Future<void> Strand::defer(const boost::function<void ()>& cb, MicroSeconds delay, ExecutionOptions options)
{
  auto prv = boost::atomic_load(&_p);
  if (prv)
    return prv->deferImpl(cb, delay, options);
  else
    return makeFutureError<void>(dyingStrandMessage);
}

bool Strand::isInThisContext() const
{
  auto prv = boost::atomic_load(&_p);
  if (prv)
    return prv->isInThisContext();
  else
    return false;
}

}
