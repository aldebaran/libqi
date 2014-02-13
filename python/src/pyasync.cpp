/*
**  Copyright (C) 2013 Aldebaran Robotics
**  See COPYING for the license
*/
#include "pysignal.hpp"
#include <boost/python.hpp>
#include <boost/python/raw_function.hpp>
#include <qitype/signal.hpp>
#include <qitype/anyobject.hpp>
#include <qimessaging/python-gil.hpp>
#include <qi/periodictask.hpp>
#include "error.hpp"
#include "pyfuture.hpp"
#include "pyobject.hpp"
#include "pythreadsafeobject.hpp"


qiLogCategory("py.signal");

namespace qi { namespace py {


    static void pyPeriodicCb(const PyThreadSafeObject& callable) {
      GILScopedLock _gil;
      PY_CATCH_ERROR(callable.object()());
    }

    class PyPeriodicTask : public qi::PeriodicTask {
    public:
      void setCallback(boost::python::object callable) {
        if (!PyCallable_Check(callable.ptr()))
          throw std::runtime_error("Not a callable");
        qi::PeriodicTask::setCallback(boost::bind<void>(pyPeriodicCb, PyThreadSafeObject(callable)));
      }
    };

    static void pyAsync(qi::Promise<boost::python::object> prom, PyThreadSafeObject safeargs) {
      GILScopedLock _gil;

      boost::python::object args = safeargs.object();
      boost::python::object callable = args[0];


      boost::python::list largs;
      for (int i = 1; i < boost::python::len(args); ++i)
        largs.append(args[i]);


      try {
        prom.setValue(callable(*boost::python::tuple(largs)));
      } catch (const boost::python::error_already_set &) {
        prom.setError(PyFormatError());
      }
    }

    //the returned future is not canceable
    static boost::python::object pyasyncParamShrinker(boost::python::tuple args, boost::python::dict kwargs) {
      //arg0 always exists
      //check args[0] is a callable
      boost::python::object callable = args[0];
      if (!PyCallable_Check(callable.ptr()))
        throw std::runtime_error("Not a callable");

      qi::uint64_t delay = boost::python::extract<qi::uint64_t>(kwargs.get("delay", 0));

      //Does not use PyThreadSafeObject, because, setValue will be done under the lock
      //and the the future will be a PyFuture, that will be destroyed and use in the python world
      //under the lock too.
      qi::Promise<boost::python::object> prom;

      boost::function<void (void)> f = boost::bind<void>(&pyAsync, prom, PyThreadSafeObject(args));

      qi::getEventLoop()->async(f, delay);
      return boost::python::object(qi::py::toPyFuture(prom.future()));
    }


    void export_pyasync() {
      boost::python::object async = boost::python::raw_function(&pyasyncParamShrinker, 1);

      async.attr("__doc__") = "async(callback [, delay=usec] [, arg1, ..., argn]) -> future\n"
                              ":param callback: the callback that will be called\n"
                              ":param delay: an optional delay in microseconds\n"
                              ":return: a future with the return value of the function\n"
                              "\n";

      boost::python::def("async", async);

      boost::python::class_<PyPeriodicTask, boost::shared_ptr<PyPeriodicTask>, boost::noncopyable >("PeriodicTask")
        .def(boost::python::init<>())
        .def("setCallback", &PyPeriodicTask::setCallback,
             "setCallback(callable)\n"
             ":param callable: a python callable, could be a method or a function\n"
             "\n"
             "set the callback used by the periodictask, this function can only be called once")
        .def("setUsPeriod", &PyPeriodicTask::setUsPeriod,
             "setUsPeriod(usPeriod)\n"
             ":param usPeriod: the period in microseconds\n"
             "\n"
             "Set the call interval in microseconds.\n"
             "This call will wait until next callback invocation to apply the change.\n"
             "To apply the change immediately, use: \n"
             "\n"
             ".. code-block:: python\n"
             "\n"
             "   task.stop()\n"
             "   task.setUsPeriod()\n"
             "   task.start()\n"
             "\n")
        .def("start", &PyPeriodicTask::start,
             "start(immediate)\n"
             ":param immediate: immediate if true, first schedule of the task will happen with no delay.\n"
             "\n"
             "start the periodic task at specified period. No effect if already running\n"
             "\n"
             ".. warning::\n"
             "   concurrent calls to start() and stop() will result in undefined behavior."
             "\n")
        .def("stop", &PyPeriodicTask::stop,
             "stop()\n"
             "Stop the periodic task. When this function returns, the callback will not be called anymore.\n"
             "Can be called from within the callback function\n"
             "\n"
             ".. warning::\n"
             "   concurrent calls to start() and stop() will result in undefined behavior."
             "\n")
        .def("asyncStop", &PyPeriodicTask::asyncStop,
             "asyncStop()\n"
             "Can be called from within the callback function"
             "Request for periodic task to stop asynchronously")
        .def("compensateCallbackTime", &PyPeriodicTask::compensateCallbackTime,
             "compensateCallbackTime(compensate)\n"
             ":param compensate: If true, call interval will take into account call duration to maintain the period.")
        .def("setName", &PyPeriodicTask::setName,
             "setName(name)\n"
             "Set name for debugging and tracking purpose")
        .def("isRunning", &PyPeriodicTask::isRunning,
             "isRunning() -> bool\n"
             ":return: true is task is running\n")
        .def("isStopping", &PyPeriodicTask::isStopping,
             "isStopping() -> bool\n"
             ":return: whether state is stopping or stopped.\n"
             "\n"
             "Can be called from within the callback to know if stop() or asyncStop()  was called.")
          ;
    }

  }
}
