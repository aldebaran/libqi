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

      CallDefinition(const std::string& methodName) : fMethodName(methodName) {
      }

      CallDefinition(const std::string& methodName, const ArgumentList& args) : fMethodName(methodName), fArgs(args) {
      }

      template<typename T>
      CallDefinition(const std::string& methodName, const T& arg0) : fMethodName(methodName) {
        fArgs.resize(1);
        fArgs[0] = arg0;
      }
      template<typename T>
      CallDefinition(const std::string& methodName, const T& arg0, const T& arg1) : fMethodName(methodName) {
        fArgs.resize(2);
        fArgs[0] = arg0;
        fArgs[1] = arg1;
      }
      template<typename T>
      CallDefinition(const std::string& methodName, const T& arg0, const T& arg1, const T& arg2) : fMethodName(methodName) {
        fArgs.resize(3);
        fArgs[0] = arg0;
        fArgs[1] = arg1;
        fArgs[2] = arg2;
      }
      template<typename T>
      CallDefinition(const std::string& methodName, const T& arg0, const T& arg1, const T& arg2, const T& arg3) : fMethodName(methodName) {
        fArgs.resize(4);
        fArgs[0] = arg0;
        fArgs[1] = arg1;
        fArgs[2] = arg2;
        fArgs[3] = arg3;
      }
      template<typename T>
      CallDefinition(const std::string& methodName, const T& arg0, const T& arg1, const T& arg2, const T& arg3, const T& arg4) : fMethodName(methodName) {
        fArgs.resize(5);
        fArgs[0] = arg0;
        fArgs[1] = arg1;
        fArgs[2] = arg2;
        fArgs[3] = arg3;
        fArgs[4] = arg4;
      }
      template<typename T>
      CallDefinition(const std::string& methodName, const T& arg0, const T& arg1, const T& arg2, const T& arg3, const T& arg4, const T& arg5) : fMethodName(methodName) {
        fArgs.resize(6);
        fArgs[0] = arg0;
        fArgs[1] = arg1;
        fArgs[2] = arg2;
        fArgs[3] = arg3;
        fArgs[4] = arg4;
        fArgs[5] = arg5;
      }

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
