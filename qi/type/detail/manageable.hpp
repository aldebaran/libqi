#pragma once
/*
**  Copyright (C) 2013 Aldebaran Robotics
**  See COPYING for the license
*/

#ifndef _QITYPE_MANAGEABLE_HPP_
#define _QITYPE_MANAGEABLE_HPP_

#include <memory>
#include <algorithm>
#include <boost/function.hpp>


#include <qi/stats.hpp>

#include <qi/api.hpp>
#include <qi/anyfunction.hpp>
#include <qi/type/typeobject.hpp>
#include <qi/signal.hpp>

#ifdef _MSC_VER
#  pragma warning( push )
#  pragma warning( disable: 4251 ) // dll interface
#endif

namespace qi {

  class ManageablePrivate;
  class ExecutionContext;

  /// Possible thread models for an object
  enum ObjectThreadingModel
  {
    /// Object is not thread safe, all method calls must occur in the same thread
    ObjectThreadingModel_SingleThread = 0,
    /// Object is thread safe, multiple calls can occur in different threads in parallel
    ObjectThreadingModel_MultiThread = 1,

    ObjectThreadingModel_Default = ObjectThreadingModel_SingleThread
  };


  class EventTrace
  {
  public:
    enum EventKind
    {
      Event_Call = 1,
      Event_Result = 2,
      Event_Error = 3,
      Event_Signal = 4
    };
    EventTrace() {}
    EventTrace(unsigned int id, EventKind  kind, unsigned int slotId,
      const AnyValue& arguments, const qi::os::timeval timestamp,
      qi::int64_t userUsTime=0, qi::int64_t systemUsTime=0,
      unsigned int callerContext=0, unsigned int calleeContext=0,
      qi::os::timeval postTimestamp = qi::os::timeval())
    : _id(id), _kind(kind), _slotId(slotId), _arguments(arguments),
      _timestamp(timestamp), _postTimestamp(postTimestamp), _userUsTime(userUsTime), _systemUsTime(systemUsTime),
      _callerContext(callerContext), _calleeContext(calleeContext)
    {}

    // trace id, used to match call and call result
    const unsigned int&     id()              const { return _id;}
    const EventKind&        kind()            const { return _kind;}
    // method or signal id
    const unsigned int&     slotId()          const { return _slotId;}
    // call or signal arguments
    const AnyValue&         arguments()       const { return _arguments;}
    const qi::os::timeval&  timestamp()       const { return _timestamp;}
    const qi::os::timeval&  postTimestamp()   const { return _postTimestamp;}
    const qi::int64_t&      userUsTime()      const { return _userUsTime;}
    const qi::int64_t&      systemUsTime()    const { return _systemUsTime;}
    const unsigned int&     callerContext()   const { return _callerContext;}
    const unsigned int&     calleeContext()   const { return _calleeContext;}

  private:
    unsigned int     _id; // trace id, used to match call and call result
    EventKind        _kind;
    unsigned int     _slotId; // method or signal id
    AnyValue         _arguments; // call or signal arguments
    qi::os::timeval  _timestamp;
    qi::os::timeval  _postTimestamp; // timestamp of eventual post call
    qi::int64_t      _userUsTime;
    qi::int64_t      _systemUsTime;
    unsigned int     _callerContext; // context of caller function
    unsigned int     _calleeContext; // context where method runs
  };

}

QI_TYPE_STRUCT_AGREGATE_CONSTRUCTOR(qi::MinMaxSum,
  ("minValue",       minValue),
  ("maxValue",       maxValue),
  ("cumulatedValue", cumulatedValue));

QI_TYPE_STRUCT_AGREGATE_CONSTRUCTOR(qi::MethodStatistics,
  ("count",  count),
  ("wall",   wall),
  ("user",   user),
  ("system", system));

QI_TYPE_STRUCT_AGREGATE_CONSTRUCTOR(qi::EventTrace,
  ("id",            id),
  ("kind",          kind),
  ("slotId",        slotId),
  ("arguments",     arguments),
  ("timestamp",     timestamp),
  ("userUsTime",    userUsTime),
  ("systemUsTime",  systemUsTime),
  ("callerContext", callerContext),
  ("calleeContext", calleeContext));

QI_TYPE_STRUCT(qi::os::timeval, tv_sec, tv_usec);

namespace qi {

  using ObjectStatistics = std::map<unsigned int, MethodStatistics>;
/** Per-instance context.
  */
  class QI_API Manageable
  {
  protected:
    Manageable();
    Manageable(const Manageable& b);
    Manageable& operator=(const Manageable& b);

  public:
    virtual ~Manageable();

    boost::mutex& initMutex();

    /// Override all ThreadingModel and force dispatch to given event loop.
    void forceExecutionContext(boost::shared_ptr<ExecutionContext> eventLoop);
    ///@return forced event loop or 0 if not set
    boost::shared_ptr<ExecutionContext> executionContext() const;

    /// @{
    /** Statistics gathering/retreiving API
     *
     */
    ///@return if statistics gatehering is enabled
    bool isStatsEnabled() const;
    /// Set statistics gathering status
    void enableStats(bool enable);
    /// Push statistics information about \p slotId.
    void pushStats(int slotId, float wallTime, float userTime, float systemTime);
    ObjectStatistics stats() const;
    /// Reset all statistical data
    void clearStats();

    /// Emitted each time a call starts and finishes, and for each signal trigger.
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
    using MethodMap = std::map<unsigned int, std::pair<AnyFunction, MetaCallType>>;
    using SignalGetter = boost::function<SignalBase* (void*)>;
    using SignalMap = std::map<unsigned int, SignalGetter>;

    /* Return the methods and signals defined at GenericObject level.
     * The 'this' argument must be the Manageable*.
    */
    static MethodMap&       manageableMmethodMap();
    static SignalMap&       manageableSignalMap();
    static MetaObject&      manageableMetaObject();
    static void             _build();
    int                     _nextTraceId();

  private:
    std::unique_ptr<ManageablePrivate> _p;
    SignalMap signalMap;
  };
}

#ifdef _MSC_VER
#  pragma warning( pop )
#endif

QI_TYPE_ENUM(qi::EventTrace::EventKind);
#endif  // _QITYPE_MANAGEABLE_HPP_
