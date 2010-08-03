/*
 * serialization.hpp
 *
 *  Created on: Oct 1, 2009 at 10:44:51 AM
 *      Author: Jean-Charles DELAY
 * 			Mail  : jdelay@aldebaran-robotics.com
 */

#ifndef LIBIPPC_SERIALIZATION_HPP_
#define LIBIPPC_SERIALIZATION_HPP_

#include <alcommon-ng/serialization/iarchive.hpp>
#include <alcommon-ng/serialization/oarchive.hpp>
#include <alcommon-ng/serialization/definition_type.hpp>
#include <alcommon-ng/serialization/call_definition.hpp>
#include <alcommon-ng/serialization/result_definition.hpp>

#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>

namespace boost {
namespace serialization {

template<class Archive>
void serialize (Archive & ar, int value, const unsigned int version);

} // namespace serialization
} // namespace boost

#include <alcommon-ng/serialization/serialization.hxx>

#endif /* !LIBIPPC_SERIALIZATION_HPP_ */
