/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/
#include <atomic>
#include <qi/strand.hpp>
#include <qi/log.hpp>
#include <qi/future.hpp>
#include <qi/getenv.hpp>
#include <deque>
#include <boost/enable_shared_from_this.hpp>

qiLogCategory("qi.strand");

namespace qi {

class StrandPrivate : public boost::enable_shared_from_this<StrandPrivate>
{
public:
  enum class State
  {
    None,
    Scheduled,
    Running,
    Canceled
    // we don't care about finished state
  };

  struct Callback
  {
    uint32_t id;
    State state;
    boost::function<void()> callback;
    qi::Promise<void> promise;
    qi::Future<void> asyncFuture;
  };

  typedef std::deque<boost::shared_ptr<Callback> > Queue;

  qi::ExecutionContext& _eventLoop;
  boost::atomic<unsigned int> _curId;
  boost::atomic<unsigned int> _aliveCount;
  bool _processing; // protected by mutex, no need for atomic
  boost::atomic<int> _processingThread;
  boost::mutex _mutex;
  boost::condition_variable _processFinished;
  bool _dying;
  Queue _queue;

  StrandPrivate(qi::ExecutionContext& eventLoop);
  ~StrandPrivate();

  Future<void> asyncAtImpl(boost::function<void()> cb, qi::SteadyClockTimePoint tp);
  Future<void> asyncDelayImpl(boost::function<void()> cb, qi::Duration delay);

  boost::shared_ptr<Callback> createCallback(boost::function<void()> cb);
  void enqueue(boost::shared_ptr<Callback> cbStruct);

  void process();
  void cancel(boost::shared_ptr<Callback> cbStruct);
};

StrandPrivate::StrandPrivate(qi::ExecutionContext& eventLoop)
  : _eventLoop(eventLoop)
  , _curId(0)
  , _aliveCount(0)
  , _processing(false)
  , _processingThread(0)
  , _dying(false)
{
}

StrandPrivate::~StrandPrivate()
{
}

boost::shared_ptr<StrandPrivate::Callback> StrandPrivate::createCallback(boost::function<void()> cb)
{
  ++_aliveCount;
  boost::shared_ptr<Callback> cbStruct = boost::make_shared<Callback>();
  cbStruct->id = ++_curId;
  cbStruct->state = State::None;
  cbStruct->callback = std::move(cb);
  return cbStruct;
}

Future<void> StrandPrivate::asyncAtImpl(boost::function<void()> cb, qi::SteadyClockTimePoint tp)
{
  boost::shared_ptr<Callback> cbStruct = createCallback(std::move(cb));
  cbStruct->promise =
    qi::Promise<void>(boost::bind(&StrandPrivate::cancel, this, cbStruct));
  qiLogDebug() << "Scheduling job id " << cbStruct->id
    << " at " << qi::to_string(tp);
  cbStruct->asyncFuture = _eventLoop.asyncAt(boost::bind(
        &StrandPrivate::enqueue, this, cbStruct),
      tp);
  return cbStruct->promise.future();
}

Future<void> StrandPrivate::asyncDelayImpl(boost::function<void()> cb, qi::Duration delay)
{
  boost::shared_ptr<Callback> cbStruct = createCallback(std::move(cb));
  cbStruct->promise =
    qi::Promise<void>(boost::bind(&StrandPrivate::cancel, this, cbStruct));
  qiLogDebug() << "Scheduling job id " << cbStruct->id
    << " in " << qi::to_string(delay);
  if (delay.count())
    cbStruct->asyncFuture = _eventLoop.asyncDelay(boost::bind(
          &StrandPrivate::enqueue, this, cbStruct),
        delay);
  else
    enqueue(cbStruct);
  return cbStruct->promise.future();
}

void StrandPrivate::enqueue(boost::shared_ptr<Callback> cbStruct)
{
  qiLogDebug() << "Enqueueing job id " << cbStruct->id;
  bool shouldschedule = false;

  {
    boost::mutex::scoped_lock lock(_mutex);
    // the callback may have been canceled
    if (cbStruct->state == State::None)
    {
      if (_dying)
      {
        cbStruct->promise.setError("the strand is dying");
        return;
      }

      _queue.push_back(cbStruct);
      cbStruct->state = State::Scheduled;
    }
    else
    {
      assert(cbStruct->state == State::Canceled);
      qiLogDebug() << "Job was canceled, dropping";
      return;
    }
    // if process was not scheduled yet, do it, there is work to do
    if (!_processing)
    {
      _processing = true;
      shouldschedule = true;
    }
  }

  if (shouldschedule)
  {
    qiLogDebug() << "StrandPrivate::process was not scheduled, doing it";
    _eventLoop.async(boost::bind(&StrandPrivate::process, shared_from_this()));
  }
}

void StrandPrivate::process()
{
  static const unsigned int QI_STRAND_QUANTUM_US =
    qi::os::getEnvDefault<unsigned int>("QI_STRAND_QUANTUM_US", 5000);

  qiLogDebug() << "StrandPrivate::process started";

  _processingThread = qi::os::gettid();

  qi::SteadyClockTimePoint start = qi::SteadyClock::now();

  bool finished = false;

  do
  {
    boost::shared_ptr<Callback> cbStruct;
    {
      boost::mutex::scoped_lock lock(_mutex);
      if (_dying)
      {
        qiLogDebug() << this << " strand is dying, stopping process";
        break;
      }

      assert(_processing);
      if (_queue.empty())
      {
        qiLogDebug() << "Queue empty, stopping";
        finished = true;
        break;
      }
      cbStruct = _queue.front();
      _queue.pop_front();
      if (cbStruct->state == State::Scheduled)
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
    boost::mutex::scoped_lock lock(_mutex);
    // if we still have work
    if (!finished && !_dying)
    {
      qiLogDebug() << "Strand quantum expired, rescheduling";
      lock.unlock();
      _eventLoop.async(boost::bind(&StrandPrivate::process, shared_from_this()));
    }
    else
    {
      _processing = false;
      _processFinished.notify_all();
    }
  }
}

void StrandPrivate::cancel(boost::shared_ptr<Callback> cbStruct)
{
  boost::mutex::scoped_lock lock(_mutex);

  switch (cbStruct->state)
  {
    case State::None:
      qiLogDebug() << "Not scheduled yet, canceling future";
      cbStruct->asyncFuture.cancel();
      cbStruct->state = State::Canceled;
      --_aliveCount;
      cbStruct->promise.setCanceled();
      break;
    case State::Scheduled:
      qiLogDebug() << "Was scheduled, removing it from queue";
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
        assert(erased);
        // Silence compile warning unused erased
        (void)erased;

      }
      --_aliveCount;
      cbStruct->promise.setCanceled();
      break;
    default:
      qiLogDebug() << "State is " << static_cast<int>(cbStruct->state)
        << ", too late for canceling";
      break;
  }
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
  join();
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
    boost::unique_lock<boost::mutex> lock(_p->_mutex);
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
      assert(task->state == StrandPrivate::State::Scheduled);
      task->promise.setError("the strand is dying");
      --prv->_aliveCount;
    }

