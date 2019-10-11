#pragma once
/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/

#ifndef _QI_EVENTLOOP_HPP_
# define _QI_EVENTLOOP_HPP_

#include <ka/macro.hpp>

KA_WARNING_DISABLE(4503, ) // decorated name length

# include <boost/thread/synchronized_value.hpp>
# include <boost/function.hpp>

# include <qi/types.hpp>
# include <qi/api.hpp>
# include <qi/clock.hpp>
# include <qi/detail/executioncontext.hpp>

KA_WARNING_PUSH()
KA_WARNING_DISABLE(4251, )
KA_WARNING_DISABLE(4996, ) // TODO: Reactivate this warning once msvc stop triggerring a warning on overloading a deprecated function

namespace qi
{
  template<typename T> class Future;

  class EventLoopPrivate;
  /**
   * \brief Class to handle eventloop.
   * \includename{qi/eventloop.hpp}
   */
  class QI_API EventLoop : public ExecutionContext
  {
  public:
    /**
     * \brief Creates a group of threads running event loops.
     * \param name Name of the event loop to create.
     * \param nthreads Initial number of threads. If lower or equal to 0, the
     *   event loop will use in order:
     *   - the value of the environment variable QI_EVENTLOOP_THREAD_COUNT if it's set,
     *   - the value returned by std::thread::hardware_concurrency() if it's greater than 3,
     *   - the fixed value of 3.
     */
    explicit EventLoop(std::string name = "eventloop", int nthreads = 0, bool spawnOnOverload = true);

    /**
     * \see EventLoop(std::string, int, bool)
     *
     * \note When the eventloop is starting, the number of threads may be adjusted
     *    to ensure that the minimum is below or equal to the maximum, and that
     *    the number of threads to start is between the minimum and the maximum
     *    included.
     */
    EventLoop(std::string name, int nthreads, int minThreads, int maxThreads,
              bool spawnOnOverload);

    /// \brief Default destructor.
    ~EventLoop() override;

    /**
     * \brief Checks if the current thread is one of the event loop threads.
     * \note It is safe to call this method concurrently.
     * \return true if the current thread is one of the event loop threads.
     */
    bool isInThisContext() const override;
    /**
     * \brief Starts the event loop. Does nothing if already started.
     * \param threadCount Number of threads. See the constructor for more information.
     * \note It is NOT safe to call this method concurrently.
     * \deprecated EventLoop automatically starts when constructed.
     */
    QI_API_DEPRECATED_MSG(EventLoop automatically starts when constructed)
    void start(int threadCount = 0);

    /// \brief Waits for all threads of the pool to terminate.
    /// \note It is NOT safe to call this method concurrently.
    /// \deprecated EventLoop automatically joins when destroyed.
    QI_API_DEPRECATED_MSG(EventLoop automatically joins when stopped)
    void join();

    /// \brief Stops all running threads. Does nothing if already stopped.
    /// \note It is NOT safe to call this method concurrently.
    /// \deprecated EventLoop automatically stops when destroyed.
    QI_API_DEPRECATED_MSG(EventLoop automatically stops when destroyed)
    void stop();

    /**
     * \brief Sets callback to be called in case of a deadlock detection.
     * \param cb Callback to be called.
     * \note It is safe to call this method concurrently.
     */
    void setEmergencyCallback(boost::function<void()> cb);

    /**
     * \brief Sets the minimum number of threads in the pool.
     * \note It is safe to call this method concurrently.
     * \note It will be effectively taken into account the next time the
     *       "ping task" is run (see environment variable `QI_EVENTLOOP_PING_TIMEOUT`).
     */
    void setMinThreads(unsigned int min);

    /**
     * \brief Sets the maximum number of threads in the pool.
     * \param max Maximum number of threads.
     * \note It is safe to call this method concurrently.
     */
    void setMaxThreads(unsigned int max);

    /// \brief Internal function.
    void *nativeHandle();

