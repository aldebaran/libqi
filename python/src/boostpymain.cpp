/*
**
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2013 Aldebaran Robotics
*/

#include <boost/python.hpp>
#include "pyapplication.hpp"
#include "pyfuture.hpp"
#include "pysession.hpp"
#include "pyobject.hpp"
#include "pysignal.hpp"
#include "pyproperty.hpp"
#include "pyservicedirectory.hpp"

qiLogCategory("qi.py");

BOOST_PYTHON_MODULE(_qi)
{
  qiLogDebug() << "PyEval_ThreadsInit()? : " << PyEval_ThreadsInitialized();

  // set the docstring of the current module scope
  boost::python::scope().attr("__doc__") = "qi bindings for python.";

  qi::py::export_pyfuture();
  qi::py::export_pyapplication();
  qi::py::export_pysession();
  qi::py::export_pyobject();
  qi::py::export_pysignal();
  qi::py::export_pyproperty();
  qi::py::export_pyservicedirectory();
}
