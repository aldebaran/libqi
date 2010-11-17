/*
* handler_pool.hpp
*
*  Created on: Oct 13, 2009 at 2:41:05 PM
*      Author: Jean-Charles DELAY
*      Mail  : jdelay@aldebaran-robotics.com
*/

#ifndef QI_CORE_HANDLERS_POOL_HPP_
#define QI_CORE_HANDLERS_POOL_HPP_

#include <boost/threadpool.hpp>

namespace qi {
  class Runnable;


  /// <summary>
  /// The thread pool used to handler ConnectionHandler(s). It creates
  /// five threads, and schedule pushed handlers to those thread.
  /// </summary>
  class HandlersPool {
  public:
    /// <summary>The HandlersPool constructor.</summary>
    HandlersPool();

    /// <summary>The HandlersPool destructor</summary>
    virtual ~HandlersPool();

  public:
    /// <summary>
    /// Push a handler into the pool and schedule it.
    /// </summary>
    void pushTask(boost::shared_ptr<qi::Runnable> handler);

  private:
    /// <summary>
    /// The ??Boost?? implementation of the thread pool.
    /// </summary>
      //boost::shared_ptr<AL::ALThreadPool> fPool;
    boost::threadpool::pool _pool;
  };

}
#endif  // QI_CORE_HANDLERS_POOL_HPP_
