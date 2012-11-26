/*
 * Copyright (c) 2012 Aldebaran Robotics. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the COPYING file.
 */

#include <qi/threadpool.hpp>
#include "threadpool_p.hpp"

#ifdef _MSC_VER
#  pragma warning( push )
#  pragma warning( disable: 4355 )
#endif

namespace qi
{
  ThreadPoolPrivate::ThreadPoolPrivate(unsigned int minWorkers, unsigned int maxWorkers,
                                       unsigned int minIdleWorkers, unsigned int maxIdleWorkers)

    :
      _threadsMap(),
      _activeWorkers(0),
      _workers(0),
      _minWorkers(minWorkers),
      _maxWorkers(maxWorkers),
      _minIdleWorkers(minIdleWorkers),
      _maxIdleWorkers(maxIdleWorkers),
      _tasksCondition(),
      _managerCondition(),
      _userCondition(),
      _tasksMutex(),
      _managerMutex(),
      _terminatedThreadsMutex(),
      _terminatedThreads(),
      _tasks(),
      /* _manager must be the last initialized */
      // warning C4355: 'this' : used in base member initializer list
      _manager (boost::thread(boost::bind(&ThreadPoolPrivate::manageThreads, this)))
  {
  }

  ThreadPoolPrivate::~ThreadPoolPrivate()
  {
    _manager.interrupt();
    _manager.join();

    for (std::map<boost::thread::id, boost::thread*>::iterator it = _threadsMap.begin();
          it != _threadsMap.end(); ++it)
    {
      (*it).second->interrupt();
      (*it).second->join();
      delete (*it).second;
    }
  }

  void ThreadPoolPrivate::notifyManager()
  {
    _managerCondition.notify_all();
  }

  bool ThreadPoolPrivate::schedule(const boost::function<void(void)>& f)
  {
    {
      boost::mutex::scoped_lock lock(_tasksMutex);

      _tasks.push(f);
    }
    _tasksCondition.notify_one();

    /* Do we need more threads ? */
    notifyManager();

    return true;
  }

  void ThreadPoolPrivate::waitForAll()
  {
    { /* Lock here otherwise we could wait just after all threads go to sleep */
      boost::mutex::scoped_lock lock(_tasksMutex);

      if ((*_activeWorkers != 0) && (_tasks.size() != 0))
        _userCondition.wait(lock);
    }
  }

  void ThreadPoolPrivate::workLoop()
  {
    qi::os::setCurrentThreadName("tp-idle");
    while (true)
    {
      boost::function<void(void)> task;

      { /* -- Locked Section -- */
        boost::mutex::scoped_lock lock(_tasksMutex);

        if (_tasks.empty())
        {
          /* Should the thread be destroyed ? */
          if ((((*_workers - *_activeWorkers) > _maxIdleWorkers) && *_workers > _minWorkers)
               || (*_workers > _maxWorkers))
          {
            break;
          }

          /* Notify user that all tasks are done */
          if (*_activeWorkers == 0)
            _userCondition.notify_all();

          /* Wait for a task to be scheduled */
          _tasksCondition.wait(lock);
        }
        else /* Retrieve the task */
        {
          task = _tasks.front();
          _tasks.pop();
        }
      } /* -- End Locked Section -- */

      if (task)
      {
        qi::os::setCurrentThreadName("tp-worker");
        ++_activeWorkers;
        task();
        --_activeWorkers;
        qi::os::setCurrentThreadName("tp-idle");
      }
    }

    /* Thread destruction */
    --_workers;
    {
      boost::mutex::scoped_lock lock(_terminatedThreadsMutex);

      _terminatedThreads.push(boost::this_thread::get_id());
    }
    notifyManager();
  }

  /*
   * Return the number of threads needed to be created
   */
  unsigned int ThreadPoolPrivate::getNewThreadsCount()
  {
    /* Not enough workers ? */
    if (*_workers < _minWorkers)
    {
      return _minWorkers - *_workers;
    }
    /* Not enough idle workers ? */
    if ((*_workers - *_activeWorkers) < _minIdleWorkers)
    {
      unsigned int newThreads = _minIdleWorkers - ((*_workers - *_activeWorkers));
      if ((*_workers + newThreads) > _maxWorkers)
        newThreads = _maxWorkers - *_workers;

      return newThreads;
    }
    /* Not enough workers for all the tasks ? */
    if (*_workers == *_activeWorkers)
    {
      unsigned int newThreads = _tasks.size();

      if ((*_workers + newThreads) > _maxWorkers)
        newThreads = _maxWorkers - *_workers;

      return newThreads;
    }

    return 0;
  }

