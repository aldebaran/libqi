/*
* call_defintion.cpp
*
*  Created on: Oct 5, 2009 at 4:35:09 PM
*      Author: Jean-Charles DELAY
* 			Mail  : jdelay@aldebaran-robotics.com
*/

#include <alcommon-ng/messaging/call_definition.hpp>

namespace AL {
  namespace Messaging {

    CallDefinition::CallDefinition () {}

    bool CallDefinition::operator==(const CallDefinition& rhs) const {
      return (
        (fMethodName == rhs.methodName())
        /* FIXME(ckilner) ambiguous && (list == rhs.getParameters()) */
        );
    }

    const std::string & CallDefinition::methodName() const {
      return fMethodName;
    }

    const std::vector<VariableValue> & CallDefinition::args() const {
      return fArgs;
    }

    std::string & CallDefinition::methodName() {
      return fMethodName;
    }

    std::vector<VariableValue> & CallDefinition::args() {
      return fArgs;
    }
  }
}
