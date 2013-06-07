#pragma once
/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/

#ifndef _QITYPE_MANAGEABLE_HPP_
#define _QITYPE_MANAGEABLE_HPP_

#include <qitype/api.hpp>
#include <qitype/functiontype.hpp>
#include <qitype/typeobject.hpp>
#include <qitype/signal.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/function.hpp>

#ifdef _MSC_VER
#  pragma warning( push )
#  pragma warning( disable: 4251 ) // dll interface
#endif

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

  struct EventTrace
  {
    enum EventKind
    {
      Event_Call = 1,
      Event_Result = 2,
      Event_Error = 3,
      Event_Signal = 4
    };
    EventTrace() {}
    EventTrace(unsigned int id, EventKind  kind, unsigned int slotId,
      const GenericValue& arguments, const qi::os::timeval timestamp)
    : id(id), kind(kind), slotId(slotId), arguments(arguments), timestamp(timestamp)
    {}

    unsigned int id; // trace id, used to match call and call result
    EventKind kind;
    unsigned int slotId; // method or signal id
    GenericValue arguments; // call or signal arguments
    qi::os::timeval timestamp;
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

    /// Emited each time a call starts and finishes, and for each signal trigger.
    Signal<EventTrace> traceObject;

    ///@return if trace mode is enabled
    bool isTraceEnabled() const;
    /** Set trace mode state.
    *
    * @warning This function should usually not be called directly.
    * Connecting/disconnecting to the traceObject signal automatically
    * enables/disables tracing mode.
    *
    * When enabled, all calls and signal triggers will be reported through the
    * "traceObject" signal.
    *
    */
    void enableTrace(bool enable);
    /// @}

    /// Starting id of features handled by Manageable
    static const uint32_t startId = 80;
    /// Stop id of features handled by Manageable
    static const uint32_t endId = 99;
    typedef std::map<unsigned int,
      std::pair<GenericFunction, MetaCallType>
    > MethodMap;
    typedef boost::function<SignalBase* (void*)> SignalGetter;
    typedef std::map<unsigned int, SignalGetter> SignalMap;
    SignalMap signalMap;
    /* Return the methods and signals defined at GenericObject level.
     * The 'this' argument must be the Manageable*.
    */
    static MethodMap&       manageableMmethodMap();
    static SignalMap&       manageableSignalMap();
    static MetaObject&      manageableMetaObject();
    static void             _build();
    int                     _nextTraceId();
    ManageablePrivate* _p;
  };
}

#ifdef _MSC_VER
#  pragma warning( pop )
#endif

QI_TYPE_ENUM_REGISTER(qi::EventTrace::EventKind);
#endif  // _QITYPE_MANAGEABLE_HPP_
