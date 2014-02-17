#pragma once
/*
**  Copyright (C) 2013 Aldebaran Robotics
**  See COPYING for the license
*/

#ifndef _PYTHON_SRC_PYSESSION_HPP_
#define _PYTHON_SRC_PYSESSION_HPP_

#include <boost/python.hpp>
#include <qimessaging/session.hpp>

namespace qi {
  namespace py {
    boost::python::object makePySession(boost::shared_ptr<qi::Session> ses);
    void export_pysession();
  }
}

#endif  // _PYTHON_SRC_PYSESSION_HPP_
