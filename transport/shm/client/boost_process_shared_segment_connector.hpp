/*
 * boost_process_shared_segment_connector.hpp
 *
 *  Created on: Oct 8, 2009 at 12:42:23 PM
 *      Author: Jean-Charles DELAY
 * 			Mail  : jdelay@aldebaran-robotics.com
 */

#ifndef LIBIPPC_BOOSTPROCESSSHAREDSEGMENTCONNECTOR_HPP_
#define LIBIPPC_BOOSTPROCESSSHAREDSEGMENTCONNECTOR_HPP_

//#include <alcommon-ng/serialization/definition_type.hpp>

#include <alcommon-ng/transport/shm/memory/process_shared_segment.hpp>
#include <alcommon-ng/transport/shm/client/process_shared_segment_connector.hpp>

#include <string>
#include <boost/interprocess/mapped_region.hpp>

namespace boi = boost::interprocess;

namespace AL {
  namespace Transport {

/**
 * @brief A connector class that connect to the given process personal
 * shared memory and implements some methods used to synchronize with that
 * process before sending data.
 */
class BoostProcessSharedSegmentConnector : public ProcessSharedSegmentConnector {
public:
  /**
   * @brief The BosstProcessSharedSegmentConnector class constructor.
   * @param process_name the process name to connect to.
   */
  BoostProcessSharedSegmentConnector (const std::string & process_name);

  /**
   * @brief The BoostProcessSharedSegmentConnector class desctructor.
   */
  virtual ~BoostProcessSharedSegmentConnector ();

  /**
   * @brief Try to connect to the process, and sync with it. Write the invite string
   * into the process buffer and notify it. Then wait for the process acknowledge before
   * leaving.
   * @param invite The name of the shared memory used to transmit data between the two processes.
   */
  virtual void handShake (const char invite [SEGMENT_NAME_MAX], DefinitionType type);

  /**
   * @brief Notify the target process that the first write is done and that it can connect
   * to the shared memory defined before by the *invite* param of the
   * handShake (const char [SEGMENT_NAME_MAX]) method.
   */
  virtual void notifyWrite ();

  /**
   * @brief Lock the target process operation mutex in order to prevent other processes to send a
   * request.
   */
  virtual void lock ();

  /**
   * @brief Unlock the target process operation mutex to allow other processes to connect.
   */
  virtual void unlock ();

private:
  /**
   * @brief Connect to the process shared memory and bind its segment to the ProcessSharedSegment *.
   */
  void connect ();

  /**
   * @brief Disconnect from the process shared memory, free the mapped_region and set this->segment to 0.
   */
  void disconnect ();

  /**
   * @brief The target process name.
   */
  std::string process_segment_name;

  /**
   * @brief Bind to the process public shared segment used to send a request.
   */
  ProcessSharedSegment * segment;

  /**
   * @brief The mapped_region used to keep the shared memory segment opened.
   */
  boi::mapped_region * mapped_region;
};

}
}
#endif /* !LIBIPPC_BOOSTPROCESSSHAREDSEGMENTCONNECTOR_HPP_ */
