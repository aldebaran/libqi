/*
 * Copyright (c) 2013 Aldebaran Robotics. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the COPYING file.
 */

#include <boost/lexical_cast.hpp>
#include <boost/thread.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/chrono/chrono_io.hpp>
#include <qi/log.hpp>
#include <qi/periodictask.hpp>


qiLogCategory("qi.PeriodicTask");

// WARNING: if you add a state, review trigger() so that it stays lockfree
enum TaskState
{
  Task_Stopped = 0,
  Task_Scheduled = 1, //< scheduled in an async()
  Task_Running = 2,   //< being executed
  Task_Stopping = 5, //< stop requested
  Task_Triggering = 6, //< force trigger
  Task_TriggerReady = 7, //< force trigger (step 2)
};

/* Transition matrix:
 Stopped      -> Starting [start()]
 Starting     -> Rescheduling [start()]
 Rescheduling -> Scheduled [start(), _wrap()]
 Scheduled    -> Running   [start()]
 Running      -> Rescheduling [ _wrap() ]
 Stopping     -> Stopped   [stop(), _wrap(), trigger()]
 Running      -> Stopping [stop()]
 Scheduled    -> Stopping [stop()]
 Scheduled    -> Triggering [trigger()]
 Triggering   -> Running [_wrap()]
 Triggering   -> Rescheduling [_trigger()]

 - State Rescheduling is a lock on _state and on _task
*/

namespace qi
{

  struct PeriodicTaskPrivate :
    boost::enable_shared_from_this<PeriodicTaskPrivate>
  {
    MethodStatistics        _callStats;
    qi::SteadyClockTimePoint _statsDisplayTime;
    PeriodicTask::Callback  _callback;
    PeriodicTask::ScheduleCallback _scheduleCallback;
    qi::Duration            _period;
    TaskState               _state;
    qi::Future<void>        _task;
    std::string             _name;
    bool                    _compensateCallTime;
    int                     _tid;
    boost::mutex            _mutex;
    boost::condition_variable _cond;

    void _reschedule(qi::Duration delay = qi::Duration(0));
    void _wrap();
    void _onTaskFinished(const qi::Future<void>& fut);
  };
  static const int invalidThreadId = -1;
  PeriodicTask::PeriodicTask() :
    _p(new PeriodicTaskPrivate)
  {
    _p->_period = qi::Duration(-1);
    _p->_tid = invalidThreadId;
    _p->_compensateCallTime =false;
    _p->_statsDisplayTime = qi::SteadyClock::now();
    _p->_name = "PeriodicTask_" + boost::lexical_cast<std::string>(this);
    _p->_state = Task_Stopped;
  }


  PeriodicTask::~PeriodicTask()
  {
    stop();
  }

  void PeriodicTask::setName(const std::string& n)
  {
    _p->_name = n;
  }

  void PeriodicTask::setCallback(const Callback& cb)
  {
    if (_p->_callback)
      throw std::runtime_error("Callback already set");
    _p->_callback = cb;
  }

  void PeriodicTask::setStrand(qi::Strand* strand)
  {
    if (strand)
      _p->_scheduleCallback = boost::bind<qi::Future<void> >(
              static_cast<qi::Future<void>(qi::Strand::*)(const Callback&,
                qi::Duration)>(
                  &qi::Strand::async),
              strand, _1, _2);
    else
      _p->_scheduleCallback = ScheduleCallback();
  }

  void PeriodicTask::setUsPeriod(qi::int64_t usp)
  {
    if (usp<0)
      throw std::runtime_error("Period cannot be negative");
    _p->_period = qi::MicroSeconds(usp);
  }

  void PeriodicTask::setPeriod(qi::Duration period)
  {
    if (period < qi::Duration(0))
      throw std::runtime_error("Period cannot be negative");
    _p->_period = period;
  }

  void PeriodicTask::start(bool immediate)
  {
    if (!_p->_callback)
      throw std::runtime_error("Periodic task cannot start without a setCallback() call first");
    if (_p->_period < qi::Duration(0))
      throw std::runtime_error("Periodic task cannot start without a setPeriod() call first");
    // we are called from the callback
    if (os::gettid() == _p->_tid)
      return;

    boost::mutex::scoped_lock l(_p->_mutex);
    if (_p->_state != Task_Stopped)
    {
      qiLogDebug() << _p->_state << " task was not stopped";
      return; // Already running or being started.
    }
    _p->_reschedule(immediate ? qi::Duration(0) : _p->_period);
  }

  void PeriodicTask::asyncStop()
  {
    boost::mutex::scoped_lock l(_p->_mutex);
    // we are allowed to go from Scheduled and Running to Stopping
    // also handle multiple stop() calls
    while (_p->_state != Task_Stopping)
    {
      switch (_p->_state)
      {
      case Task_Scheduled:
        _p->_state = Task_Stopping;
        continue;
      case Task_Running:
        _p->_state = Task_Stopping;
        continue;
      case Task_Stopped:
        return;
      default:
        break;
      }
      _p->_cond.wait(l);
    }
    // We do not want to wait for callback to trigger. Since at this point
    // the callback (_wrap)  is not allowed to touch _task, we can just
    // cancel/wait it
    _p->_task.cancel();
  }

  void PeriodicTask::stop()
  {
    asyncStop();

    if (os::gettid() == _p->_tid)
      return;

    boost::mutex::scoped_lock l(_p->_mutex);
    while (_p->_state == Task_Stopping)
      _p->_cond.wait(l);
  }

