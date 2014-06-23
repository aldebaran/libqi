#pragma once
/*
 * Copyright (c) 2012 Aldebaran Robotics. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the COPYING file.
 */

#ifndef _QI_THREADPOOL_HPP_
# define _QI_THREADPOOL_HPP_

# include <boost/function.hpp>
# include <qi/api.hpp>

namespace qi
{
  class ThreadPoolPrivate;
  /**
   * \brief Pool of workers
   * \includename{qi/threadpool.hpp}
   */
  class QI_API ThreadPool
  {
  public:
    /**
     * \brief ThreadPool constructor.
     * \param minWorkers Minimum number of workers in the pool at any time
     * \param maxWorkers Maximum number of workers allowed in the pool
     * \param minIdleWorkers Minimum number of inactive workers and ready to execute
     *                       a task immediately
     * \param maxIdleWorkers Maximum number of workers inactive
     *
     * If maxIdleWorkers is 0, then no thread reclamation is done.
     */
    ThreadPool(unsigned int minWorkers = 2, unsigned int maxWorkers = 100,
               unsigned int minIdleWorkers = 1, unsigned int maxIdleWorkers = 0);
    /**
     * \brief ThreadPool destructor.
     *
     * \verbatim
     * The destructor will quit all threads and return. All tasks left in the queue
     * are dropped. To be sure that all tasks are completed, use
     * :cpp:func:`qi::ThreadPool::waitForAll()`
     *
     * .. seealso::
     *     :cpp:func:`qi::ThreadPool::size() const`, :cpp:func:`qi::ThreadPool::waitForAll()`
     * \endverbatim
     */
    ~ThreadPool();

    /**
     * \brief Returns the number of workers in the pool.
     * \return Number of workers.
     */
    unsigned int size() const;
    /**
     * \brief Returns the number of active workers in the pool.
     * \return Number of active workers.
     */
    unsigned int active() const;

    /**
     * \brief Resizes the pool.
     * \param n New number of max workers in the pool
     *
     * \verbatim
     * This function will set the new maximum number of workers in the pool. It is
     * impossible to set the maximum workers value to a lower value than the minimum
     * one (value is discarded).
     *
     * .. seealso::
     *     :cpp:func:`qi::ThreadPool::setMinWorkers(unsigned int)`, :cpp:func:`qi::ThreadPool::getMaxWorkers() const`
     * \endverbatim
     */
    void setMaxWorkers(unsigned int n);
    /**
     * \brief Gets maximum number of workers in the pool.
     * \return Maximum number of workers.
     *
     * \verbatim
     * Return the maximum number of workers.
     *
     * .. seealso::
     *     :cpp:func:`qi::ThreadPool::setMaxWorkers(unsigned int)`
     * \endverbatim
     */
    unsigned int getMaxWorkers() const;
    /**
     * \brief Sets minimum workers.
     * \param n New number of min workers in the pool
     *
     * \verbatim
     * This function will set the new minimum number of workers in the pool. It will
     * spwan new workers if minWorkers is increased. It is impossible to set the
     * minimum workers value to a greater value than maximum workers (value is
     * discarded).
     *
     * .. seealso::
     *     :cpp:func:`qi::ThreadPool::setMaxWorkers(unsigned int)`, :cpp:func:`qi::ThreadPool::getMinWorkers() const`
     * \endverbatim
     */
    void setMinWorkers(unsigned int n);
    /**
     * \brief Gets minimum workers in the pool.
     * \return Minimum number of workers.
     *
     * \verbatim
     * .. seealso::
     *     :cpp:func:`qi::ThreadPool::setMinWorkers(unsigned int)`
     * \endverbatim
     */
    unsigned int getMinWorkers() const;
    /**
     * \brief Sets minimum inactive workers in the pool.
     * \param n New number of min idle workers in the pool.
     *
     * \verbatim
     * This function will change the minIdleWorkers number. It will spwan new
     * workers if necessary and ensure that at any time, n workers are ready to
     * handle new tasks.
     *
     * .. seealso::
     *     :cpp:func:`qi::ThreadPool::setMaxIdleWorkers(unsigned int)`, :cpp:func:`qi::ThreadPool::getMinIdleWorkers() const`
     * \endverbatim
     */
    void setMinIdleWorkers(unsigned int n);
    /**
     * \brief Returns minimum number of inactive workers in the pool.
     * \return Minimum number of inactive workers.
     *
     * \verbatim
     * Return the minimum number of inactive workers.
     *
     * .. seealso::
     *     :cpp:func:`qi::ThreadPool::setMinIdleWorkers(unsigned int)`
     * \endverbatim
     */
    unsigned int getMinIdleWorkers() const;
    /**
     * \brief Sets maximum number of inactive workers in the pool.
     * \param n New number of max idle workers in the pool.
     *
     * \verbatim
     * This function will change the maximum number of inactive workers in the pool.
     *
     * .. seealso::
     *     :cpp:func:`qi::ThreadPool::setMinIdleWorkers(unsigned int)`, :cpp:func:`qi::ThreadPool::getMaxIdleWorkers() const`
     * \endverbatim
     */
    void setMaxIdleWorkers(unsigned int n);
    /**
     * \brief Returns maximum number of inactive workers in the pool.
     * \return Maximum mumber of inactive workers.
     *
     * \verbatim
     * Return the maximum number of inactive workers.
     *
     * .. seealso::
     *     :cpp:func:`qi::ThreadPool::setMaxIdleWorkers(unsigned int)`
     * \endverbatim
     */
    unsigned int getMaxIdleWorkers() const;

    /// \brief Stop the threadpool, no more will be accepted
    void stop();

    /// \brief Put the threadpool back in a state where it accept request
    void reset();

    /**
     * \brief Sleeps until all tasks are completed.
     * If all workers are inactive and there is no task left, returns immediately.
     * Otherwise sleeps until all tasks in the pool are completed.
     */
    void waitForAll();

    /**
     * \brief Adds a task to the pool.
     * \param f Boost function with type void(void) that represents the task.
     * \return True if the task has been added to the tasks queue.
     *
     * Adds a task to the pool. The task will begin immediately if there is
     * at least one idling worker. Otherwise the tasks will be executed as soon
     * as all tasks scheduled before have begun and a worker is idling.
     */
    bool schedule(const boost::function<void(void)>& f);

  private:
    ThreadPoolPrivate* _p;
  };
}

#endif  // _QI_THREADPOOL_HPP_
