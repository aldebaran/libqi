/*
 * boost_server_shared_segment_connector.hpp
 *
 *  Created on: Oct 13, 2009 at 3:26:29 PM
 *      Author: Jean-Charles DELAY
 * 			Mail  : jdelay@aldebaran-robotics.com
 */

#ifndef LIBIPPC_BOOSTSERVERSHAREDSEGMENTCONNECTOR_HPP_
#define LIBIPPC_BOOSTSERVERSHAREDSEGMENTCONNECTOR_HPP_

//#include <alcommon-ng/serialization/definition_type.hpp>
#include <alcommon-ng/transport/shm/memory/process_shared_segment.hpp>
#include <alcommon-ng/transport/shm/server/server_shared_segment_connector.hpp>

#include <boost/interprocess/mapped_region.hpp>

namespace AL {
  namespace Transport {

/**
 * @brief A connector class that connect to the given shared zone
 * and implements some methods used to synchronize with the other process
 * when sending data to it.
 */
class BoostServerSharedSegmentConnector : public ServerSharedSegmentConnector {
public:
  /**
   * @brief The BoostServerSharedSegmentConnector constructor.
   * @param segment_name The segment to connect to.
   */
  BoostServerSharedSegmentConnector (const std::string & segment_name);

  /**
   * @brief The BoostServerSharedSegmentConnector destructor.
   */
  virtual ~BoostServerSharedSegmentConnector ();

  /**
   * @brief Connect to the shared memory. Allocate the mapped region, and set the ProcessSharedSegment *
   */
  virtual void connect ();

  /**
   * @brief Disconnect from the shared memory. Delete the mapped regiona and set this->segment to 0.
   */
  virtual void disconnect ();

  /**
   * @brief Simple getter to the mutex in the shared memory.
   * @return A reference to the segment->request_mutex.
   */
  template <typename L>
  L & getMutex ();

  /**
   * @brief Allow the server to wait for a request.
   * @param lock The lock to provide when waiting.
   * @return The string corresponding to the request (i.e. the client __invite__ string).
   */
  template <typename L>
  std::string waitForRequest (L & lock, bool & server_running);

  /**
   * @brief Notify the client when the connection is initialized.
   */
  virtual void notifyInit ();

  /**
   * @brief Broadcast and notify every one waiting on the request_mutex.
   */
  virtual void broadcast ();

  /**
   * @brief Allow the server to re-synchronize with the client when connecting to the secondary shared memory.
   * This method make the server wait until the client is writing for the first time.
   * @param lock
   */
  template <typename L>
  void resync (L & lock);

  DefinitionType getType () const;

private:
  /**
   * @brief The ProcessSharedSegment pointer.
   */
  ProcessSharedSegment * segment;

  /**
   * @brief The mapped_region used to keep the shared memory opened.
   */
  boi::mapped_region * mapped_region;
};

}
}

#include <alcommon-ng/transport/shm/server/boost_server_shared_segment_connector.hxx>

#endif /* !LIBIPPC_BOOSTSERVERSHAREDSEGMENTCONNECTOR_HPP_ */
