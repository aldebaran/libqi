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

    static qi::AnyReference pysignalCb(const std::vector<qi::AnyReference>& cargs, const PyThreadSafeObject& callable) {
      GILScopedLock _lock;
      boost::python::list   args;
      boost::python::object ret;

      std::vector<qi::AnyReference>::const_iterator it;
      for (it = cargs.begin(); it != cargs.end(); ++it) {
        args.append(it->to<boost::python::object>());
      }
      PY_CATCH_ERROR(ret = callable.object()(*boost::python::tuple(args)));
      return qi::AnyReference::from(ret).clone();
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
        PyThreadSafeObject obj(callable);

        if (!PyCallable_Check(callable.ptr()))
          throw std::runtime_error("Not a callable");
        qi::uint64_t r;
        {
          GILScopedUnlock _unlock;
          //no need to store a ptr on ourself. (this exist if the callback is triggered)
          r = _sig->connect(qi::AnyFunction::fromDynamicFunction(boost::bind(pysignalCb, _1, obj)));
        }
        if (_async)
        {
          return boost::python::object(toPyFuture(qi::Future<qi::uint64_t>(r)));
        }
        return boost::python::object(r);
      }

      boost::python::object disconnect(qi::uint64_t id, bool _async = false) {
        bool r;
        {
          GILScopedUnlock _unlock;
          r = _sig->disconnect(id);
        }
        if (_async)
        {
          return boost::python::object(toPyFuture(qi::Future<bool>(r)));
        }

        return boost::python::object(r);
      }

      boost::python::object disconnectAll(bool _async = false) {
        bool r;
        {
          GILScopedUnlock _unlock;
          r = _sig->disconnectAll();
        }
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
        _sig->trigger(qi::AnyReference::from(args).content().asTupleValuePtr());
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
        PyThreadSafeObject obj(callable);
        if (!PyCallable_Check(callable.ptr()))
          throw std::runtime_error("Not a callable");
        qi::Future<SignalLink> f;
        {
          GILScopedUnlock _unlock;
          //no need to store a ptr on ourself. (this exist if the callback is triggered)
          f = _obj.connect(_sigid, qi::AnyFunction::fromDynamicFunction(boost::bind(pysignalCb, _1, obj)));
        }
        return toPyFutureAsync(f, _async);
      }

      boost::python::object disconnect(qi::uint64_t id, bool _async = false) {
        qi::Future<void> f;
        {
          GILScopedUnlock _unlock;
          f = _obj.disconnect(id);
        }
        return toPyFutureAsync(f, _async);
      }

      //this function is named trigger in the qi.Signal object,
      //the python wrapper add a __call__ method bound to this one. (see qi/__init__.py)
      void trig(boost::python::tuple args, boost::python::dict kwargs) {
        GILScopedUnlock _unlock;
        _obj.metaPost(_sigid, qi::AnyReference::from(args).content().asTupleValuePtr());
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
          .def("connect", &PySignal::connect, (boost::python::arg("callback"), boost::python::arg("_async") = false),
               "connect(callback) -> int\n"
               ":param callback: the callback that will be called when the signal is triggered\n"
               ":return: the connection id of the registered callback.\n"
               "\n"
               "Connect the signal to a callback, the callback will be called each time the signal is triggered. "
               "Use the id returned to unregister the callback")

          .def("disconnect", &PySignal::disconnect, (boost::python::arg("id"), boost::python::arg("_async") = false),
               "disconnect(id) -> bool\n"
               ":param id: the connection id returned by connect\n"
               ":return: true on success\n"
               "\n"
               "Disconnect the callback associated to id.")

          .def("disconnectAll", &PySignal::disconnectAll, (boost::python::arg("_async") = false),
               "disconnectAll() -> bool\n"
               ":return: true on success\n"
               "\n"
               "disconnect all callback associated to the signal. This function should be used very carefully. It's extremely rare that it is needed.")

          .def("__call__", boost::python::raw_function(&signal_param_shrinker<PySignal>),
               "__call__(*args) -> None\n"
               "Trigger the signal");

      boost::python::class_<PyProxySignal>("_ProxySignal", boost::python::no_init)
          .def("connect", &PyProxySignal::connect, (boost::python::arg("callback"), boost::python::arg("_async") = false))
          .def("disconnect", &PyProxySignal::disconnect, (boost::python::arg("id"), boost::python::arg("_async") = false))
          .def("__call__", boost::python::raw_function(&signal_param_shrinker<PyProxySignal>));
    }

  }
}
