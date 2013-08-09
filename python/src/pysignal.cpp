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
#include <qitype/anyobject.hpp>
#include "gil.hpp"
#include "error.hpp"
#include "pyfuture.hpp"

qiLogCategory("py.signal");

namespace qi { namespace py {

    static qi::AnyReference pysignalCb(const std::vector<qi::AnyReference>& cargs, boost::python::object callable) {
      GILScopedLock _lock;
      boost::python::list   args;
      boost::python::object ret;

      std::vector<qi::AnyReference>::const_iterator it;
      for (it = cargs.begin(); it != cargs.end(); ++it) {
        args.append(it->to<boost::python::object>());
      }
      PY_CATCH_ERROR(ret = callable(*boost::python::tuple(args)));
      return qi::AnyReference(ret).clone();
    }

    class PySignal {
    public:
      PySignal(const qi::Signature &signature = "m")
        : _sig(new qi::SignalBase(signature))
      {
      }

      PySignal(const PySignal& rhs)
        : _sig(new qi::SignalBase(qi::Signature("m")))
      {

        *_sig = *rhs._sig;
      }

      PySignal &operator=(const PySignal& rhs) {
        *_sig = *rhs._sig;
        return *this;
      }

      ~PySignal() {
        //the dtor can lock waiting for callback ends
        GILScopedUnlock _unlock;
        delete _sig;
      }

      boost::python::object connect(boost::python::object callable, bool _async = false) {
        GILScopedUnlock _unlock;
        //no need to store a ptr on ourself. (this exist if the callback is triggered)
        qi::uint64_t r = _sig->connect(qi::AnyFunction::fromDynamicFunction(boost::bind(pysignalCb, _1, callable)));
        GILScopedLock _lock;
        if (_async)
        {
          return boost::python::object(toPyFuture(qi::Future<qi::uint64_t>(r)));
        }

        return boost::python::object(r);
      }

      boost::python::object disconnect(qi::uint64_t id, bool _async = false) {
        GILScopedUnlock _unlock;
        bool r = _sig->disconnect(id);
        GILScopedLock _lock;
        if (_async)
        {
          return boost::python::object(toPyFuture(qi::Future<bool>(r)));
        }

        return boost::python::object(r);
      }

      boost::python::object disconnectAll(bool _async = false) {
        GILScopedUnlock _unlock;
        bool r = _sig->disconnectAll();
        GILScopedLock _lock;
        if (_async)
        {
          return boost::python::object(toPyFuture(qi::Future<bool>(r)));
        }
        return boost::python::object(r);
      }

      //this function is named trigger in the qi.Signal object,
      //the python wrapper add a __call__ method bound to this one. (see qi/__init__.py)
      void trig(boost::python::tuple args, boost::python::dict kwargs) {
        GILScopedUnlock _unlock;
        _sig->trigger(qi::AnyReference(args).asDynamic().asTupleValuePtr());
      }

    public:
      qi::SignalBase *_sig;
    };

    class PyProxySignal {
    public:
      PyProxySignal(qi::AnyObject obj, const qi::MetaSignal &signal)
        : _obj(obj)
        , _sigid(signal.uid()){
      }

      boost::python::object connect(boost::python::object callable, bool _async = false) {
        GILScopedUnlock _unlock;
        //no need to store a ptr on ourself. (this exist if the callback is triggered)
        qi::FutureSync<SignalLink> f = _obj->connect(_sigid, qi::AnyFunction::fromDynamicFunction(boost::bind(pysignalCb, _1, callable)));
        GILScopedLock _lock;
        if (_async)
        {
          return boost::python::object(toPyFuture(f));
        }

        return boost::python::object(f.value());
      }

      boost::python::object disconnect(qi::uint64_t id, bool _async = false) {
        GILScopedUnlock _unlock;
        qi::FutureSync<void> f = _obj->disconnect(id);
        GILScopedLock _lock;
        if (_async)
        {
          return boost::python::object(toPyFuture(f));
        }

        return boost::python::object(f.value());
      }

      //this function is named trigger in the qi.Signal object,
      //the python wrapper add a __call__ method bound to this one. (see qi/__init__.py)
      void trig(boost::python::tuple args, boost::python::dict kwargs) {
        GILScopedUnlock _unlock;
        _obj->metaPost(_sigid, qi::AnyReference(args).asDynamic().asTupleValuePtr());
      }

    private:
      qi::AnyObject _obj;
      unsigned int  _sigid;
    };

    qi::SignalBase *getSignal(boost::python::object obj) {
      PySignal* sig = boost::python::extract<PySignal*>(obj);
      if (!sig)
        return 0;
      return sig->_sig;
    }

    boost::python::object makePySignal(const std::string &signature) {
      GILScopedLock _lock;
      return boost::python::object(PySignal(signature));
    }

    boost::python::object makePyProxySignal(const qi::AnyObject &obj, const qi::MetaSignal &signal) {
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
          .def("connect", &PySignal::connect, (boost::python::arg("callback"), boost::python::arg("_async") = false))
          .def("disconnect", &PySignal::disconnect, (boost::python::arg("id"), boost::python::arg("_async") = false))
          .def("disconnect_all", &PySignal::disconnectAll, (boost::python::arg("_async") = false))
          .def("__call__", boost::python::raw_function(&signal_param_shrinker<PySignal>));

      boost::python::class_<PyProxySignal>("_ProxySignal", boost::python::no_init)
          .def("connect", &PyProxySignal::connect, (boost::python::arg("callback"), boost::python::arg("_async") = false))
          .def("disconnect", &PyProxySignal::disconnect, (boost::python::arg("id"), boost::python::arg("_async") = false))
          .def("__call__", boost::python::raw_function(&signal_param_shrinker<PyProxySignal>));

    }

  }
}
