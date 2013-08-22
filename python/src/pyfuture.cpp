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
#include "error.hpp"
#include "pythreadsafeobject.hpp"

namespace qi {
  namespace py {

    void pyFutureCb(const qi::Future<qi::AnyValue>& fut, const PyThreadSafeObject& callable) {
      GILScopedLock _lock;
      PyFuturePtr pfut(new PyFuture(fut));
      //reconstruct a pyfuture from the c++ future (the c++ one is always valid here)
      //both pypromise and pyfuture could have disappeared here.
      PY_CATCH_ERROR(callable.object()(pfut));
    }

    class PyPromise;
    static void pyFutureCbProm(const boost::python::object &callable, PyPromise *pp) {
      GILScopedLock _lock;
      PY_CATCH_ERROR(callable(pp));
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

    PyPromise::PyPromise(boost::python::object callable)
      : qi::Promise<qi::AnyValue> (boost::bind<void>(&pyFutureCbProm, callable, this))
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

    PyFuturePtr PyPromise::future() {
      PyFuturePtr pfp(new PyFuture);
      *pfp = qi::Promise<qi::AnyValue>::future();
      return pfp;
    }

    boost::python::object makeFuture(qi::Future<qi::AnyReference> fut) {
      PyPromise prom;
      qi::adaptFuture(fut, prom, qi::FutureValueConverterTakeAnyReference<qi::AnyValue>());
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
          .def("setCanceled", &PyPromise::setCanceled)
          .def("setError", &PyPromise::setError)
          .def("setValue", &PyPromise::setValue)
          .def("future", &PyPromise::future);

      boost::python::class_<PyFuture, boost::shared_ptr<PyFuture> >("Future", boost::python::no_init)
          .def("value", &PyFuture::value, (boost::python::args("timeout") = qi::FutureTimeout_Infinite))
          .def("error", &PyFuture::error, (boost::python::args("timeout") = qi::FutureTimeout_Infinite))
          .def("wait", &PyFuture::wait, (boost::python::args("timeout") = qi::FutureTimeout_Infinite))
          .def("hasError", &PyFuture::hasError, (boost::python::args("timeout") = qi::FutureTimeout_Infinite))
          .def("hasValue", &PyFuture::hasValue, (boost::python::args("timeout") = qi::FutureTimeout_Infinite))
          .def("cancel", &PyFuture::cancel)
          .def("isFinished", &PyFuture::isFinished)
          .def("isRunning", &PyFuture::isRunning)
          .def("isCanceled", &PyFuture::isCanceled)
          .def("isCanceleable", &PyFuture::isCanceleable)
          .def("addCallback", &PyFuture::addCallback);
    }
  }
}
