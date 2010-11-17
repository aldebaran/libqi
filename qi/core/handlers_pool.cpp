/*
* handler_pool.cpp
*
*  Created on: Oct 13, 2009 at 2:41:05 PM
*      Author: Jean-Charles DELAY
*      Mail  : jdelay@aldebaran-robotics.com
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
