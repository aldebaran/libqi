/*
**
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2013 Aldebaran Robotics
*/


#ifndef   	PYSIGNAL_HPP_
# define   	PYSIGNAL_HPP_

#include <boost/python.hpp>

namespace qi {
  namespace py {
    boost::python::object makePySignal();
    void export_pysignal();
  }
}

#endif	    /* !PYSIGNAL_HPP_ */
