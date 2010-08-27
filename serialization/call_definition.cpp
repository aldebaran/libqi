/*
* call_defintion.cpp
*
*  Created on: Oct 5, 2009 at 4:35:09 PM
*      Author: Jean-Charles DELAY
* 			Mail  : jdelay@aldebaran-robotics.com
*/

#include <alcommon-ng/serialization/call_definition.hpp>

namespace AL {
  namespace Messaging {

    CallDefinition::CallDefinition () {}

    bool CallDefinition::operator==(const CallDefinition& rhs) const {
      return (
        (fMethodName == rhs.methodName()) &&
        (fModuleName == rhs.moduleName())
        /* FIXME(ckilner) ambiguous && (list == rhs.getParameters()) */
        );
    }

    const std::string & CallDefinition::methodName() const {
      return fMethodName;
    }

    const std::string & CallDefinition::moduleName() const {
      return fModuleName;
    }

    const std::vector<VariableValue> & CallDefinition::args() const {
      return fArgs;
    }

    std::string & CallDefinition::methodName() {
      return fMethodName;
    }

    std::string & CallDefinition::moduleName() {
      return fModuleName;
    }

    std::vector<VariableValue> & CallDefinition::args() {
      return fArgs;
    }
  }
}
