/*
 * result_handler.cpp
 *
 *  Created on: Nov 4, 2009 at 11:52:28 AM
 *      Author: Jean-Charles DELAY
 * 			Mail  : delay.jc@gmail.com
 */

#include <alcommon-ng/transport/shm/client/result_handler.hpp>
#include <alcommon-ng/transport/shm/transport_config.hpp>
#include <althread/alcriticalsection.h>

namespace AL {
  namespace Messaging {

ResultHandler::ResultHandler () : pool(SHARED_SEGMENT_SIZE, 500) {
  pool.init();
  m_resultMutex = AL::ALMutex::createALMutex();
}

ResultHandler::~ResultHandler () {
}

void ResultHandler::push (unsigned int id) 
{
  AL::ALCriticalSection section(m_resultMutex);
  if (id == 0)
  {
    printf("id 0");
  }
  m_results[id] = boost::shared_ptr<ResultInfo>(new ResultInfo());
}

boost::shared_ptr<ResultInfo> ResultHandler::get (unsigned int id) 
{
  AL::ALCriticalSection section(m_resultMutex);
  return m_results[id];
}

void ResultHandler::remove (unsigned int id) 
{
  AL::ALCriticalSection section(m_resultMutex);
  std::map<unsigned int, boost::shared_ptr<ResultInfo> >::iterator it = m_results.find(id);
  //std::cout << "remove : " << id << std::endl;
  m_results.erase(it);
}

unsigned int ResultHandler::generateID () {
  boost::mutex::scoped_lock l(counter_access);
  return ++counter;
}

AL::shmpool::pool& ResultHandler::getShmPool () {
  return pool;
}

}
}
