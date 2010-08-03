/*
 * result_handler.hpp
 *
 *  Created on: Nov 4, 2009 at 11:52:28 AM
 *      Author: Jean-Charles DELAY
 * 			Mail  : delay.jc@gmail.com
 */

#ifndef LIBIPPC_RESULTHANDLER_HPP_
#define LIBIPPC_RESULTHANDLER_HPP_

#include <alippc/transport/shm/client/result_info.hpp>

#include <shmpool/pool.hpp>

#include <map>
#include <boost/shared_ptr.hpp>
#include <althread/almutex.h>

namespace AL {
  namespace Messaging {


class ResultHandler {
public:
  ResultHandler ();
  virtual ~ResultHandler ();

public:
  void push (unsigned int id);
  boost::shared_ptr<ResultInfo> get (unsigned int id);
  void remove (unsigned int id);
  unsigned int generateID ();

  AL::shmpool::pool& getShmPool ();

private:
  AL::ALPtr<AL::ALMutex> m_resultMutex;
  std::map<unsigned int, boost::shared_ptr<ResultInfo> > m_results;

  unsigned int counter;
  boost::mutex counter_access;

  AL::shmpool::pool pool;
};



}
}
#endif /* !LIBIPPC_RESULTHANDLER_HPP_ */
