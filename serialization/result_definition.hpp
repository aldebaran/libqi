/*
* result_defintion.hpp
*
*  Created on: Oct 5, 2009 at 5:34:02 PM
*      Author: Jean-Charles DELAY
* 			Mail  : jdelay@aldebaran-robotics.com
*/

#ifndef MESSAGING_RESULT_DEFINITION_HPP_
#define MESSAGING_RESULT_DEFINITION_HPP_

#include <alcommon-ng/collections/variables_list.hpp>

namespace AL {
  namespace Messaging {

    /**
    * @brief A exception class definition used to define a result returned by a
    * remote procedure call.
    * This class and its attributes must be serializable in order to be sent to
    * the server.
    */
    class ResultDefinition {
    public:
      ResultDefinition ();

      ResultDefinition (const VariableValue & val);
      bool operator==(const ResultDefinition& rhs) const;
      virtual ~ResultDefinition();

      void setException(const std::string & message);

      template <typename T>
      void value (const T & val);

      const bool& isException() const;
      bool& isException();

      std::string exceptionMessage() const;

      const VariableValue & value() const;
      VariableValue& value();

    private:


      /**
      * is exception during call
      */
      bool fIsException;

      /**
      * pcall -> ID
      * callVoid -> int 42 ?
      * call -> exception.what() or result
      */
      VariableValue fValue;
    };

  }
}

#include <alcommon-ng/serialization/result_definition.hxx>

#endif  // MESSAGING_RESULT_DEFINITION_HPP_
