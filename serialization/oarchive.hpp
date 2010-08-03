/*
 * oarchive.hpp
 *
 *  Created on: Oct 2, 2009 at 12:28:51 PM
 *      Author: Jean-Charles DELAY
 * 			Mail  : jdelay@aldebaran-robotics.com
 */

#ifndef LIBIPPC_OARCHIVE_HPP_
#define LIBIPPC_OARCHIVE_HPP_

#include <iostream>

#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_oarchive_impl.hpp>

using namespace boost::archive;

namespace AL {
  namespace Messaging {

/**
 * @brief An input archive class used to serialize any data.
 */
class OArchive : public binary_oarchive {
public:
  /**
   * @brief The OArchive class constructor.
   * @param is The output stream to write to.
   * @param flags Flags to pass to the binary_archive.
   */
  OArchive (std::ostream & os, unsigned int flags = 0) :
    binary_oarchive (os, flags) {
  }

  /**
   * @brief The OArchive class constructor.
   * @param is The streambuf to write to.
   * @param flags Flags to pass to the binary_archive.
   */
  OArchive (std::streambuf & bsb, unsigned int flags = 0) :
    binary_oarchive (bsb, flags) {
  }

  /**
   * @brief The OArchive class destructor.
   */
  virtual ~OArchive () {}

};

}
}

#endif /* !LIBIPPC_OARCHIVE_HPP_ */
