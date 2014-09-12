/*
 * Copyright (c) 2013 Aldebaran Robotics. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the COPYING file.
 */

#include <boost/lexical_cast.hpp>
#include <boost/thread.hpp>
#include <boost/enable_shared_from_this.hpp>

#include <qi/log.hpp>
#include <qi/periodictask.hpp>


qiLogCategory("qi.PeriodicTask");

// WARNING: if you add a state, review trigger() so that it stays lockfree
enum TaskState
{
  Task_Stopped = 0,
  Task_Scheduled = 1, //< scheduled in an async()
  Task_Running = 2,   //< being executed
  Task_Rescheduling = 3, //< being rescheduled (protects _task)
  Task_Starting = 4, //< being started
  Task_Stopping = 5, //< stop requested
  Task_Triggering = 6, //< force trigger
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

// Persist trying to transition state, log if it takes too long, but never abort
inline void setState(qi::Atomic<int>& state, TaskState from, TaskState to)
{
  for (unsigned i=0; i<1000; ++i)
    if (state.setIfEquals(from, to))
      return;
  while (true)
  {
    for (unsigned i=0; i<1000; ++i)
    {
      if (state.setIfEquals(from, to))
        return;
      qi::os::msleep(1); // TODO: 1ms is probably too long
    }
    qiLogWarning() << "PeriodicTask is stuck " << from << ' ' << to << ' ' << *state;
  }
}

inline void setState(qi::Atomic<int>& state, TaskState from, TaskState to, TaskState from2, TaskState to2)
{
  for (unsigned i=0; i<1000; ++i)
    if (state.setIfEquals(from, to) || state.setIfEquals(from2, to2))
      return;
  while (true)
  {
    for (unsigned i=0; i<1000; ++i)
    {
      if (state.setIfEquals(from, to) || state.setIfEquals(from2, to2))
        return;
      qi::os::msleep(1); // TODO: 1ms is probably too long
    }
    qiLogWarning() << "PeriodicTask is stuck " << from << ' ' << to << ' '  << from2 << ' ' << to2 << ' '<< *state;
  }
}

namespace qi
{

  struct PeriodicTaskPrivate :
    boost::enable_shared_from_this<PeriodicTaskPrivate>
  {
    MethodStatistics        _callStats;
    qi::int64_t             _statsDisplayTime;
    PeriodicTask::Callback  _callback;
    PeriodicTask::ScheduleCallback _scheduleCallback;
    qi::int64_t             _usPeriod;
    qi::Atomic<int>         _state;
    qi::Future<void>        _task;
    std::string             _name;
    bool                    _compensateCallTime;
    int                     _tid;

    void _reschedule(qi::int64_t delay);
    void _wrap();
    void _trigger(qi::Future<void> future);
  };
  static const int invalidThreadId = -1;
  PeriodicTask::PeriodicTask() :
    _p(new PeriodicTaskPrivate)
  {
    _p->_usPeriod = -1;
    _p->_tid = invalidThreadId;
    _p->_compensateCallTime =false;
    _p->_statsDisplayTime = qi::os::ustime();
    _p->_name = "PeriodicTask_" + boost::lexical_cast<std::string>(this);
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
    _p->_usPeriod = usp;
  }

  void PeriodicTask::start(bool immediate)
  {
    if (!_p->_callback)
      throw std::runtime_error("Periodic task cannot start without a setCallback() call first");
    if (_p->_usPeriod < 0)
      throw std::runtime_error("Periodic task cannot start without a setUsPeriod() call first");
    //Stopping is not handled by start, stop will handle it for us.
    stop();
    if (!_p->_state.setIfEquals(Task_Stopped, Task_Starting))
      return; // Already running or being started.
    if (!_p->_state.setIfEquals(Task_Starting, Task_Rescheduling))
      qiLogError() << "Periodic task internal error while starting";
    _p->_reschedule(immediate?0:_p->_usPeriod);
  }

  void PeriodicTask::trigger()
  {
    while (true)
    {
      if (_p->_state.setIfEquals(Task_Stopped, Task_Stopped) ||
          _p->_state.setIfEquals(Task_Stopping, Task_Stopping) ||
          _p->_state.setIfEquals(Task_Starting, Task_Starting) ||
          _p->_state.setIfEquals(Task_Running, Task_Running) ||
          _p->_state.setIfEquals(Task_Rescheduling, Task_Rescheduling) ||
          _p->_state.setIfEquals(Task_Triggering, Task_Triggering))
        return;
      if (_p->_state.setIfEquals(Task_Scheduled, Task_Triggering))
      {
        _p->_task.cancel();
        _p->_task.connect(&PeriodicTaskPrivate::_trigger, _p, _1,
            FutureCallbackType_Sync);
        return;
      }
    }
  }

  void PeriodicTaskPrivate::_trigger(qi::Future<void> future)
  {
    // if future was not canceled, the task already ran, don't retrigger
    if (!future.isCanceled())
      return;

    // else, start the task now if we are still triggering
    if (_state.setIfEquals(Task_Triggering, Task_Rescheduling))
      _reschedule(0);
  }

