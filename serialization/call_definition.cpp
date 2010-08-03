/*
 * call_defintion.cpp
 *
 *  Created on: Oct 5, 2009 at 4:35:09 PM
 *      Author: Jean-Charles DELAY
 * 			Mail  : jdelay@aldebaran-robotics.com
 */

#include <alcommon-ng/serialization/call_definition.hpp>

#include <cstring>

namespace AL {
  namespace Messaging {

CallDefinition::CallDefinition () :
  request_id(-1), as_res(false), is_pc(false) {
  methodName = "myMethod";
}

CallDefinition::CallDefinition (uint32_t request_id) :
  request_id(request_id), as_res(false), is_pc(false) {
  methodName = "myMethod";
}

CallDefinition::~CallDefinition () {
}

int32_t CallDefinition::getRequestId () const {
  return request_id;
}

void CallDefinition::setRequestId (int32_t id) {
  request_id = id;
}

void CallDefinition::setMethodName (const std::string & methodName) {
  this->methodName = methodName;
}

void CallDefinition::setModuleName (const std::string & moduleName) {
  this->moduleName = moduleName;
}

void CallDefinition::isPCall (bool value) {
  this->is_pc = value;
}

bool CallDefinition::isPCall () const {
  return is_pc;
}

void CallDefinition::asResult (bool value) {
  as_res = value;
}

void CallDefinition::setSender (const std::string & sender) {
  this->sender = sender;
}

const std::string & CallDefinition::getSender () const {
  return sender;
}

bool CallDefinition::asResult () const {
  return as_res;
}

const std::string & CallDefinition::getMethodName () const {
  return methodName;
}

const std::string & CallDefinition::getModuleName () const {
  return moduleName;
}

const VariablesList & CallDefinition::getParameters () const {
  return list;
}

}
}
