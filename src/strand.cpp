/*
**  Copyright (C) 2018 Softbank Robotics Europe
**  See COPYING for the license
*/
#include <atomic>
#include <boost/atomic.hpp>
#include <boost/thread/synchronized_value.hpp>
#include <boost/container/flat_map.hpp>

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

  // Executes the provided function and returns any caught exception's message if any.
  // Procedure<void()> Proc
  template<class Proc>
  Strand::OptionalErrorMessage safeInvoke(Proc&& proc) noexcept
  {
    return ka::invoke_catch(ka::exception_message_t{}, [&] {
      std::forward<Proc>(proc)();
      return Strand::OptionalErrorMessage{};
    });
  }

  // Tries to set a promise in error, absorbing the exception if the promise
  // was already set.
  void trySetError(Promise<void> prom, const std::string& error)
  {
    if (!prom.future().isFinished())
    {
      try
      {
        prom.setError(error);
      }
      catch (const FutureException& ex)
      {
        if (ex.state() != FutureException::ExceptionState_PromiseAlreadySet)
          throw;
      }
    }
  }

  static const auto dyingStrandMessage = "The strand is dying.";
}

  // Stores promises and set them in error on destruction if they are not removed before.
  class StrandPrivate::ScopedPromiseGroup
  {
    struct ErrorSetter
    {
      qi::Promise<void> promise;

      void operator()() {
        if (!promise.future().isFinished()) // That `if` could be removed but it reduces the chances of
                                            // ending with an exception thrown when setting the promise
        {
          promise.setError("Strand joining - deferred task promise broken");
        }
      }
    };

  public:
    ~ScopedPromiseGroup()
    {
      setAllInError();
    }

    // Registers a promise to be set in error once this object is destroyed.
    FutureUniqueId add(Promise<void> promise)
    {
      const auto id = promise.future().uniqueId();
      _promiseTerminators->emplace(id, ErrorSetter{std::move(promise)});
      return id;
    }

    // Removes a promise to avoid setting it in error.
    // Note: `id` should have been obtained through `add()`.
    void remove(FutureUniqueId id)
    {
      _promiseTerminators->erase(id);
    }

    // Sets all the currently registered promises in error and unregisters them.
    void setAllInError()
    {
      auto synchedTerminators = _promiseTerminators.synchronize();
      for (auto&& slot : *synchedTerminators)
      {
        auto errorMsg = safeInvoke([&]{
          // We move the setter out so that setter's resources are guaranteed
          // to be released as soon as invocation is terminated (whatever the issue).
          auto errorSetter = std::move(slot.second);
          errorSetter();
        });
        if (errorMsg)
        {
          qiLogWarning() << "Error when setting promise in error: " << *errorMsg;
        }
      }
      synchedTerminators->clear();
    }

  private:
    using FutureFuncMap = boost::container::flat_map<FutureUniqueId, ErrorSetter>;
    boost::synchronized_value<FutureFuncMap> _promiseTerminators;
  };

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


StrandPrivate::StrandPrivate(qi::ExecutionContext& executor)
  : _executor(executor)
  , _curId(0)
  , _aliveCount(0)
  , _processing(false)
  , _processingThread(0)
  , _dying(false)
  , _deferredTasksFutures{ std::make_shared<ScopedPromiseGroup>() }
{
}


StrandPrivate::~StrandPrivate() {
  const auto error = ka::invoke_catch(ka::exception_message_t{}, [this]{
    join();
    return Strand::OptionalErrorMessage{};
  });

  if (error)
  {
    qiLogWarning() << "Error while joining tasks in StrandPrivate destruction. "
      "Detail: " << *error;
  }
}