  void ThreadPoolPrivate::manageThreads()
  {
    qi::os::setCurrentThreadName("tp-manager");
    while (true)
    {
      unsigned int threadstoCreate = getNewThreadsCount();

      /* Create Threads */
      for (unsigned int i = 0; i < threadstoCreate; i++)
      {
        boost::thread* newThread = new boost::thread(boost::bind(&ThreadPoolPrivate::workLoop, this));

        _threadsMap[newThread->get_id()] = newThread;
        ++_workers;
      }

      /* Check if we have too much threads */
      if (((*_workers - *_activeWorkers) > _maxIdleWorkers) || (*_workers > _maxWorkers))
        _tasksCondition.notify_all();

      /* Join and free terminated threads */
      while (!_terminatedThreads.empty())
      {
        boost::thread::id threadToFreeId;
        std::map<boost::thread::id, boost::thread*>::iterator  threadToFreeIterator;

        { /* -- Locked Section -- */
          boost::mutex::scoped_lock lock(_terminatedThreadsMutex);

          threadToFreeId = _terminatedThreads.front();
          _terminatedThreads.pop();
        } /* -- End Locked Section -- */

        threadToFreeIterator = _threadsMap.find(threadToFreeId);

        /* Should be immediate since the thread has already quit */
        (*threadToFreeIterator).second->interrupt();
        (*threadToFreeIterator).second->join();
        delete (*threadToFreeIterator).second;

        _threadsMap.erase(threadToFreeIterator);
      }

      { /* -- Locked Section -- */
        boost::mutex::scoped_lock lock(_managerMutex);

        _managerCondition.wait(lock);
      } /* -- End Locked Section -- */
    }
  }

  /*
   * ThreadPool
   */

  ThreadPool::ThreadPool(unsigned int minWorkers, unsigned int maxWorkers,
                         unsigned int minIdleWorkers, unsigned int maxIdleWorkers)
    : _p (new ThreadPoolPrivate(minWorkers, maxWorkers, minIdleWorkers, maxIdleWorkers))
  {
  }

  ThreadPool::~ThreadPool()
  {
    delete _p;
  }

  unsigned int ThreadPool::size() const
  {
    return *_p->_workers;
  }

  unsigned int ThreadPool::active() const
  {
    return *_p->_activeWorkers;
  }

  void ThreadPool::setMaxWorkers(unsigned int n)
  {
    if (n < _p->_minWorkers)
      return;
    _p->_maxWorkers = n;
    _p->notifyManager();
  }

  void ThreadPool::setMinIdleWorkers(unsigned int n)
  {
    _p->_minIdleWorkers = n;
    _p->notifyManager();
  }

  void ThreadPool::setMaxIdleWorkers(unsigned int n)
  {
    _p->_maxIdleWorkers = n;
    _p->notifyManager();
  }

  void ThreadPool::setMinWorkers(unsigned int n)
  {
    if (n > _p->_maxWorkers)
      return;
    _p->_minWorkers = n;
    _p->notifyManager();
  }

  unsigned int ThreadPool::getMaxWorkers() const
  {
    return _p->_maxWorkers;
  }

  unsigned int ThreadPool::getMinIdleWorkers() const
  {
    return _p->_minIdleWorkers;
  }

  unsigned int ThreadPool::getMaxIdleWorkers() const
  {
    return _p->_maxIdleWorkers;
  }

  unsigned int ThreadPool::getMinWorkers() const
  {
    return _p->_minWorkers;
  }

  bool ThreadPool::schedule(const boost::function<void(void)>& f)
  {
    return _p->schedule(f);
  }

  void ThreadPool::waitForAll()
  {
    _p->waitForAll();
  }
}

#ifdef _MSC_VER
#  pragma warning( pop )
#endif