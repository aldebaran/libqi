/*
**  Copyright (C) 2013 Aldebaran Robotics
**  See COPYING for the license
*/
#include "pyfuture.hpp"
#include <qi/future.hpp>
#include <boost/python.hpp>
#include "gil.hpp"
#include "error.hpp"
#include "pythreadsafeobject.hpp"

namespace qi {
  namespace py {

    void pyFutureCb(const qi::Future<qi::AnyValue>& fut, const PyThreadSafeObject& callable) {
      GILScopedLock _lock;
      //reconstruct a pyfuture from the c++ future (the c++ one is always valid here)
      //both pypromise and pyfuture could have disappeared here.
      PY_CATCH_ERROR(callable.object()(PyFuture(fut)));
    }

    class PyPromise;
    static void pyFutureCbProm(const PyThreadSafeObject &callable, PyPromise *pp) {
      GILScopedLock _lock;
      PY_CATCH_ERROR(callable.object()(pp));
    }


    PyFuture::PyFuture()
    {}


    PyFuture::PyFuture(const qi::Future<qi::AnyValue>& fut)
      : qi::Future<qi::AnyValue>(fut)
    {}

    boost::python::object PyFuture::value(int msecs) const {
      qi::AnyValue gv;
      {
        GILScopedUnlock _unlock;
        //throw in case of error
        gv = qi::Future<qi::AnyValue>::value(msecs);
      }
      return gv.to<boost::python::object>();
    }

    std::string PyFuture::error(int msecs) const {
      GILScopedUnlock _unlock;
      return qi::Future<qi::AnyValue>::error(msecs);
    }

    FutureState PyFuture::wait(int msecs) const {
      GILScopedUnlock _unlock;
      return qi::Future<qi::AnyValue>::wait(msecs);
    }

    bool PyFuture::hasError(int msecs) const{
      GILScopedUnlock _unlock;
      return qi::Future<qi::AnyValue>::hasError(msecs);
    }

    bool PyFuture::hasValue(int msecs) const {
      GILScopedUnlock _unlock;
      return qi::Future<qi::AnyValue>::hasValue(msecs);
    }

    void PyFuture::addCallback(const boost::python::object &callable) {
      PyThreadSafeObject obj(callable);
      {
        GILScopedUnlock _unlock;
        connect(boost::bind<void>(&pyFutureCb, _1, obj));
      }
    }

    PyPromise::PyPromise()
    {
    }

    PyPromise::PyPromise(const qi::Promise<qi::AnyValue> &ref)
      : qi::Promise<qi::AnyValue>(ref)
    {
    }

    PyPromise::PyPromise(boost::python::object callable)
      : qi::Promise<qi::AnyValue> (boost::bind<void>(&pyFutureCbProm, PyThreadSafeObject(callable), this))
    {
    }

    void PyPromise::setValue(const boost::python::object &pyval) {
      //TODO: remove the useless copy here.
      qi::AnyValue gvr = qi::AnyValue::from(pyval);
      {
        GILScopedUnlock _unlock;
        qi::Promise<qi::AnyValue>::setValue(gvr);
      }
    }

    PyFuture PyPromise::future() {
      return qi::Promise<qi::AnyValue>::future();
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

          .def("setCanceled", &PyPromise::setCanceled,
               "setCanceled() -> None\n"
               "Set the state of the promise to Canceled")

          .def("setError", &PyPromise::setError,
               "setError(error) -> None\n"
               "Set the error of the promise")

          .def("setValue", &PyPromise::setValue,
               "setValue(value) -> None\n"
               "Set the value of the promise")

          .def("future", &PyPromise::future,
               "future() -> qi.Future\n"
               "Get a qi.Future from the promise, you can get multiple from the same promise.");

      boost::python::class_<PyFuture>("Future", boost::python::no_init)
          .def("value", &PyFuture::value, (boost::python::args("timeout") = qi::FutureTimeout_Infinite),
               "value(timeout) -> value\n"
               "return the value of the Future. Block until the future is ready. Raise an exception if the future has error. "
               "If a timeout is specified in parameter, and the future is not ready past that time, the function raise an exception.")

          .def("error", &PyFuture::error, (boost::python::args("timeout") = qi::FutureTimeout_Infinite),
               "error(timeout) -> None\n"
               "return the error of the Future. Block until the future is ready. "
               "Raise an exception if the function timeout or the future has no error")

          .def("wait", &PyFuture::wait, (boost::python::args("timeout") = qi::FutureTimeout_Infinite),
               "wait(timeout) -> qi.FutureState\n"
               "Wait for the future to be ready. Raise an exception on timeout. Return a qi.FutureState." )

          .def("hasError", &PyFuture::hasError, (boost::python::args("timeout") = qi::FutureTimeout_Infinite),
               "hasError(timeout) -> bool\n"
               "Return true or false depending on the future having an error. Raise an exception on timeout")

          .def("hasValue", &PyFuture::hasValue, (boost::python::args("timeout") = qi::FutureTimeout_Infinite),
               "hasValue(timeout) -> bool\n"
               "Return true or false depending on the future having a value. Raise an exception on timeout")

          .def("cancel", &PyFuture::cancel,
               "cancel() -> None\n"
               "If the future is cancelable, ask for cancelation.")

          .def("isFinished", &PyFuture::isFinished,
               "isFinished() -> bool\n"
               "Is the future not running anymore? (true if hasError or hasValue or isCanceled)")

          .def("isRunning", &PyFuture::isRunning,
               "isRunning() -> bool\n"
               "Is the future still running?")

          .def("isCanceled", &PyFuture::isCanceled,
               "isCanceled() -> bool\n"
               "Is the future canceled?")

          .def("isCanceleable", &PyFuture::isCanceleable,
               "isCanceleable() -> bool\n"
               "Is the future canceleable. (not all future are canceleable)")

          .def("addCallback", &PyFuture::addCallback,
               "addCallback(cb) -> None\n"
               "Add a callback that will be called then the future become ready. The callback will be called even if the future is already ready."
               "The first argument of the callback is the future itself.");
    }
  }
}
