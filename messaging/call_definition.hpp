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

      CallDefinition(const std::string& methodName) {
        fMethodName = methodName;
      }

      template<typename T>
      CallDefinition(const std::string& methodName, const T& arg0) : fMethodName(methodName) {
        fArgs.push_back(arg0);
      }
      // ... more templates ...

      ~CallDefinition() {}
      bool operator==(const CallDefinition& rhs) const;

      const std::string & methodName() const;
      std::string & methodName();
      const ArgumentList& args() const;
      ArgumentList& args();

    private:
      std::string  fMethodName;
      ArgumentList fArgs;
    };
  }
}

#endif  // MESSAGING_CALL_DEFINITION_HPP_