void StrandPrivate::join() QI_NOEXCEPT(true)
{
  if (joined)
  {
    qiLogDebug() << "Strand joining (" << this << ") -> already joined, ignored.";
    return;
  }

  boost::unique_lock<boost::recursive_mutex> lock(_mutex);
  qiLogDebug() << "Strand joining (" << this << ")...";

  _dying = true; // Starting from this point, either this thread or the processing thread will complete the joining.

  if (isInThisContext())
  {
    qiLogDebug() << "Strand joining (" << this << ") -> joining in the thread currently executing task:"
      << "deferring join until after task is finished...";
    return;
  }

  qiLogDebug() << "Strand joining (" << this << ") -> Joining starts : processing=" << _processing
    << ", size=" << _aliveCount << ")";

  qiLogDebug() << "Strand joining (" << this << ") -> clearing scheduled tasks...";
  for (auto&& task : _queue)
  {
    if (task->state == StrandPrivate::State::Canceled)
      continue;
    QI_ASSERT(task->state == StrandPrivate::State::Scheduled);

    const auto errorMsg = safeInvoke([&]{
      task->promise.setError(dyingStrandMessage);
    });
    if (errorMsg)
    {
      qiLogWarning() << "Error when setting promise in error: " << *errorMsg;
    }
  }
  _queue.clear();

  qiLogDebug() << "Strand joining (" << this << ") -> clearing deferred tasks...";
  _deferredTasksFutures.reset();

  qiLogDebug() << "Strand joining (" << this << ") -> waiting for currently executing task to finish...";
  _processFinished.wait(lock, [&]{ return !_processing; });

  qiLogDebug() << "Strand joining (" << this << ") -> DONE";
  joined = true;
}



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
  boost::recursive_mutex::scoped_lock lock(_mutex);
  if (_dying)
  {
    qiLogDebug() << this << " strand is dying, stopping defer call";
    return makeFutureError<void>(dyingStrandMessage);
  }

  boost::shared_ptr<Callback> cbStruct = createCallback(std::move(cb), options);

  cbStruct->promise = qi::Promise<void>(
    ka::scope_lock_proc(boost::bind(&StrandPrivate::cancel, this, cbStruct),
                        ka::mutable_store(weak_from_this())));

  qiLogDebug() << "Deferring job id " << cbStruct->id << " in " << qi::to_string(delay);
  if (delay.count())
  {
    cbStruct->asyncFuture = _executor.asyncDelay(track([=]{
      enqueue(cbStruct, options);
    }), delay, options).then(ka::constant_function());
    _deferredTasksFutures->add(cbStruct->promise);
  }
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

    if (_dying)
    {
      trySetError(cbStruct->promise, dyingStrandMessage);
      qiLogDebug() << "Strand is dying on job id " << cbStruct->id;
      return false;
    }

    auto scheduleCallback = [&] {
      _queue.push_back(cbStruct);
      cbStruct->state = State::Scheduled;
    };

    // the callback may have been canceled
    if (cbStruct->state == State::None)
    {
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
      if (cbStruct->asyncFuture.isValid())
      {
        _deferredTasksFutures->remove(cbStruct->promise.future().uniqueId());
      }
      return true;
    }

    return false;
  }();

  if (shouldschedule)
  {
    qiLogDebug() << "StrandPrivate::process was not scheduled, doing it";
    _executor.async(track([=]{ process(); }), options);
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
    _executor.async(track([=] { process(); }));
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

  if (_dying)
  {
    qiLogDebug() << this << " strand is dying, stopping task cancellation";
    trySetError(cbStruct->promise, dyingStrandMessage);
    return;
  }

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
  : _p(boost::make_shared<StrandPrivate>(*qi::getEventLoop()))
{
  qiLogDebug() << this << " new strand";
}

Strand::Strand(qi::ExecutionContext& eventloop)
  : _p(boost::make_shared<StrandPrivate>(eventloop))
{
}

Strand::~Strand()
{
  join();
}

void Strand::join() QI_NOEXCEPT(true)
{
  // keep it alive until we unlock the mutex
  auto pimpl = boost::atomic_exchange(&_p, {});
  QI_ASSERT_NULL(_p);
  if (!pimpl)
  {
    qiLogDebug() << this << " already joined";
    return;
  }
  pimpl->join();
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
  return ka::invoke_catch(ka::exception_message_t{}, [this]() {join(); return OptionalErrorMessage{};});
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
      }, ka::exception_message_t{});

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
