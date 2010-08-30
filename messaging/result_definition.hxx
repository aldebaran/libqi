#pragma once
/*
* result_definition.hxx
*
*  Created on: Oct 5, 2009 at 5:34:28 PM
*      Author: Jean-Charles DELAY
* 			Mail  : jdelay@aldebaran-robotics.com
*/

#ifndef MESSAGING_RESULT_DEFINITION_HXX_
#define MESSAGING_RESULT_DEFINITION_HXX_

namespace AL {
  namespace Messaging {

    template <typename T>
    void ResultDefinition::value(const T & val) {
      fIsException = false;
      fValue = ReturnValue(val);
    }

  }
}

#endif  // MESSAGING_RESULT_DEFINITION_HXX_
