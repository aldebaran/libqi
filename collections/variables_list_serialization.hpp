/*
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2010 Aldebaran Robotics
*/

#ifndef    AL_MESSAGING_VARIABLES_LIST_SERIALIZATION_HPP_
# define   AL_MESSAGING_VARIABLES_LIST_SERIALIZATION_HPP_

#include <boost/variant.hpp>

namespace boost {
  namespace serialization {
    template<class Archive>
    void serialize(Archive &, AL::Messaging::EmptyValue &, const unsigned int) {
      return;
    }

    template<class Archive>
    void serialize(Archive &ar, AL::Messaging::VariableValue &value, const unsigned int) {
      ar & boost::serialization::make_nvp("value", value.value());
    }
  }
}

#endif  /* !AL_MESSAGING_VARIABLES_LIST_SERIALIZATION_PP_ */
