/*
 * Copyright (c) 2012 Aldebaran Robotics. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the COPYING file.
 */

#pragma once

#ifndef _LIBQI_QI_THREADPOOL_HPP_
# define _LIBQI_QI_THREADPOOL_HPP_

# include <boost/function.hpp>

# include <qi/qi.hpp>

namespace qi
{
  class ThreadPoolPrivate;

  /// Pool of workers
  class QI_API ThreadPool
  {
    public:
      /// ThreadPool constructor.
      ThreadPool(unsigned int minWorkers = 2, unsigned int maxWorkers = 8,
                 unsigned int minIdleWorkers = 1, unsigned int maxIdleWorkers = 4);
      /// ThreadPool destructor.
      ~ThreadPool();

      /// Returns the number of workers in the pool.
      unsigned int size() const;
      /// Returns the number of active workers in the pool.
      unsigned int active() const;

      /// Resizes the pool.
      void setMaxWorkers(unsigned int n);
      /// Sets minimum number of workers in the pool.
      unsigned int getMaxWorkers() const;
      /// Sets max idle workers.
      void setMinWorkers(unsigned int n);
      /// Sets min idle workers.
      unsigned int getMinWorkers() const;
      /// Returns maximum number of workers in the pool.
      void setMinIdleWorkers(unsigned int n);
      /// Returns minimum number of workers in the pool.
      unsigned int getMinIdleWorkers() const;
      /// Returns maximum number of inactive workers in the pool.
      void setMaxIdleWorkers(unsigned int n);
      /// Returns minimum number of inactive workers in the pool.
      unsigned int getMaxIdleWorkers() const;

      /// Sleeps until all tasks are completed
      void waitForAll();

      /// Adds a task to the pool.
      bool schedule(const boost::function<void(void)>& f);

    private:
      ThreadPoolPrivate* _p;
  };
}

#endif // _LIBQI_QI_THREADPOOL_HPP_