    // DEPRECATED
    /// @{
    /**
     * \brief Calls given function once after given delay in microseconds.
     * \param callback Callback to be called.
     * \param usDelay Delay before call the callback in microsecond.
     * \return A canceleable future.
     * \deprecated use qi::async with qi::Duration
     */
    template<typename R>
    QI_API_DEPRECATED_MSG(Use 'asyncDelay' instead)
    Future<R> async(const boost::function<R()>& callback, uint64_t usDelay);
    QI_API_DEPRECATED_MSG(Use 'asyncDelay' instead)
    Future<void> async(const boost::function<void()>& callback, uint64_t usDelay)
    {
      return asyncDelayImpl(callback, qi::MicroSeconds(usDelay));
    }
    QI_API_DEPRECATED_MSG(Use 'asyncDelay' instead)
    Future<void> async(const boost::function<void()>& callback, qi::Duration delay) override
    {
      return asyncDelayImpl(callback, delay);
    }
    QI_API_DEPRECATED_MSG(Use 'asyncAt' instead)
    Future<void> async(
        const boost::function<void()>& callback, qi::SteadyClockTimePoint timepoint) override
    {
      return asyncAt(callback, timepoint);
    }

    using ExecutionContext::async;
    /// @}

    /**
     * \brief Similar to async() but without cancelation or notification.
     * \param callback Callback to be called.
     * \param usDelay Delay before call the callback in microsecond.
     */
    QI_API_DEPRECATED_MSG(Use 'asyncDelay' instead)
    void post(const boost::function<void ()>& callback, uint64_t usDelay)
    {
      postDelayImpl(callback, qi::MicroSeconds(usDelay));
    }
    QI_API_DEPRECATED_MSG(Use 'asyncDelay' instead)
    void post(const boost::function<void ()>& callback, qi::Duration delay)
    {
      postDelayImpl(callback, delay);
    }
    QI_API_DEPRECATED_MSG(Use 'asyncAt' instead)
    void post(const boost::function<void ()>& callback, qi::SteadyClockTimePoint timepoint);
    // END DEPRECATED

    using ExecutionContext::post;

    /**
     * \brief Monitors event loop to detect deadlocks.
     * \param helper an other event loop used for monitoring.
     * \param maxUsDelay maximum expected delay between an async() and its execution.
     * \return A canceleable future. Invoke cancel() to terminate monitoring.
     *         In case an async() call does not execute in time, the
     *         future's error will be set.
     */
    Future<void> monitorEventLoop(EventLoop* helper, uint64_t maxUsDelay);

  private:
    using ImplPtr = std::shared_ptr<EventLoopPrivate>;
    boost::synchronized_value<ImplPtr> _p;
    std::string       _name;

    void postImpl(boost::function<void()> callback, ExecutionOptions options) override
    {
      postDelayImpl(callback, qi::Duration(0), options);
    }

    void postDelayImpl(boost::function<void()> callback, qi::Duration delay
      , ExecutionOptions options = defaultExecutionOptions()
    );

    qi::Future<void> asyncAtImpl(boost::function<void()> cb, qi::SteadyClockTimePoint tp
      , ExecutionOptions options = defaultExecutionOptions()
    ) override;

    qi::Future<void> asyncDelayImpl(boost::function<void()> cb, qi::Duration delay
      , ExecutionOptions options = defaultExecutionOptions()
    ) override;
  };

  /// \brief Returns the global eventloop, created on demand on first call.
  QI_API EventLoop* getEventLoop();

  /// \brief Returns the global network eventloop, created on demand on first call.
  QI_API EventLoop* getNetworkEventLoop();

  /**
   * \brief Starts the eventloop with nthread threads. Does nothing if already started.
   * \param nthread Set the minimum number of worker threads in the pool.
   */
  QI_API void startEventLoop(int nthread);

  namespace detail {
    // The eventloop uses this exception to make workers terminate.
    // One reason can be the worker has been idle for too long.
    struct TerminateThread : std::exception
    {
      static const char* message()
      {
        return "Terminated";
      }
      const char* what() const KA_NOEXCEPT(true) override
      {
        return message();
      }
    };

    using IntPercent = int;

    // Percent of `n` in `[a, b]` (i.e. `a` and `b` included).
    // In other words: distance between a and n relative to the distance betwen a and b.
    // Precondition: (a <= n <= b) && (b-a <= INT_MAX/100)
    // Postcondition: 0 <= posInBetween(a, n, b) <= 100
    inline IntPercent posInBetween(int a, int n, int b)
    {
      QI_ASSERT(a <= n && n <= b);
      const int ab = b-a;
      const int an = n-a;
      QI_ASSERT(an < std::numeric_limits<int>::max()/100);
      return ab == 0 ? 100 : (100*an)/ab;
    }
  }
}

KA_WARNING_POP()

# include <qi/detail/eventloop.hxx>
# include <qi/async.hpp>

#endif  // _QI_EVENTLOOP_HPP_
