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


    class PySignal : qi::SignalBase {
    public:
      PySignal(const std::string &signature)
        : qi::SignalBase(signature)
      {
      }

      ~PySignal() {
      }

      qi::uint64_t connect(boost::python::object callable) {
        return qi::SignalBase::connect(boost::bind<void>(callable, this));
      }

      bool disconnect(qi::uint64_t id) {
        return qi::SignalBase::disconnect(id);
      }

      //renamed to avoid "hidden overload" warning. Yes we know :)
      void trig(boost::python::object arg) {
        qi::GenericValueRef gvr(arg);
        qi::SignalBase::trigger(gvr.asTupleValuePtr());
      }

    };

    boost::python::object makePySignal(const std::string &signature) {
      return boost::python::object(PySignal(signature));
    }

    void export_pysignal() {
      boost::python::class_<PySignal>("Signal", boost::python::init<const std::string &>())
          .def("connect", &PySignal::connect, (boost::python::arg("callback")))
          .def("disconnect", &PySignal::disconnect, (boost::python::arg("id")))
          .def("trigger", &PySignal::trig, (boost::python::arg("arguments")));
    }

  }
}