  void PeriodicTask::trigger()
  {
    qiLogDebug() << "triggering";
    boost::mutex::scoped_lock l(_p->_mutex);
    if (_p->_state == Task_Scheduled)
    {
      _p->_state = Task_Triggering;
      _p->_task.cancel();
      while (_p->_state == Task_Triggering)
        _p->_cond.wait(l);
      if (_p->_state != Task_TriggerReady)
      {
        qiLogDebug() << "already triggered";
        return;
      }
      _p->_reschedule();
    }
  }

  void PeriodicTaskPrivate::_onTaskFinished(const qi::Future<void>& fut)
  {
    if (fut.isCanceled())
    {
      qiLogDebug() << "run canceled";
      boost::mutex::scoped_lock l(_mutex);
      if (_state == Task_Stopping)
        _state = Task_Stopped;
      else if (_state == Task_Triggering)
        _state = Task_TriggerReady;
      else
        assert(false && "state is not stopping nor triggering");
      _cond.notify_all();
    }
  }

  void PeriodicTaskPrivate::_reschedule(qi::Duration delay)
  {
    qiLogDebug() << "rescheduling in " << delay;
    if (_scheduleCallback)
      _task = _scheduleCallback(boost::bind(&PeriodicTaskPrivate::_wrap, shared_from_this()), delay);
    else
      _task = getEventLoop()->async(boost::bind(&PeriodicTaskPrivate::_wrap, shared_from_this()), delay);
    _state = Task_Scheduled;
    _task.connect(boost::bind(
          &PeriodicTaskPrivate::_onTaskFinished, shared_from_this(), _1));
  }

  void PeriodicTaskPrivate::_wrap()
  {
    qiLogDebug() << "callback start";
    {
      boost::mutex::scoped_lock l(_mutex);
      assert(_state != Task_Stopped);
      /* To avoid being stuck because of unhandled transition, the rule is
       * that any other thread playing with our state can only do so
       * to stop us, and must eventualy reach the Stopping state
       */
      if (_state == Task_Stopping)
      {
        _state = Task_Stopped;
        _cond.notify_all();
        return;
      }
      assert(_state == Task_Scheduled || _state == Task_Triggering);
      _state = Task_Running;
      _cond.notify_all();
    }
    bool shouldAbort = false;
    qi::SteadyClockTimePoint now;
    qi::Duration delta;
    qi::int64_t usr, sys;
    bool compensate = _compensateCallTime; // we don't want that bool to change in the middle
    try
    {
      qi::SteadyClockTimePoint start = qi::SteadyClock::now();
      std::pair<qi::int64_t, qi::int64_t> cpu = qi::os::cputime();
      _tid = os::gettid();
      _callback();
      _tid = invalidThreadId;
      now = qi::SteadyClock::now();
      delta = now - start;
      std::pair<qi::int64_t, qi::int64_t> cpu2 = qi::os::cputime();
      usr = cpu2.first - cpu.first;
      sys = cpu2.second - cpu.second;
    }
    catch (const std::exception& e)
    {
      qiLogInfo() << "Exception in task " << _name << ": " << e.what();
      shouldAbort = true;
    }
    catch(...)
    {
      qiLogInfo() << "Unknown exception in task callback.";
      shouldAbort = true;
    }
    if (shouldAbort)
    {
      qiLogDebug() << "should abort, bye";
      boost::mutex::scoped_lock l(_mutex);
      _state = Task_Stopped;
      _cond.notify_all();
      return;
    }
    else
    {
      _callStats.push(
          (float)boost::chrono::duration_cast<qi::MicroSeconds>(delta).count() / 1e6f,
          (float)usr / 1e6f,
          (float)sys / 1e6f);

      if (now - _statsDisplayTime >= qi::Seconds(20))
      {
        float secTime = float(boost::chrono::duration_cast<qi::MicroSeconds>(now - _statsDisplayTime).count()) / 1e6f;
        _statsDisplayTime = now;
        unsigned int count = _callStats.count();
        std::string catName = "stats." + _name;
        qiLogVerbose(catName.c_str())
          << (_callStats.user().cumulatedValue() * 100.0 / secTime)
          << "%  "
          << count
          << "  " << _callStats.wall().asString(count)
          << "  " << _callStats.user().asString(count)
          << "  " << _callStats.system().asString(count)
          ;
        _callStats.reset();
      }

      qiLogDebug() << "continuing";
      {
        boost::mutex::scoped_lock l(_mutex);
        if (_state != Task_Running)
        {
          qiLogDebug() << "continuing " << _state;
          assert(_state == Task_Stopping);
          _state = Task_Stopped;
          _cond.notify_all();
          return;
        }
        _reschedule(std::max(qi::Duration(0), _period - (compensate ? delta : qi::Duration(0))));
      }
    }
  }

  void PeriodicTask::compensateCallbackTime(bool enable)
  {
    _p->_compensateCallTime = enable;
  }

  bool PeriodicTask::isRunning() const
  {
    int s;
    {
      boost::mutex::scoped_lock l(_p->_mutex);
      s = _p->_state;
    }
    return s != Task_Stopped && s != Task_Stopping;
  }

  bool PeriodicTask::isStopping() const
  {
    int s;
    {
      boost::mutex::scoped_lock l(_p->_mutex);
      s = _p->_state;
    }
    return s == Task_Stopped || s == Task_Stopping;
  }
}
