#pragma once
/*
* result_definition_serialization.hxx
*
*  Created on: Oct 5, 2009 at 5:34:28 PM
*      Author: Jean-Charles DELAY
*      Mail  : jdelay@aldebaran-robotics.com
*/


#ifndef MESSAGING_RESULT_DEFINITION_SERIALIZATION_HXX_
#define MESSAGING_RESULT_DEFINITION_SERIALIZATION_HXX_

#include <alcommon-ng/messaging/result_definition.hpp>
#include <alcommon-ng/collections/variables_list_serialization.hpp>

namespace boost {
  namespace serialization {
    template<class Archive>
    void serialize(Archive & ar,
      AL::Messaging::ResultDefinition& result,
      const unsigned int version) {
      ar & boost::serialization::make_nvp("is_exception", result.isException());
      ar & boost::serialization::make_nvp("v", result.value());
    }
  }
}

#endif  // MESSAGING_RESULT_DEFINITION_SERIALIZATION_HXX_
