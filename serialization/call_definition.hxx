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
  ar & boost::serialization::make_nvp("request_id", request_id);
  ar & boost::serialization::make_nvp("methodName", methodName);
  ar & boost::serialization::make_nvp("moduleName", moduleName);
  ar & boost::serialization::make_nvp("list", list);
  ar & boost::serialization::make_nvp("as_res", as_res);
  ar & boost::serialization::make_nvp("is_pc", is_pc);
  ar & boost::serialization::make_nvp("sender", sender);
}

}
}
