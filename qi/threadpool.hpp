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

  class QI_API ThreadPool
  {
    public:
      ThreadPool(unsigned int minWorkers = 2, unsigned int maxWorkers = 8,
                 unsigned int minIdleWorkers = 1, unsigned int maxIdleWorkers = 4);
      ~ThreadPool();

      unsigned int size() const;
      unsigned int active() const;

      void setMaxWorkers(unsigned int n);
      unsigned int getMaxWorkers() const;
      void setMinWorkers(unsigned int n);
      unsigned int getMinWorkers() const;
      void setMinIdleWorkers(unsigned int n);
      unsigned int getMinIdleWorkers() const;
      void setMaxIdleWorkers(unsigned int n);
      unsigned int getMaxIdleWorkers() const;

      void waitForAll();

      bool schedule(const boost::function<void(void)>& f);

    private:
      ThreadPoolPrivate* _p;
  };
}

#endif // _LIBQI_QI_THREADPOOL_HPP_
