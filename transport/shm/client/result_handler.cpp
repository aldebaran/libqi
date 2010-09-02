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
  namespace Transport {

ResultHandler::ResultHandler ()
  : pool(SHARED_SEGMENT_SIZE, 500) {
  pool.init();
  m_resultMutex = AL::ALMutex::createALMutex();
}

ResultHandler::~ResultHandler () {
}

const std::string &ResultHandler::get(unsigned int id)
{
  AL::ALCriticalSection section(m_resultMutex);
  return m_results[id];
}

std::string &ResultHandler::set(unsigned int id, const std::string retval)
{
  AL::ALCriticalSection section(m_resultMutex);
  return m_results[id] = retval;
}

void ResultHandler::remove (unsigned int id)
{
  AL::ALCriticalSection section(m_resultMutex);
  std::map<unsigned int, std::string>::iterator it = m_results.find(id);
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
