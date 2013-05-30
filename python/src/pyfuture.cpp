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

namespace qi {
  namespace py {



    static void pyFutureCb(boost::python::object callable, boost::python::object pp) {
      GILScopedLock _lock;
      PY_CATCH_ERROR(callable(pp));
    }

    class PyPromise;
    static void pyFutureCbProm(boost::python::object callable, PyPromise *pp) {
      GILScopedLock _lock;
      PY_CATCH_ERROR(callable(pp));
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
        //throw in case of error
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

      //because we use shared_ptr, we get a correct pyObject here.
      boost::python::object obj(shared_from_this());
      //we store a ref on ourself, because our future can get out of scope.
      //so the shared future state will keep a ref on us. When the promise will
      //be destroyed this will destroy the ref on us.
      GILScopedUnlock _unlock;
      connect(boost::bind<void>(&pyFutureCb, callable, obj));
    }

    typedef boost::shared_ptr<PyFuture> PyFuturePtr;



    class PyPromise: public qi::Promise<qi::GenericValue> {
    public:
      PyPromise() {};

      PyPromise(boost::python::object callable)
        : qi::Promise<qi::GenericValue> (boost::bind<void>(&pyFutureCbProm, callable, this))
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

      PyFuturePtr future() {
        PyFuturePtr pfp(new PyFuture);
        *pfp = qi::Promise<qi::GenericValue>::future();
        return pfp;
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

      boost::python::class_<PyFuture, boost::shared_ptr<PyFuture> >("Future")
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
