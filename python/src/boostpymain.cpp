/*
**  Copyright (C) 2013 Aldebaran Robotics
**  See COPYING for the license
*/
#include <boost/python.hpp>
#include "pyapplication.hpp"
#include "pyfuture.hpp"
#include "pysession.hpp"
#include "pyobject.hpp"
#include "pysignal.hpp"
#include "pyproperty.hpp"
#include "pyservicedirectory.hpp"
#include "pyobjectfactory.hpp"

qiLogCategory("qi.py");

BOOST_PYTHON_MODULE(_qi)
{

  boost::python::docstring_options doc_options(true, false, false);

  // Initialize in case we use _qi without importing qi
  // No-op when using qi
  Py_Initialize();
  PyEval_InitThreads();

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
  qi::py::export_pyobjectfactory();
}
