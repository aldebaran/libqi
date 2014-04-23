/*
**  Copyright (C) 2013 Aldebaran Robotics
**  See COPYING for the license
*/
#include <boost/python.hpp>
#include <qi/log.hpp>
#include <qipython/pyexport.hpp>

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

  qi::py::export_all();
}
