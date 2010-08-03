/*
 * handler_pool.cpp
 *
 *  Created on: Oct 13, 2009 at 2:41:05 PM
 *      Author: Jean-Charles DELAY
 * 			Mail  : jdelay@aldebaran-robotics.com
 */

#include <alippc/transport/common/handlers_pool.hpp>

#include <cassert>
#include <boost/bind.hpp>
#include <boost/smart_ptr.hpp>

#ifndef foreach
# include <boost/foreach.hpp>
# define foreach    BOOST_FOREACH
#endif

namespace AL {
  namespace Messaging {

HandlersPool::HandlersPool () {
  //pool.size_controller().resize(20);
  fPool = AL::ALPtr<AL::ALThreadPool>(new  AL::ALThreadPool(2,  200, 50, 100,  2 ));// Update offset
  fPool->init( 2,  200, 50, 100,  5 );
}

HandlersPool::~HandlersPool () {
  //fPool.wait();
}

void HandlersPool::pushTask (AL::ALPtr<Runnable> handler)
{
  boost::shared_ptr<Runnable> job(handler);
  fPool->enqueue(handler);
 /* schedule(pool, boost::bind(&Runnable::run, job));*/
}

}
}
