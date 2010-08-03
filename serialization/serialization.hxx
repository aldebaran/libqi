/*
 * serialization.hxx
 *
 *  Created on: Oct 2, 2009 at 12:02:46 PM
 *      Author: Jean-Charles DELAY
 * 			Mail  : jdelay@aldebaran-robotics.com
 */

namespace boost {
namespace serialization {

template<class Archive>
void serialize(Archive & ar, int value, const unsigned int version) {
	ar & value;
}

} /* namespace serialization */
} /* namespace boost */
