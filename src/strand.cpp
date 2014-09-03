/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/
#include <qi/strand.hpp>
#include <qi/log.hpp>
#include <qi/future.hpp>
#include <deque>
#include <boost/enable_shared_from_this.hpp>

qiLogCategory("qi.strand");

namespace qi {

class StrandPrivate : public boost::enable_shared_from_this<StrandPrivate>
{
public:
  enum State
  {
    State_None,
    State_Scheduled,
    State_Running,
    State_Canceled
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
  Queue _queue;

  StrandPrivate(qi::ExecutionContext& eventLoop);
  ~StrandPrivate();

  Future<void> async(boost::function<void()> cb, qi::SteadyClockTimePoint tp);
  Future<void> async(boost::function<void()> cb, qi::Duration delay);

  boost::shared_ptr<Callback> createCallback(
      boost::function<void()> cb);
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
{
}

StrandPrivate::~StrandPrivate()
{
}

boost::shared_ptr<StrandPrivate::Callback> StrandPrivate::createCallback(
    boost::function<void()> cb)
{
  ++_aliveCount;
  boost::shared_ptr<Callback> cbStruct = boost::make_shared<Callback>();
  cbStruct->id = ++_curId;
  cbStruct->state = State_None;
  cbStruct->callback = cb;
  return cbStruct;
}

Future<void> StrandPrivate::async(boost::function<void()> cb,
    qi::SteadyClockTimePoint tp)
{
  boost::shared_ptr<Callback> cbStruct = createCallback(cb);
  cbStruct->promise =
    qi::Promise<void>(boost::bind(&StrandPrivate::cancel, this, cbStruct));
  qiLogDebug() << "Scheduling job id " << cbStruct->id
    << " at " << tp;
  cbStruct->asyncFuture = _eventLoop.async(boost::bind(
        &StrandPrivate::enqueue, this, cbStruct),
      tp);
  return cbStruct->promise.future();
}

Future<void> StrandPrivate::async(boost::function<void()> cb,
    qi::Duration delay)
{
  boost::shared_ptr<Callback> cbStruct = createCallback(cb);
  cbStruct->promise =
    qi::Promise<void>(boost::bind(&StrandPrivate::cancel, this, cbStruct));
  qiLogDebug() << "Scheduling job id " << cbStruct->id
    << " in " << delay;
  if (delay.count())
    cbStruct->asyncFuture = _eventLoop.async(boost::bind(
          &StrandPrivate::enqueue, this, cbStruct),
        delay);
  else
    enqueue(cbStruct);
  return cbStruct->promise.future();
}

void StrandPrivate::enqueue(boost::shared_ptr<Callback> cbStruct)
{
  qiLogDebug() << "Enqueueing job id " << cbStruct->id;
  boost::mutex::scoped_lock lock(_mutex);
  // the callback may have been canceled
  if (cbStruct->state == State_None)
  {
    _queue.push_back(cbStruct);
    cbStruct->state = State_Scheduled;
  }
  else
    qiLogDebug() << "Job is not schedulable, state " << (int)cbStruct->state;
  // if process was not scheduled yet, do it, there is work to do
  if (!_processing)
  {
    qiLogDebug() << "StrandPrivate::process was not scheduled, doing it";
    _processing = true;
    _eventLoop.async(boost::bind(&StrandPrivate::process, shared_from_this()),
        qi::Duration(0));
  }
}

void StrandPrivate::process()
{
  qiLogDebug() << "StrandPrivate::process started";

  _processingThread = qi::os::gettid();

  while (true)
  {
    boost::shared_ptr<Callback> cbStruct;
    {
      boost::mutex::scoped_lock lock(_mutex);
      assert(_processing);
      if (_queue.empty())
      {
        qiLogDebug() << "Queue empty, stopping";
        _processing = false;
        _processFinished.notify_all();
        return;
      }
      cbStruct = _queue.front();
      _queue.pop_front();
      if (cbStruct->state == State_Scheduled)
      {
        --_aliveCount;
        cbStruct->state = State_Running;
      }
      else
      {
        // Job was canceled, cancel() already has done --_aliveCount
        qiLogDebug() << "Abandoning job id " << cbStruct->id
          << ", state: " << cbStruct->state;
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
  }

  _processingThread = 0;
}

void StrandPrivate::cancel(boost::shared_ptr<Callback> cbStruct)
{
  boost::mutex::scoped_lock lock(_mutex);

  switch (cbStruct->state)
  {
    case State_None:
      qiLogDebug() << "Not scheduled yet, canceling future";
      cbStruct->asyncFuture.cancel();
      cbStruct->state = State_Canceled;
      --_aliveCount;
      cbStruct->promise.setCanceled();
      break;
    case State_Scheduled:
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
      }
      --_aliveCount;
      cbStruct->promise.setCanceled();
      break;
    default:
      qiLogDebug() << "State is " << cbStruct->state
        << ", too late for canceling";
      break;
  }
}

Strand::Strand()
  : _p(new StrandPrivate(*qi::getEventLoop()))
{
}

Strand::Strand(qi::ExecutionContext& eventloop)
  : _p(new StrandPrivate(eventloop))
{
}

Strand::~Strand()
{
  if (isInThisContext())
  {
    // don't wait if we are destroying the strand from within the strand
    return;
  }

  boost::unique_lock<boost::mutex> lock(_p->_mutex);
  qiLogVerbose() << this << " Dying (processing: " << _p->_processing
    << ", size: " << _p->_aliveCount << ")";
  while (_p->_processing || _p->_aliveCount)
  {
    _p->_processFinished.wait(lock);
    qiLogVerbose() << this << " Still dying (processing: " << _p->_processing
      << ", size: " << _p->_aliveCount << ")";
  }
}

Future<void> Strand::async(const boost::function<void()>& cb,
    qi::SteadyClockTimePoint tp)
{
  return _p->async(cb, tp);
}

Future<void> Strand::async(const boost::function<void()>& cb,
    qi::Duration delay)
{
  return _p->async(cb, delay);
}

void Strand::post(const boost::function<void()>& callback)
{
  _p->enqueue(_p->createCallback(callback));
}

bool Strand::isInThisContext()
{
  return _p->_processingThread == qi::os::gettid();
}

}
