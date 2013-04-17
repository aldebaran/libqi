/*
**
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2013 Aldebaran Robotics
*/
#include "pysignal.hpp"
#include <boost/python.hpp>
#include <qitype/signal.hpp>

namespace qi { namespace py {


    class PySignal {
    public:
      PySignal()
      {
      }

      ~PySignal() {
      }

      qi::uint64_t connect(boost::python::object callable) {
        return sig.connect(boost::bind<void>(callable, this));
      }

      bool disconnect(qi::uint64_t id) {
        return sig.disconnect(id);
      }

      void trigger(boost::python::object arg) {
        qi::GenericValueRef gvr(arg);
        sig.trigger(gvr.asTupleValuePtr());
      }

    private:
      qi::Signal<void (qi::GenericValue)> sig;
    };

    boost::python::object makePySignal() {
      boost::python::object ret;
      return ret;
    }

    void export_pysignal() {
      boost::python::class_<PySignal>("Signal")
          .def("connect", &PySignal::connect, (boost::python::arg("callback")))
          .def("disconnect", &PySignal::disconnect, (boost::python::arg("id")))
          .def("trigger", &PySignal::trigger, (boost::python::arg("arguments")));
    }

  }
}
