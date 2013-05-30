/*
**
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2013 Aldebaran Robotics
*/
#include "pysignal.hpp"
#include <boost/python.hpp>
#include <boost/python/raw_function.hpp>
#include <qitype/signal.hpp>
#include <qitype/genericobject.hpp>
#include "gil.hpp"
#include "error.hpp"

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
      PY_CATCH_ERROR(ret = callable(*boost::python::tuple(args)));
      return qi::GenericValueRef(ret).clone();
    }

    class PySignal : public qi::SignalBase {
    public:
      PySignal(const qi::Signature &signature = "[m]")
        : qi::SignalBase(signature)
      {
      }

      ~PySignal() {
      }

      qi::uint64_t connect(boost::python::object callable) {
        GILScopedUnlock _unlock;
        //no need to store a ptr on ourself. (this exist if the callback is triggered)
        return qi::SignalBase::connect(qi::makeDynamicGenericFunction(boost::bind(pysignalCb, _1, callable)));
      }

      bool disconnect(qi::uint64_t id) {
        GILScopedUnlock _unlock;
        return qi::SignalBase::disconnect(id);
      }

      //this function is named trigger in the qi.Signal object,
      //the python wrapper add a __call__ method bound to this one. (see qi/__init__.py)
      void trig(boost::python::tuple args, boost::python::dict kwargs) {
        GILScopedUnlock _unlock;
        qi::SignalBase::trigger(qi::GenericValueRef(args).asDynamic().asTupleValuePtr());
      }
    };

    class PyProxySignal {
    public:
      PyProxySignal(qi::ObjectPtr obj, const qi::MetaSignal &signal)
        : _obj(obj)
        , _sigid(signal.uid()){
      }

      qi::uint64_t connect(boost::python::object callable) {
        GILScopedUnlock _unlock;
        //no need to store a ptr on ourself. (this exist if the callback is triggered)
        return _obj->connect(_sigid, qi::makeDynamicGenericFunction(boost::bind(pysignalCb, _1, callable)));
      }

      bool disconnect(qi::uint64_t id) {
        GILScopedUnlock _unlock;
        return _obj->disconnect(id).hasValue();
      }

      //this function is named trigger in the qi.Signal object,
      //the python wrapper add a __call__ method bound to this one. (see qi/__init__.py)
      void trig(boost::python::tuple args, boost::python::dict kwargs) {
        GILScopedUnlock _unlock;
        _obj->metaPost(_sigid, qi::GenericValueRef(args).asDynamic().asTupleValuePtr());
      }

    private:
      qi::ObjectPtr _obj;
      unsigned int  _sigid;
    };

    qi::SignalBase *getSignal(boost::python::object obj) {
      return boost::python::extract<PySignal*>(obj);
    }

    boost::python::object makePySignal(const std::string &signature) {
      GILScopedUnlock _unlock;
      return boost::python::object(PySignal(signature));
    }

    boost::python::object makePyProxySignal(const qi::ObjectPtr &obj, const qi::MetaSignal &signal) {
      return boost::python::object(PyProxySignal(obj, signal));
    }

    template <typename T>
    static boost::python::object signal_param_shrinker(boost::python::tuple args, boost::python::dict kwargs) {
      T& pys = boost::python::extract<T&>(args[0]);
      boost::python::list l;
      for (unsigned i = 1; i < boost::python::len(args); ++i)
        l.append(args[i]);
      pys.trig(boost::python::tuple(l), kwargs);
      return boost::python::object();
    }

    void export_pysignal() {
      boost::python::class_<PySignal>("Signal", boost::python::init<>())
          .def(boost::python::init<const std::string &>())
          .def("connect", &PySignal::connect, (boost::python::arg("callback")))
          .def("disconnect", &PySignal::disconnect, (boost::python::arg("id")))
          .def("disconnect_all", &PySignal::disconnectAll)
          .def("__call__", boost::python::raw_function(&signal_param_shrinker<PySignal>));

      boost::python::class_<PyProxySignal>("_ProxySignal", boost::python::no_init)
          .def("connect", &PyProxySignal::connect, (boost::python::arg("callback")))
          .def("disconnect", &PyProxySignal::disconnect, (boost::python::arg("id")))
          .def("__call__", boost::python::raw_function(&signal_param_shrinker<PyProxySignal>));

    }

  }
}
