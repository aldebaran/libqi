#pragma once
/*
**  Copyright (C) 2013 Aldebaran Robotics
**  See COPYING for the license
*/

#ifndef _QITYPE_MANAGEABLE_HPP_
#define _QITYPE_MANAGEABLE_HPP_

#include <qitype/api.hpp>
#include <qitype/anyfunction.hpp>
#include <qitype/typeobject.hpp>
#include <qitype/signal.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/function.hpp>
#include <algorithm>

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

  /// Stores min, max and sum of values fed to it
  class MinMaxSum
  {
  public:
    MinMaxSum() : _minValue(0), _maxValue(0), _cumulatedValue(0) {}
    MinMaxSum(float minValue, float maxValue, float cumulatedValue)
    : _minValue(minValue), _maxValue(maxValue), _cumulatedValue(cumulatedValue)
    {}

    const float& minValue()       const { return _minValue;}
    const float& maxValue()       const { return _maxValue;}
    const float& cumulatedValue() const { return _cumulatedValue;}
    void push(float val, bool init = false)
    {
      if (init)
        _minValue = _maxValue = _cumulatedValue = val;
      else
      {
        _cumulatedValue += val;
        _minValue = std::min(_minValue, val);
        _maxValue = std::max(_maxValue, val);
      }
    }

  private:
    float _minValue;
    float _maxValue;
    float _cumulatedValue;
  };

  /// Store statistics about method calls.
  class MethodStatistics
  {
  public:
    MethodStatistics()
    : _count(0) {}
    MethodStatistics(unsigned count, MinMaxSum wall, MinMaxSum user, MinMaxSum system)
    : _count(count), _wall(wall), _user(user), _system(system)
    {}
    void push(float wall, float user, float system)
    {
      _wall.push(wall, _count==0);
      _user.push(user, _count==0);
      _system.push(system, _count==0);
      ++_count;
    }
    const MinMaxSum& wall() const     { return _wall;}
    const MinMaxSum& user() const     { return _user;}
    const MinMaxSum& system() const   { return _system;}
    const unsigned int& count() const { return _count;}

  private:
    unsigned int _count;
    MinMaxSum _wall;
    MinMaxSum _user;
    MinMaxSum _system;
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
      const AnyValue& arguments, const qi::os::timeval timestamp, qi::int64_t userUsTime=0, qi::int64_t systemUsTime=0)
    : _id(id), _kind(kind), _slotId(slotId), _arguments(arguments),
      _timestamp(timestamp), _userUsTime(userUsTime), _systemUsTime(systemUsTime)
    {}

    // trace id, used to match call and call result
    const unsigned int&     id()              const { return _id;}
    const EventKind&        kind()            const { return _kind;}
    // method or signal id
    const unsigned int&     slotId()          const { return _slotId;}
    // call or signal arguments
    const AnyValue&         arguments()       const { return _arguments;}
    const qi::os::timeval&  timestamp()       const { return _timestamp;}
    const qi::int64_t&      userUsTime()      const { return _userUsTime;}
    const qi::int64_t&      systemUsTime()    const { return _systemUsTime;}

  private:
    unsigned int     _id; // trace id, used to match call and call result
    EventKind        _kind;
    unsigned int     _slotId; // method or signal id
    AnyValue         _arguments; // call or signal arguments
    qi::os::timeval  _timestamp;
    qi::int64_t      _userUsTime;
    qi::int64_t      _systemUsTime;
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

    typedef boost::shared_ptr<boost::recursive_timed_mutex> TimedMutexPtr;
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
    typedef std::map<unsigned int,
      std::pair<AnyFunction, MetaCallType>
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
