/*
**
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2013 Aldebaran Robotics
*/


#ifndef   	PYPROPERTY_HPP_
# define   	PYPROPERTY_HPP_

#include <boost/python.hpp>

namespace qi {
  namespace py {
    boost::python::object makePyProperty();
    void export_pyproperty();
  }
}

#endif	    /* !PYSESSION_HPP_ */
