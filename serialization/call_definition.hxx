/*
 * call_defintion.hxx
 *
 *  Created on: Oct 5, 2009 at 5:14:44 PM
 *      Author: Jean-Charles DELAY
 * 			Mail  : jdelay@aldebaran-robotics.com
 */

#include <alcommon-ng/serialization/call_definition.hpp>

namespace AL {
  namespace Messaging {

template<typename T>
void CallDefinition::push (const T & value) {
  list.push_back(value);
}

template<class Archive>
void CallDefinition::serialize(Archive & ar, const unsigned int version) {
  (void) version;
  ar & request_id;
  ar & methodName;
  ar & moduleName;
  ar & list;
  ar & as_res;
  ar & is_pc;
  ar & sender;
}

}
}
