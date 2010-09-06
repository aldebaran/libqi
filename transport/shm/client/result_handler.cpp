/*
 * result_handler.cpp
 *
 *  Created on: Nov 4, 2009 at 11:52:28 AM
 *      Author: Jean-Charles DELAY
 *       Mail  : delay.jc@gmail.com
 */

#include <alcommon-ng/transport/shm/client/result_handler.hpp>
#include <alcommon-ng/transport/shm/transport_config.hpp>
#include <althread/alcriticalsection.h>

namespace AL {
  namespace Transport {

ResultHandler::ResultHandler ()
  : pool(SHARED_SEGMENT_SIZE, 500) {
  pool.init();
}

ResultHandler::~ResultHandler () {
}

boost::shared_ptr<ResultInfo> ResultHandler::get(unsigned int id)
{
  boost::mutex::scoped_lock l(m_resultMutex);
  return m_results[id];
}

void ResultHandler::set(unsigned int id)
{
  boost::mutex::scoped_lock l(m_resultMutex);
  m_results[id] = boost::shared_ptr<ResultInfo>(new ResultInfo());
}

void ResultHandler::set(unsigned int id, const std::string retval)
{
  boost::shared_ptr<ResultInfo> myres = get(id);
  if (!myres) {
    std::cout << "WARNING" << std::endl;
    myres = boost::shared_ptr<ResultInfo>(new ResultInfo());
    boost::mutex::scoped_lock l(m_resultMutex);
    m_results[id] = myres;
  }
  myres->setResult(retval);
}

void ResultHandler::remove(unsigned int id)
{
  boost::mutex::scoped_lock l(m_resultMutex);
  std::map<unsigned int,  boost::shared_ptr<ResultInfo> >::iterator it = m_results.find(id);
  m_results.erase(it);
}

unsigned int ResultHandler::generateID() {
  boost::mutex::scoped_lock l(counter_access);
  return ++counter;
}

AL::shmpool::pool& ResultHandler::getShmPool () {
  return pool;
}

}
}