    qiLogVerbose() << this << " joined, remaining tasks: " << prv->_aliveCount;
  }
}

Future<void> Strand::async(const boost::function<void()>& cb,
    qi::SteadyClockTimePoint tp)
{
  auto prv = boost::atomic_load(&_p);
  if (prv)
    return prv->asyncAtImpl(cb, tp);
  else
    return makeFutureError<void>("the strand is dying");
}

Future<void> Strand::async(const boost::function<void()>& cb,
    qi::Duration delay)
{
  auto prv = boost::atomic_load(&_p);
  if (prv)
    return prv->asyncDelayImpl(cb, delay);
  else
    return makeFutureError<void>("the strand is dying");
}

Future<void> Strand::asyncAtImpl(boost::function<void()> cb, qi::SteadyClockTimePoint tp)
{
  auto prv = boost::atomic_load(&_p);
  if (prv)
    return prv->asyncAtImpl(std::move(cb), tp);
  else
    return makeFutureError<void>("the strand is dying");
}

Future<void> Strand::asyncDelayImpl(boost::function<void()> cb, qi::Duration delay)
{
  auto prv = boost::atomic_load(&_p);
  if (prv)
    return prv->asyncDelayImpl(std::move(cb), delay);
  else
    return makeFutureError<void>("the strand is dying");
}

void Strand::postImpl(boost::function<void()> callback)
{
  auto prv = boost::atomic_load(&_p);
  if (prv)
    prv->enqueue(prv->createCallback(std::move(callback)));
}

bool Strand::isInThisContext()
{
  auto prv = boost::atomic_load(&_p);
  if (prv)
    return prv->_processingThread == qi::os::gettid();
  else
    return false;
}

}
