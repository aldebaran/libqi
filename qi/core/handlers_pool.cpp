/*
*  Author(s):
*  - Chris  Kilner <ckilner@aldebaran-robotics.com>
*  - Cedric Gestes <gestes@aldebaran-robotics.com>
*
*  Copyright (C) 2010 Aldebaran Robotics
*/

#include <qi/core/runnable.hpp>
#include <qi/core/handlers_pool.hpp>
#include <boost/threadpool.hpp>

namespace qi {

  HandlersPool::HandlersPool()
    : _pool(20)
  {
  }

  HandlersPool::~HandlersPool() {
    //vfPool.wait();
  }

  void HandlersPool::pushTask(boost::shared_ptr<qi::Runnable> handler)
  {
    _pool.schedule(boost::bind(&qi::Runnable::run, handler));
  }

}
