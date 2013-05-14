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
  class SignalBase;
  namespace py {
    boost::python::object makePySignal(const std::string &signature);
    qi::SignalBase *getSignal(boost::python::object obj);
    void export_pysignal();
  }
}

#endif	    /* !PYSIGNAL_HPP_ */
