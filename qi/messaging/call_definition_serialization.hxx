#pragma once
/*
* call_defintion.hxx
*
*  Created on: Oct 5, 2009 at 5:14:44 PM
*      Author: Jean-Charles DELAY
*      Mail  : jdelay@aldebaran-robotics.com
*/

#ifndef MESSAGING_CALL_DEFINITION_SERIALIZATION_HXX_
#define MESSAGING_CALL_DEFINITION_SERIALIZATION_HXX_

#include <qi/messaging/call_definition.hpp>
#include <qi/collections/variables_list_serialization.hpp>

namespace boost {
  namespace serialization {

    template<class Archive>
    void serialize(Archive & ar,
      qi::messaging::CallDefinition & callDef,
      const unsigned int version) {
        ar & boost::serialization::make_nvp("method", callDef.methodName());
        ar & boost::serialization::make_nvp("args",   callDef.args());
    }

  }
}

#endif  // MESSAGING_CALL_DEFINITION_SERIALIZATION_HXX_
