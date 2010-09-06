/*
* handler_pool.cpp
*
*  Created on: Oct 13, 2009 at 2:41:05 PM
*      Author: Jean-Charles DELAY
*      Mail  : jdelay@aldebaran-robotics.com
*/

#include <alcommon-ng/transport/common/handlers_pool.hpp>
#include <alcommon-ng/transport/common/i_runnable.hpp>
#include <althread/althreadpool.h>
#include <boost/shared_ptr.hpp>

namespace AL {
  namespace Transport {

    HandlersPool::HandlersPool() :
    fPool(new  AL::ALThreadPool(2,  200, 50, 100,  2 ))
    {
      fPool->init( 2,  200, 50, 100,  5 );
    }

    HandlersPool::~HandlersPool() {
      //fPool.wait();
    }

    void HandlersPool::pushTask(boost::shared_ptr<IRunnable> handler)
    {
      fPool->enqueue(handler);
      /* schedule(pool, boost::bind(&Runnable::run, job));*/
    }

  }
}
