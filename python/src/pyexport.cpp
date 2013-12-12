/*
**  Copyright (C) 2013 Aldebaran Robotics
**  See COPYING for the license
*/
#include <qipython/pyexport.hpp>
#include <qipython/pyapplication.hpp>
#include <qipython/pyfuture.hpp>
#include <qipython/pysession.hpp>
#include <qipython/pyobject.hpp>
#include <qipython/pysignal.hpp>
#include <qipython/pyproperty.hpp>
#include <qipython/pyobjectfactory.hpp>
#include <qipython/pyasync.hpp>
#include <qipython/pylog.hpp>
#include <qipython/pypath.hpp>
#include <qipython/pytranslator.hpp>

namespace qi {
  namespace py {
    void export_all()
    {
      qi::py::export_pyfuture();
      qi::py::export_pyapplication();
      qi::py::export_pysession();
      qi::py::export_pyobject();
      qi::py::export_pysignal();
      qi::py::export_pyproperty();
      qi::py::export_pyobjectfactory();
      qi::py::export_pyapplicationsession();
      qi::py::export_pyasync();
      qi::py::export_pylog();
      qi::py::export_pypath();
      qi::py::export_pytranslator();
    }
  }
}
