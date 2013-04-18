/*
**
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2013 Aldebaran Robotics
*/

#include "pyfuture.hpp"
#include <qi/future.hpp>
#include <boost/python.hpp>
#include "gil.hpp"

namespace qi {
  namespace py {

    class PyPromise;
    void pysignalCbP(boost::python::object callable, PyPromise *pp) {
      GILScopedLock _lock;
      callable(pp);
    }

    void pysignalCbF(boost::python::object callable, PyFuture *pp) {
      GILScopedLock _lock;
      callable(pp);
    }

    class PyPromise: public qi::Promise<qi::GenericValue> {
    public:
      PyPromise() {};

      PyPromise(boost::python::object callable)
        : qi::Promise<qi::GenericValue> (boost::bind<void>(&pysignalCbP, callable, this))
      {
      }

      void setValue(const boost::python::object &pyval) {
        //TODO: remove the useless copy here.
        qi::GenericValue gvr = qi::GenericValue::from(pyval);
        qi::Promise<qi::GenericValue>::setValue(gvr);
      }

      PyFuture future() {
        return qi::Promise<qi::GenericValue>::future();
      }
    };

    boost::python::object makeFuture(qi::Future<qi::GenericValuePtr> fut) {
      PyPromise prom;
      qi::adaptFuture(fut, prom, qi::FutureValueConverterTakeGenericValuePtr<qi::GenericValue>());
      return boost::python::object(prom.future());
    }


    void export_pyfuture() {
      boost::python::enum_<qi::FutureState>("FutureState")
          .value("None", qi::FutureState_None)
          .value("Running", qi::FutureState_Running)
          .value("Canceled", qi::FutureState_Canceled)
          .value("FinishedWithError", qi::FutureState_FinishedWithError)
          .value("FinishedWithValue", qi::FutureState_FinishedWithValue);

      boost::python::enum_<qi::FutureTimeout>("FutureTimeout")
          .value("None", qi::FutureTimeout_None)
          .value("Infinite", qi::FutureTimeout_Infinite);

      boost::python::class_<PyPromise>("Promise")
          .def(boost::python::init<boost::python::object>())
          .def("set_canceled", &PyPromise::setCanceled)
          .def("set_error", &PyPromise::setError)
          .def("set_value", &PyPromise::setValue)
          .def("future", &PyPromise::future);

      boost::python::class_<PyFuture>("Future")
          .def("value", &PyFuture::value, (boost::python::args("timeout") = qi::FutureTimeout_Infinite))
          .def("error", &PyFuture::error, (boost::python::args("timeout") = qi::FutureTimeout_Infinite))
          .def("wait", &PyFuture::wait, (boost::python::args("timeout") = qi::FutureTimeout_Infinite))
          .def("has_error", &PyFuture::hasError, (boost::python::args("timeout") = qi::FutureTimeout_Infinite))
          .def("has_value", &PyFuture::hasValue, (boost::python::args("timeout") = qi::FutureTimeout_Infinite))
          .def("cancel", &PyFuture::cancel)
          .def("is_finished", &PyFuture::isFinished)
          .def("is_running", &PyFuture::isRunning)
          .def("is_canceled", &PyFuture::isCanceled)
          .def("is_canceleable", &PyFuture::isCanceleable)
          .def("add_callback", &PyFuture::add_callback);
    }
  }
}
