/*
 * result_definition.cpp
 *
 *  Created on: Oct 5, 2009 at 5:34:02 PM
 *      Author: Jean-Charles DELAY
 * 			Mail  : jdelay@aldebaran-robotics.com
 */

#include <alcommon-ng/serialization/result_definition.hpp>

namespace AL {
  namespace Messaging {

ResultDefinition::ResultDefinition () :
  request_id(0), is_exception(false), v(0) {
}

ResultDefinition::ResultDefinition (const CallDefinition &def) :
  request_id(def.getRequestId()), is_exception(false), v(0) {
}

ResultDefinition::ResultDefinition (uint32_t request_id, const VariableValue & val) :
  request_id(request_id), is_exception(false), v(val) {
}

ResultDefinition::~ResultDefinition () {
}

bool ResultDefinition::operator==(const ResultDefinition& rhs) const {
  return (
    /*(this->v.as<std::string>() == rhs.exceptionMessage()) && */
    (this->request_id == rhs.getRequestId()) 
    /* FIXME(ckilner) ambiguous && (this->v == rhs.value()) */
    );
}

bool ResultDefinition::exceptionCaught () const {
  return is_exception;
}

uint32_t ResultDefinition::getRequestId () const {
  return request_id;
}

void ResultDefinition::setRequestId (uint32_t id) {
  this->request_id = id;
}

void ResultDefinition::exception (const std::string & message) {
  is_exception = true;
  v = VariableValue(message);
}

std::string ResultDefinition::exceptionMessage () const {
  return v.as<std::string>();
}

const VariableValue & ResultDefinition::value () const {
  return v;
}

}
}
