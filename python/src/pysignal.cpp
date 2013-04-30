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
#include "gil.hpp"
qiLogCategory("py.signal");

namespace qi { namespace py {

    static qi::GenericValuePtr pysignalCb(const std::vector<qi::GenericValuePtr>& cargs, boost::python::object callable) {
      GILScopedLock _lock;
      boost::python::list   args;
      boost::python::object ret;

      std::vector<qi::GenericValuePtr>::const_iterator it;
      for (it = cargs.begin(); it != cargs.end(); ++it) {
        args.append(it->to<boost::python::object>());
      }
      ret = callable(*boost::python::tuple(args));
      return qi::GenericValueRef(ret).clone();
    }

    class PySignal : public qi::SignalBase {
    public:
      PySignal(const std::string &signature = "[m]")
        : qi::SignalBase(signature)
      {
      }

      ~PySignal() {
      }

      qi::uint64_t connect(boost::python::object callable) {
        return qi::SignalBase::connect(qi::makeDynamicGenericFunction(boost::bind(pysignalCb, _1, callable)));
      }

      bool disconnect(qi::uint64_t id) {
        return qi::SignalBase::disconnect(id);
      }

      //this function is named trigger in the qi.Signal object,
      //the python wrapper add a __call__ method bound to this one. (see qi/__init__.py)
      void trig(boost::python::tuple args, boost::python::dict kwargs) {
        GILScopedUnlock _unlock;
        qi::SignalBase::trigger(qi::GenericValueRef(args).asDynamic().asTupleValuePtr());
      }

    };

    boost::python::object makePySignal(const std::string &signature) {
      return boost::python::object(PySignal(signature));
    }

    void export_pysignal() {
      boost::python::class_<PySignal>("Signal", boost::python::init<>())
          .def(boost::python::init<const std::string &>())
          .def("connect", &PySignal::connect, (boost::python::arg("callback")))
          .def("disconnect", &PySignal::disconnect, (boost::python::arg("id")))
          .def("disconnect_all", &PySignal::disconnectAll)
          .def("trigger", &PySignal::trig);
    }

  }
}
