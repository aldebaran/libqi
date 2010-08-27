/*
* result_definition.cpp
*
*  Created on: Oct 5, 2009 at 5:34:02 PM
*      Author: Jean-Charles DELAY
* 			Mail  : jdelay@aldebaran-robotics.com
*/

#include <alcommon-ng/messaging/result_definition.hpp>

namespace AL {
  namespace Messaging {

    ResultDefinition::ResultDefinition() :
  fIsException(false), fValue(0) {
  }

  ResultDefinition::ResultDefinition(const VariableValue & val) :
  fIsException(false), fValue(val) {
  }

  ResultDefinition::~ResultDefinition() {
  }

  bool ResultDefinition::operator==(const ResultDefinition& rhs) const {
    return true; // FIXME

    //return (
    //  
    //  /*(this->v.as<std::string>() == rhs.exceptionMessage()) && */
    //  (this->request_id == rhs.getRequestId()) 
    //  /* FIXME(ckilner) ambiguous && (this->v == rhs.value()) */
    //  );
  }

  const bool& ResultDefinition::isException() const {
    return fIsException;
  }

  bool& ResultDefinition::isException() {
    return fIsException;
  }

  void ResultDefinition::setException(const std::string & message) {
    fIsException = true;
    fValue = VariableValue(message);
  }

  std::string ResultDefinition::exceptionMessage() const {
    return fValue.as<std::string>();
  }

  const VariableValue & ResultDefinition::value() const {
    return fValue;
  }

  VariableValue & ResultDefinition::value() {
    return fValue;
  }

  }
}
