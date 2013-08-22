#pragma once
/*
 * Copyright (c) 2012 Aldebaran Robotics. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the COPYING file.
 */


#ifndef _SRC_THREADPOOL_P_HPP_
#define _SRC_THREADPOOL_P_HPP_

# include <queue>

# include <boost/function.hpp>
# include <boost/thread/thread.hpp>
# include <boost/thread/condition_variable.hpp>
# include <boost/thread/recursive_mutex.hpp>

# include <qi/atomic.hpp>

namespace qi
{
  class ThreadPoolPrivate
  {
    public:
      ThreadPoolPrivate(unsigned int minWorkers, unsigned int maxWorkers,
                        unsigned int minIdleWorkers, unsigned int maxIdleWorkers);
      ~ThreadPoolPrivate();

      bool schedule(const boost::function<void(void)>& f);
      void notifyManager();
      void waitForAll();
      void stop();
      void reset();

      std::map<boost::thread::id, boost::thread*> _threadsMap;
      qi::Atomic<unsigned int>                    _activeWorkers;
      qi::Atomic<unsigned int>                    _workers;
      unsigned int                                _minWorkers;
      unsigned int                                _maxWorkers;
      unsigned int                                _minIdleWorkers;
      unsigned int                                _maxIdleWorkers;

    private:
      unsigned int getNewThreadsCount();
      void workLoop();
      void manageThreads();

      boost::condition_variable                   _tasksCondition;
      boost::condition_variable                   _managerCondition;
      boost::condition_variable                   _userCondition;
      boost::mutex                                _tasksMutex;
      boost::mutex                                _managerMutex;
      boost::mutex                                _terminatedThreadsMutex;
      std::queue<boost::thread::id>               _terminatedThreads;
      std::queue<boost::function<void(void)>* >   _tasks;
      boost::thread                               _manager;
      bool                                        _closing;

  };
}

#endif  // _SRC_THREADPOOL_P_HPP_