  void PeriodicTaskPrivate::_wrap()
  {
    if (*_state == Task_Stopped)
      qiLogError()  << "PeriodicTask inconsistency: stopped from callback";
    /* To avoid being stuck because of unhandled transition, the rule is
    * that any other thread playing with our state can only do so
    * to stop us, and must eventualy reach the Stopping state
    */
    if (_state.setIfEquals(Task_Stopping, Task_Stopped))
      return;
    /* reschedule() needs to call async() before reseting state from rescheduling
    *  to scheduled, to protect the _task object. So we might still be
    * in rescheduling state here.
    */
    while (*_state == Task_Rescheduling)
      boost::this_thread::yield();
    // order matters! check scheduled state first as the state cannot change
    // from triggering to scheduled but can change in the other way
    if (!_state.setIfEquals(Task_Scheduled, Task_Running) &&
        !_state.setIfEquals(Task_Triggering, Task_Running))
    {
      setState(_state, Task_Stopping, Task_Stopped);
      return;
    }
    bool shouldAbort = false;
    qi::int64_t wall = 0, now=0, delta=0;
    qi::int64_t usr, sys;
    bool compensate = _compensateCallTime; // we don't want that bool to change in the middle
    try
    {
      wall = qi::os::ustime();
      std::pair<qi::int64_t, qi::int64_t> cpu = qi::os::cputime();
      _tid = os::gettid();
      _callback();
      _tid = invalidThreadId;
      now = qi::os::ustime();
      wall = now - wall;
      std::pair<qi::int64_t, qi::int64_t> cpu2 = qi::os::cputime();
      usr = cpu2.first - cpu.first;
      sys = cpu2.second - cpu.second;
      if (compensate)
        delta = wall;
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
      setState(_state, Task_Stopping, Task_Stopped,
                       Task_Running, Task_Stopped);
      return;
    }
    else
    {
      _callStats.push((float)wall / 1e6f, (float)usr / 1e6f, (float)sys / 1e6f);

      if (now - _statsDisplayTime >= 20000000)
      {
        float secTime = float(now - _statsDisplayTime) / 1e6f;
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

      if (!_state.setIfEquals(Task_Running, Task_Rescheduling))
      { // If we are not in running state anymore, someone switched us
        // to stopping
        setState(_state, Task_Stopping, Task_Stopped);
        return;
      }
      _reschedule(std::max((qi::int64_t)0, _usPeriod - delta));
    }
  }
  void PeriodicTaskPrivate::_reschedule(qi::int64_t delay)
  {
    qiLogDebug() << _name <<" rescheduling in " << delay;
    if (_scheduleCallback)
      _task = _scheduleCallback(boost::bind(&PeriodicTaskPrivate::_wrap, shared_from_this()), qi::MicroSeconds(delay));
    else
      _task = getEventLoop()->async(boost::bind(&PeriodicTaskPrivate::_wrap, shared_from_this()), delay);
    if (!_state.setIfEquals(Task_Rescheduling, Task_Scheduled))
      qiLogError() << "PeriodicTask forbidden state change while rescheduling " << *_state;
  }

  void PeriodicTask::asyncStop()
  {
    if (_p->_state.setIfEquals(Task_Stopped, Task_Stopped))
      return;
    // we are allowed to go from Scheduled and Running to Stopping
    // also handle multiple stop() calls
    while (!_p->_state.setIfEquals(Task_Scheduled , Task_Stopping) &&
           !_p->_state.setIfEquals(Task_Running, Task_Stopping) &&
           !_p->_state.setIfEquals(Task_Stopped, Task_Stopped) &&
           !_p->_state.setIfEquals(Task_Stopping, Task_Stopping))
      boost::this_thread::yield();
    // We do not want to wait for callback to trigger. Since at this point
    // the callback (_wrap)  is not allowed to touch _task, we can just cancel/wait it
    try
    {
      _p->_task.cancel();
    }
    catch(...)
    {}
  }

  void PeriodicTask::stop()
  {
    asyncStop();
    if (os::gettid() == _p->_tid)
      return;
    try
    {
      _p->_task.wait();
    }
    catch (...) {}

    // So here state can be stopping (callback was aborted) or stopped
    // We set to stopped either way to be ready for restart.
    if (!_p->_state.setIfEquals(Task_Stopping , Task_Stopped) &&
        !_p->_state.setIfEquals(Task_Stopped, Task_Stopped))
      qiLogError() << "PeriodicTask inconsistency, expected Stopped, got " << *_p->_state;
  }

  void PeriodicTask::compensateCallbackTime(bool enable)
  {
    _p->_compensateCallTime = enable;
  }

  bool PeriodicTask::isRunning() const
  {
    int s = *_p->_state;
    return s != Task_Stopped && s!= Task_Stopping;
  }

  bool PeriodicTask::isStopping() const
  {
    int s = *_p->_state;
    return s == Task_Stopped || s == Task_Stopping;
  }
}
