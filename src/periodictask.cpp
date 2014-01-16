/*
 * Copyright (c) 2013 Aldebaran Robotics. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the COPYING file.
 */

#include <boost/lexical_cast.hpp>
#include <boost/thread.hpp>

#include <qi/log.hpp>
#include <qi/periodictask.hpp>


qiLogCategory("qi.PeriodicTask");

enum TaskState
{
  Task_Stopped = 0,
  Task_Scheduled = 1, //< scheduled in an async()
  Task_Running = 2,   //< being executed
  Task_Rescheduling = 3, //< being rescheduled (protects _task)
  Task_Starting = 4, //< being started
  Task_Stopping = 5, //< stop requested
};

/* Transition matrix:
 Stopped      -> Starting [start()]
 Starting     -> Rescheduling [start()]
 Rescheduling -> Scheduled [start(), _wrap()]
 Scheduled    -> Running   [start()]
 Running      -> Rescheduling [ _wrap() ]
 Stopping     -> Stopped   [_wrap()]
 Runnig       -> Stopping [stop()]
 Scheduled    -> Stopping [stop()]

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

  struct PeriodicTaskPrivate
  {
    MethodStatistics        _callStats;
    qi::int64_t             _statsDisplayTime;
    PeriodicTask::Callback  _callback;
    qi::int64_t             _usPeriod;
    qi::Atomic<int>         _state;
    qi::Future<void>        _task;
    std::string             _name;
    bool                    _compensateCallTime;
  };

  PeriodicTask::PeriodicTask()
  {
    _p = new PeriodicTaskPrivate;
    _p->_usPeriod = -1;
    _p->_compensateCallTime =false;
    _p->_statsDisplayTime = qi::os::ustime();
    _p->_name = "PeriodicTask_" + boost::lexical_cast<std::string>(this);
  }


  PeriodicTask::~PeriodicTask()
  {
    stop();
    delete _p;
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
    _reschedule(immediate?0:_p->_usPeriod);
  }

  void PeriodicTask::_wrap()
  {
    if (*_p->_state == Task_Stopped)
      qiLogError()  << "PeriodicTask inconsistency: stopped from callback";
    /* To avoid being stuck because of unhandled transition, the rule is
    * that any other thread playing with our state can only do so
    * to stop us, and must eventualy reach the Stopping state
    */
    if (_p->_state.setIfEquals(Task_Stopping, Task_Stopped))
      return;
    /* reschedule() needs to call async() before reseting state from rescheduling
    *  to scheduled, to protect the _task object. So we might still be
    * in rescheduling state here.
    */
    while (*_p->_state == Task_Rescheduling)
      boost::this_thread::yield();
    if (!_p->_state.setIfEquals(Task_Scheduled, Task_Running))
    {
      setState(_p->_state, Task_Stopping, Task_Stopped);
      return;
    }
    bool shouldAbort = false;
    qi::int64_t wall = 0, now=0, delta=0;
    qi::int64_t usr, sys;
    bool compensate = _p->_compensateCallTime; // we don't want that bool to change in the middle
    try
    {
      wall = qi::os::ustime();
      std::pair<qi::int64_t, qi::int64_t> cpu = qi::os::cputime();
      _p->_callback();
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
      qiLogInfo() << "Exception in task callback: " << e.what();
      shouldAbort = true;
    }
    catch(...)
    {
      qiLogInfo() << "Unknown exception in task callback.";
      shouldAbort = true;
    }
    if (shouldAbort)
    {
      setState(_p->_state, Task_Stopping, Task_Stopped,
                       Task_Running, Task_Stopped);
      return;
    }
    else
    {
      _p->_callStats.push((float)wall / 1e6f, (float)usr / 1e6f, (float)sys / 1e6f);

      if (now - _p->_statsDisplayTime >= 20000000)
      {
        float secTime = float(now - _p->_statsDisplayTime) / 1e6f;
        _p->_statsDisplayTime = now;
        unsigned int count = _p->_callStats.count();
        std::string catName = "stats." + _p->_name;
        qiLogVerbose(catName.c_str())
          << (_p->_callStats.user().cumulatedValue() * 100.0 / secTime)
          << "%  "
          << count
          << "  " << _p->_callStats.wall().asString(count)
          << "  " << _p->_callStats.user().asString(count)
          << "  " << _p->_callStats.system().asString(count)
          ;
        _p->_callStats.reset();
      }

      if (!_p->_state.setIfEquals(Task_Running, Task_Rescheduling))
      { // If we are not in running state anymore, someone switched us
        // to stopping
        setState(_p->_state, Task_Stopping, Task_Stopped);
        return;
      }
      _reschedule(std::max((qi::int64_t)0, _p->_usPeriod - delta));
    }
  }
  void PeriodicTask::_reschedule(qi::int64_t delay)
  {
    qiLogDebug() << _p->_name <<" rescheduling in " << delay;
    _p->_task = getEventLoop()->async(boost::bind(&PeriodicTask::_wrap, this), delay);
    if (!_p->_state.setIfEquals(Task_Rescheduling, Task_Scheduled))
      qiLogError() << "PeriodicTask forbidden state change while rescheduling " << *_p->_state;
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
