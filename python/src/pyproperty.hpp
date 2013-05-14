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
  class PropertyBase;
  namespace py {
    boost::python::object makePyProperty(const std::string &signature);
    qi::PropertyBase *getProperty(boost::python::object obj);
    void export_pyproperty();
  }
}

#endif	    /* !PYSESSION_HPP_ */
