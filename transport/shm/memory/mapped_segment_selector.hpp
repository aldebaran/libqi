/*
 * mapped_segment_selector.hpp
 *
 *  Created on: Oct 12, 2009 at 10:27:06 AM
 *      Author: Jean-Charles DELAY
 * 			Mail  : jdelay@aldebaran-robotics.com
 */

#ifndef LIBIPPC_MAPPEDSEGMENTSELECTOR_HPP_
#define LIBIPPC_MAPPEDSEGMENTSELECTOR_HPP_

#include <alcommon-ng/transport/shm/memory/mapped_shared_segment.hpp>

#include <map>
#include <string>
#include <cstring>

#include <boost/interprocess/mapped_region.hpp>
#include <boost/interprocess/sync/interprocess_mutex.hpp>

namespace boi = boost::interprocess;

namespace AL {
  namespace Messaging {

/**
 * @brief A segment definition as save in the MappedSegmentSelector.segments map
 */
struct SegmentDefinition {
  MappedSharedSegment * segment;
  boi::mapped_region * region;
  unsigned int flags;
};

/**
 * @brief A segment name. This structure is only useful because maps cannot allow
 * char buffers as key object.
 */
struct SegmentName {
  SegmentName (const char name [SEGMENT_NAME_MAX]) { std::memcpy(value, name, SEGMENT_NAME_MAX); }
  char value [SEGMENT_NAME_MAX];
};

/**
 * @brief The segment name comparator use by the MappedSegmentSelector.segments map
 */
struct SegmentNameComparator {
  /**
   * @brief Compare the two strings s1 & s2 and return the result of (s1 < s2)
   * @param s1 The first string to compare.
   * @param s2 The second string to compare.
   * @return (s1 < s2)
   */
  bool operator () (const SegmentName s1, const SegmentName s2) const {
    return std::strncmp(s1.value, s2.value, SEGMENT_NAME_MAX) < 0 ? true : false;
  }
};

/**
 * @brief Singleton that open or create a shared memory segment and produce a (mapped_region *).
 */
class MappedSegmentSelector {
private:
  /**
   * @brief The MappedSegmentSelector class constructor.
   */
  MappedSegmentSelector ();

  /**
   * @brief The MappedSegmentSelector destructor.
   */
  virtual ~MappedSegmentSelector ();

public:
  /**
   * @brief Static method that return the singleton instance.
   * @return The MappedSegmentSelector singleton instance.
   */
  static MappedSegmentSelector & instance ();

  /**
   * @brief Generate a shared segment identified by its name __segment_name__ and its flags
   * (MS_OPEN/MS_CREATE/MS_REMOVE).
   * @param segment_name The segment name to create/open/remove
   * @param flags The flags to handle the shared segment (open/create/remove).
   * @return A pointer to the shared segment identified by __segment_name__.
   */
  MappedSharedSegment * get (const char segment_name [SEGMENT_NAME_MAX], unsigned int flags);

  /**
   * @brief Ask for a segment removal.
   * @param segment_name The segment to free.
   */
  void free (const char segment_name [SEGMENT_NAME_MAX]);

public:
  /**
   * @brief Flag to create the shared segment.
   */
  static const unsigned int MS_CREATE = (1 << 0);

  /**
   * @brief Flag to open the shared segment.
   */
  static const unsigned int MS_OPEN = (1 << 1);

  /**
   * @brief Flag to remove the shared segment.
   */
  static const unsigned int MS_REMOVE = (1 << 2);

private:
  /**
   * @brief Create a shared segment.
   * @param segment_name The name of the segment to create.
   * @param sd The structure to fill to identify the segment.
   */
  void createSharedSegment (const char segment_name [SEGMENT_NAME_MAX], SegmentDefinition & sd);

  /**
   * @brief Open a shared segment.
   * @param segment_name The name of the segment to open.
   * @param sd The structure to fill to identify the segment.
   */
  void openSharedSegment (const char segment_name [SEGMENT_NAME_MAX], SegmentDefinition & sd);

  /**
   * @brief Create or open a shared segment.
   * @param segment_name The name of the segment to create/open.
   * @param sd The structure to fill to identify the segment.
   */
  void createOrOpenSharedSegment (const char segment_name [SEGMENT_NAME_MAX], SegmentDefinition & sd);

  /**
   * @brief Close a shared segment. and remove it if flag MS_REMOVE has been passed.
   * @param segment_name The name of the segment to close.
   * @param sd The structure that identify the segment.
   */
  void closeSharedSegment (const SegmentName segment_name, SegmentDefinition & sd);

  /**
   * @brief Mutex to protect the map access.
   */
  boi::interprocess_mutex map_use;

  /**
   * typedefs for the segment definition map.
   */
  typedef std::map<const SegmentName, SegmentDefinition, SegmentNameComparator> segments_map;
	typedef std::pair<const SegmentName, SegmentDefinition> segments_map_elt;

	/**
	 * @brief The segments map.
	 */
  segments_map segments;
};

}
}
#endif /* !LIBIPPC_MAPPEDSEGMENTSELECTOR_HPP_ */
