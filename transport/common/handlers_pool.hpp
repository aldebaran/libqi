/*
* handler_pool.hpp
*
*  Created on: Oct 13, 2009 at 2:41:05 PM
*      Author: Jean-Charles DELAY
*      Mail  : jdelay@aldebaran-robotics.com
*/

#ifndef AL_TRANSPORT_HANDLERS_POOL_HPP_
#define AL_TRANSPORT_HANDLERS_POOL_HPP_

#include <list>
#include <boost/thread/mutex.hpp>
#include <boost/thread/condition.hpp>
#include <alcommon-ng/transport/common/runnable.hpp>
#include <althread/althreadpool.h>

namespace AL {
  namespace Transport {

    class ConnectionHandler;

    /// <summary>
    /// The thread pool used to handler ConnectionHandler(s). It creates
    /// five threads, and schedule pushed handlers to those thread.
    /// </summary>
    class HandlersPool {
    public:
      /// <summary>The HandlersPool constructor.</summary>
      HandlersPool ();

      /// <summary>The HandlersPool destructor</summary>
      virtual ~HandlersPool ();

    public:
      /// <summary>
      /// Push a handler into the pool and schedule it.
      /// </summary>
      void pushTask (boost::shared_ptr<Runnable> handler);

    private:
      /// <summary>
      /// The Boost implementation of the thread pool.
      /// </summary>
      boost::shared_ptr<AL::ALThreadPool> fPool;
    };

  }
}
#endif  // AL_TRANSPORT_HANDLERS_POOL_HPP_
