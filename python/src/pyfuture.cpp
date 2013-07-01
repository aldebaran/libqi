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
      : qi::Future<qi::AnyValue>(fut)
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

    void PyFuture::addCallback(boost::python::object callable) {

      //because we use shared_ptr, we get a correct pyObject here.
      boost::python::object obj(shared_from_this());
      //we store a ref on ourself, because our future can get out of scope.
      //so the shared future state will keep a ref on us. When the promise will
      //be destroyed this will destroy the ref on us.
      GILScopedUnlock _unlock;
      connect(boost::bind<void>(&pyFutureCb, callable, obj));
    }

    typedef boost::shared_ptr<PyFuture> PyFuturePtr;



    class PyPromise: public qi::Promise<qi::AnyValue> {
    public:
      PyPromise() {};

      PyPromise(boost::python::object callable)
        : qi::Promise<qi::AnyValue> (boost::bind<void>(&pyFutureCbProm, callable, this))
      {
      }

      void setValue(const boost::python::object &pyval) {
        //TODO: remove the useless copy here.
        qi::AnyValue gvr = qi::AnyValue::from(pyval);
        {
          GILScopedUnlock _unlock;
          qi::Promise<qi::AnyValue>::setValue(gvr);
        }
      }

      PyFuturePtr future() {
        PyFuturePtr pfp(new PyFuture);
        *pfp = qi::Promise<qi::AnyValue>::future();
        return pfp;
      }
    };

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

      boost::python::class_<PyFuture, boost::shared_ptr<PyFuture> >("Future")
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
