/*
* handler_pool.cpp
*
*  Created on: Oct 13, 2009 at 2:41:05 PM
*      Author: Jean-Charles DELAY
*      Mail  : jdelay@aldebaran-robotics.com
*/

#include <qi/transport/common/handlers_pool.hpp>
#include <qi/transport/common/i_runnable.hpp>
#include <boost/threadpool.hpp>

namespace qi {
  namespace transport {

    HandlersPool::HandlersPool()
      : _pool(20)
    {
    }

    HandlersPool::~HandlersPool() {
      //vfPool.wait();
    }

    void HandlersPool::pushTask(boost::shared_ptr<IRunnable> handler)
    {
      _pool.schedule(boost::bind(&IRunnable::run, handler));
    }
  }
}
