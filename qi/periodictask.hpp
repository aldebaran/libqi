#pragma once
/*
* Copyright (c) Aldebaran Robotics 2013 All Rights Reserved
*/
#ifndef _QI_PERIODICTASK_HPP_
# define _QI_PERIODICTASK_HPP_

# include <string>

# include <boost/function.hpp>
# include <boost/utility.hpp>

# include <qi/atomic.hpp>
# include <qi/future.hpp>
# include <qi/stats.hpp>
# include <qi/trackable.hpp>
# include <qi/actor.hpp>
# include <qi/clock.hpp>

namespace qi
{
  struct PeriodicTaskPrivate;
  /**
   * \brief Control a task executed periodically and asynchronously.
   * \includename{qi/periodictask.hpp}
   */
  class QI_API PeriodicTask: public boost::noncopyable
  {
  public:
    /// \brief Callback is a boost::function.
    using Callback = boost::function<void()>;

    /// \brief Default constructor.
    PeriodicTask();

    /// \brief Default destructor.
    ~PeriodicTask();

    /// @{
    /**
     * One of the setCallback() functions below must be called before
     * any other operation. Once set the callback cannot be changed.
     * If the callback throws, async task will be stopped
     */
    template <typename T>
    auto setCallback(T&& /*cb*/) -> typename std::enable_if<detail::IsAsyncBind<typename std::decay<T>::type>::value>::type
    {
      static_assert(sizeof(T) && false,
          "Don't use PeriodicTask::setCallback(qi::bind(...)) but setCallback(...) directly");
    }
    template <typename T>
    auto setCallback(T&& cb) -> typename std::enable_if<!detail::IsAsyncBind<typename std::decay<T>::type>::value>::type
    {
      _setCallback(std::forward<T>(cb));
    }
    template <typename AF, typename ARG0, typename... ARGS>
    inline void setCallback(AF&& callable, ARG0&& arg0, ARGS&&... args)
    {
      _connectMaybeActor(arg0);
      _setCallback(boost::bind(std::forward<AF>(callable), std::forward<ARG0>(arg0), std::forward<ARGS>(args)...));
    }

    /**
     * Set the strand on which to schedule the calls
     *
     * \warning This must be called *after* the call to setCallback or it will
     * have no effect.
     */
    void setStrand(qi::Strand* strand);

    /** \brief Set the call interval in microseconds.
     * \deprecated since 2.3. Use setPeriod.
     */
    void setUsPeriod(qi::int64_t usPeriod);

    /**
     * \brief Set the call interval.
     * \param period the PeriodicTask period
     *
     * This call will wait until next callback invocation to apply the change.
     * If you call this function from within the callback, it will be taken into
     * account immediately.
     */
    void setPeriod(qi::Duration period);

    /**
     * \brief Start the periodic task at specified period.
     *
     * No effect if already running. No effect if called from within the
     * callback.
     * \param immediate if true, first schedule of the task will happen with no delay.
     * \warning concurrent calls to start() and stop() will result in undefined behavior.
     */
    void start(bool immediate = true);

    /**
     * Trigger a started periodic task to run right now.
     * Does nothing if the periodic task just ran, is running, starting,
     * stopping or stopped.
     */
    void trigger();

    /**
     * \brief Stop the periodic task.
     *
     * When this function returns, the callback will not be called anymore.
     * Can be called from within the callback function.
     * \warning concurrent calls to start() and stop() will result in undefined behavior.
     */
    void stop();

    /**
     * \brief Request for periodic task to stop asynchronously.
     * Can be safely called from within the callback.
     */
    void asyncStop();

    /**
     * If argument is true, call interval will take into account call duration
     * to maintain the period.
     */
    void compensateCallbackTime(bool compensate);

    /// \brief Set name for debugging and tracking purpose.
    /// \param name Name of the periodic task.
    void setName(const std::string& name);

    /// \return true if task is running
    bool isRunning() const;

    /**
     * \return whether state is stopping or stopped
     *
     * Can be called from within the callback to know if stop() or asyncStop()
     * was called.
     */
    bool isStopping() const;

  private:
    boost::shared_ptr<PeriodicTaskPrivate> _p;

    template <typename ARG0>
    inline typename boost::enable_if<
        boost::is_base_of<Actor, typename detail::Unwrap<ARG0>::type>,
        void>::type
        _connectMaybeActor(const ARG0& arg0)
    {
      setStrand(detail::Unwrap<ARG0>::unwrap(arg0)->strand());
    }
    template <typename ARG0>
    inline typename boost::disable_if<
        boost::is_base_of<Actor, typename detail::Unwrap<ARG0>::type>,
        void>::type
        _connectMaybeActor(const ARG0&)
    {
      setStrand(0);
    }

    void _setCallback(const Callback& cb);
  };

}
#endif
