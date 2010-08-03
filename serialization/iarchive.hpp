/*
 * oarchive.hpp
 *
 *  Created on: Oct 2, 2009 at 12:28:51 PM
 *      Author: Jean-Charles DELAY
 * 			Mail  : jdelay@aldebaran-robotics.com
 */

#ifndef LIBIPPC_IARCHIVE_HPP_
#define LIBIPPC_IARCHIVE_HPP_

#include <iostream>

#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_iarchive_impl.hpp>

using namespace boost::archive;

namespace AL {
  namespace Messaging {

/**
 * @brief An input archive class used to de-serialize any data.
 */
class IArchive : public binary_iarchive {
public:
  /**
   * @brief The IArchive class constructor.
   * @param is The input stream to read from.
   * @param flags Flags to pass to the binary_archive.
   */
  IArchive (std::istream & is, unsigned int flags = 0) :
    binary_iarchive (is, flags) {
  }

  /**
   * @brief The IArchive class constructor.
   * @param bsb The streambuf to read from.
   * @param flags Flags to pass to the binary_archive.
   */
  IArchive (std::streambuf & bsb, unsigned int flags = 0) :
    binary_iarchive (bsb, flags) {
  }

  /**
   * @brief The IArchive class destructor.
   */
  virtual ~IArchive () {}

};

}
}
#endif /* !LIBIPPC_IARCHIVE_HPP_ */
