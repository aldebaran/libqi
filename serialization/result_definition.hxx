/*
 * result_definition.hxx
 *
 *  Created on: Oct 5, 2009 at 5:34:28 PM
 *      Author: Jean-Charles DELAY
 * 			Mail  : jdelay@aldebaran-robotics.com
 */

namespace AL {
  namespace Messaging {

template <typename T>
void ResultDefinition::value (const T & val) {
  is_exception = false;
  v = VariableValue(val);
}

template<class Archive>
void ResultDefinition::serialize(Archive & ar, const unsigned int version) {
  (void) version;
  ar & request_id;
  ar & is_exception;
  ar & v;
}

}
}

