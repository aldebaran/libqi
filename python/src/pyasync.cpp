/*
**  Copyright (C) 2013 Aldebaran Robotics
**  See COPYING for the license
*/
#include "pysignal.hpp"
#include <boost/python.hpp>
#include <boost/python/raw_function.hpp>
#include <qitype/signal.hpp>
#include <qitype/anyobject.hpp>
#include "gil.hpp"
#include "error.hpp"
#include "pyfuture.hpp"
#include "pyobject.hpp"
#include "pythreadsafeobject.hpp"

qiLogCategory("py.signal");

namespace qi { namespace py {

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

      qi::getDefaultThreadPoolEventLoop()->async(f, delay);
      return boost::python::object(qi::py::toPyFuture(prom.future()));
    }


    void export_pyasync() {
      boost::python::object async = boost::python::raw_function(&pyasyncParamShrinker, 1);

      async.attr("__doc__") = "async(callback [, delay=usec] [, arg1, ..., argn]) -> future\n"
                              ":param callback: the callback that will be called\n"
                              ":param delay: a delay in microseconds\n"
                              ":return: a future with the return value of the function\n"
                              "\n";

      boost::python::def("async", async);
    }

  }
}

