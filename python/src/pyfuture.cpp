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

    template <typename T>
    static void pySignalCb(boost::python::object callable, T *pp) {
      GILScopedLock _lock;
      callable(pp);
    }


    PyFuture::PyFuture()
    {}

    PyFuture::PyFuture(const PyFuture& fut)
      : qi::Future<qi::GenericValue>(fut)
    {}

    PyFuture::PyFuture(const qi::Future<qi::GenericValue>& fut)
      : qi::Future<qi::GenericValue>(fut)
    {}

    boost::python::object PyFuture::value(int msecs) const {
      qi::GenericValue gv;
      {
        GILScopedUnlock _unlock;
        gv = qi::Future<qi::GenericValue>::value(msecs);
      }
      return gv.to<boost::python::object>();
    }

    std::string PyFuture::error(int msecs) const {
      GILScopedUnlock _unlock;
      return qi::Future<qi::GenericValue>::error(msecs);
    }

    FutureState PyFuture::wait(int msecs) const {
      GILScopedUnlock _unlock;
      return qi::Future<qi::GenericValue>::wait(msecs);
    }

    bool PyFuture::hasError(int msecs) const{
      GILScopedUnlock _unlock;
      return qi::Future<qi::GenericValue>::hasError(msecs);
    }

    bool PyFuture::hasValue(int msecs) const {
      GILScopedUnlock _unlock;
      return qi::Future<qi::GenericValue>::hasValue(msecs);
    }

    void PyFuture::add_callback(boost::python::object callable) {
      connect(boost::bind<void>(&pySignalCb<PyFuture>, callable, this));
    }



    class PyPromise: public qi::Promise<qi::GenericValue> {
    public:
      PyPromise() {};

      PyPromise(boost::python::object callable)
        : qi::Promise<qi::GenericValue> (boost::bind<void>(&pySignalCb<PyPromise>, callable, this))
      {
      }

      void setValue(const boost::python::object &pyval) {
        //TODO: remove the useless copy here.
        qi::GenericValue gvr = qi::GenericValue::from(pyval);
        {
          GILScopedUnlock _unlock;
          qi::Promise<qi::GenericValue>::setValue(gvr);
        }
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
