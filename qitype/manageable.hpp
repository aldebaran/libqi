#pragma once
/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/

#ifndef _QITYPE_MANAGEABLE_HPP_
#define _QITYPE_MANAGEABLE_HPP_

#include <qitype/api.hpp>

#include <boost/thread/mutex.hpp>

namespace qi {

  class ManageablePrivate;
  class EventLoop;

  /// Possible thread models for an object
  enum ObjectThreadingModel
  {
    /// Object is not thread safe, all method calls must occur in the same thread
    ObjectThreadingModel_SingleThread = 0,
    /// Object is thread safe, multiple calls can occur in different threads in parallel
    ObjectThreadingModel_MultiThread = 1
  };

  struct MethodStatistics
  {
    MethodStatistics()
    : cumulatedTime(0), minTime(0), maxTime(0), count(0) {}
    float cumulatedTime; //< Cumulated call durations in seconds
    float minTime;       //< Min call duration in seconds
    float maxTime;       //< Max call duration in seconds
    unsigned count;      //< Number of calls
  };

  typedef std::map<unsigned int, MethodStatistics> ObjectStatistics;
/** Per-instance context.
  */
  class QITYPE_API Manageable
  {
  public:
    Manageable();
    ~Manageable();
    Manageable(const Manageable& b);
    void operator = (const Manageable& b);


    /// Override all ThreadingModel and force dispatch to given event loop.
    void forceEventLoop(EventLoop* eventLoop);
    ///@return forced event loop or 0 if not set
    EventLoop* eventLoop() const;

    typedef boost::shared_ptr<boost::timed_mutex> TimedMutexPtr;
    ///@return the mutex associated with managed object.
    TimedMutexPtr mutex(); // non-recursive of course!

    /// @{
    /** Statistics gathering/retreiving API
     *
     */
    ///@return if statistics gatehering is enabled
    bool isStatsEnabled() const;
    /// Set statistics gathering status
    void enableStats(bool enable);
    /// Push statistics information about \p slotId.
    void pushStats(int slotId, float value);
    ObjectStatistics stats() const;
    /// Reset all statistical data
    void clearStats();

    /// @}

    ManageablePrivate* _p;
  };
}

#endif  // _QITYPE_MANAGEABLE_HPP_
