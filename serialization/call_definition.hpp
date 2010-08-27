/*
* call_defintion.hpp
*
*  Created on: Oct 5, 2009 at 4:35:09 PM
*      Author: Jean-Charles DELAY
* 			Mail  : jdelay@aldebaran-robotics.com
*/

#ifndef MESSAGING_CALL_DEFINITION_HPP_
#define MESSAGING_CALL_DEFINITION_HPP_

#include <string>
#include <vector>
#include <alcommon-ng/collections/variables_list.hpp>

namespace AL {
  namespace Messaging {

    /**
    * @brief A basic call definition used to define a remote procedure call.
    * This class and its attributes must be serializable in order to be sent to
    * the server.
    * "client" class
    */
    class CallDefinition {
    public:
      CallDefinition ();
      ~CallDefinition() {}
      bool operator==(const CallDefinition& rhs) const;

      const std::string & methodName () const;
      const std::string & moduleName () const;
      std::string & methodName();
      std::string & moduleName();
      const std::vector<VariableValue>& args() const;
      std::vector<VariableValue>& args();

    private:
      /**
      * moduleName
      */
      std::string fModuleName;

      /**
      * methodName
      */
      std::string fMethodName;

      /**
      * list parameter
      */
      std::vector<VariableValue> fArgs;
    };
  }
}

#endif  // MESSAGING_CALL_DEFINITION_HPP_
