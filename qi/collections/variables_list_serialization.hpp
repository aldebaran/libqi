/*
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2010 Aldebaran Robotics
*/

#ifndef    QI_MESSAGING_VARIABLES_LIST_SERIALIZATION_HPP_
# define   QI_MESSAGING_VARIABLES_LIST_SERIALIZATION_HPP_

#include <qi/collections/variables_list.hpp>

namespace boost {
  namespace serialization {
    template<class Archive>
    void serialize(Archive &, qi::messaging::EmptyValue &, const unsigned int) {
      return;
    }

    template<class Archive>
    void serialize(Archive &ar, qi::messaging::VariableValue &value, const unsigned int) {
      ar & boost::serialization::make_nvp("value", value.value());
    }
  }
}

#endif  /* !QI_MESSAGING_VARIABLES_LIST_SERIALIZATION_PP_ */
