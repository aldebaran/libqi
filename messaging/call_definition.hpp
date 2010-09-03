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

    /// <summary>
    /// A basic call definition used to define a remote procedure call.
    /// This class and its attributes must be serializable in order to be sent to
    /// the server."client" class
    /// </summary>
    class CallDefinition {
    public:
      CallDefinition ();

      CallDefinition(const std::string& methodName) : fMethodName(methodName) {
      }

      CallDefinition(const std::string& methodName, const ArgumentList& args) : fMethodName(methodName), fArgs(args) {
      }

      template<typename T0>
      CallDefinition(const std::string& methodName, const T0& arg0) : fMethodName(methodName) {
        fArgs.push_back(arg0);
      }

      template<typename T0, typename T1>
      CallDefinition(const std::string& methodName, const T0& arg0, const T1& arg1) : fMethodName(methodName) {
        fArgs.push_back(arg0);
        fArgs.push_back(arg1);
      }

      template<typename T0, typename T1, typename T2>
      CallDefinition(const std::string& methodName, const T0& arg0, const T1& arg1, const T2& arg2) : fMethodName(methodName) {
        fArgs.push_back(arg0);
        fArgs.push_back(arg1);
        fArgs.push_back(arg2);
      }

      template<typename T0, typename T1, typename T2, typename T3>
      CallDefinition(const std::string& methodName, const T0& arg0, const T1& arg1, const T2& arg2, const T3& arg3) : fMethodName(methodName) {
        fArgs.push_back(arg0);
        fArgs.push_back(arg1);
        fArgs.push_back(arg2);
        fArgs.push_back(arg3);
      }

      template<typename T0, typename T1, typename T2, typename T3, typename T4>
      CallDefinition(const std::string& methodName, const T0& arg0, const T1& arg1, const T2& arg2, const T3& arg3, const T4& arg4) : fMethodName(methodName) {
        fArgs.push_back(arg0);
        fArgs.push_back(arg1);
        fArgs.push_back(arg2);
        fArgs.push_back(arg3);
        fArgs.push_back(arg4);
      }

      template<typename T0, typename T1, typename T2, typename T3, typename T4, typename T5>
      CallDefinition(const std::string& methodName, const T0& arg0, const T1& arg1, const T2& arg2, const T3& arg3, const T4& arg4, const T5& arg5) : fMethodName(methodName) {
        fArgs.push_back(arg0);
        fArgs.push_back(arg1);
        fArgs.push_back(arg2);
        fArgs.push_back(arg3);
        fArgs.push_back(arg4);
        fArgs.push_back(arg5);
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
