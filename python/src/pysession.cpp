/*
**
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2013 Aldebaran Robotics
*/
#include "pysession.hpp"
#include <qimessaging/session.hpp>
#include <boost/python.hpp>
#include <boost/python/stl_iterator.hpp>
#include <qitype/genericobjectbuilder.hpp>
#include "pyfuture.hpp"
#include "gil.hpp"
#include "pysignal.hpp"

qiLogCategory("qi.py");

namespace qi { namespace py {


qi::GenericValuePtr triggerBouncer(qi::SignalBase *sig, const std::vector<qi::GenericValuePtr>& args) {
  sig->trigger(args);
  return qi::GenericValuePtr();
}

    class PySession {
    public:
      PySession()
        : _ses(new qi::Session)
        , connected(makePySignal("[m]"))
        , disconnected(makePySignal("[m]"))
        , nSigConnected(0)
        , nSigDisconnected(0)
      {
        // Get SignalBase from our PySignals
        qi::SignalBase *conn = getSignal(connected);
        qi::SignalBase *disconn = getSignal(disconnected);
        // Connect our PySignals with qi::Session signals, have to use a dynamic generic function to trigger
        nSigConnected = _ses->connected.connect(qi::makeDynamicGenericFunction(boost::bind(&triggerBouncer, conn, _1)));
        nSigDisconnected = _ses->disconnected.connect(qi::makeDynamicGenericFunction(boost::bind(&triggerBouncer, disconn, _1)));
      }

      ~PySession() {
        _ses->connected.disconnect(nSigConnected);
        _ses->disconnected.disconnect(nSigDisconnected);
      }

      //return a future, or None (and throw in case of error)
      boost::python::object connect(const std::string &url, bool _async=false) {
        GILScopedUnlock _unlock;
        if (_async)
          return boost::python::object(toPyFuture(_ses->connect(url)));
        else {
          qi::Future<void> fut = _ses->connect(url);
          {
            fut.value(); //throw on error
          }
          return boost::python::object();
        }
      }

      //return a future, or None (and throw in case of error)
      boost::python::object close(bool _async=false) {
        GILScopedUnlock _unlock;
        if (_async)
          return boost::python::object(toPyFuture(_ses->close()));
        else {
          qi::Future<void> fut = _ses->close();
          {
            fut.value(); //throw on error
          }
          return boost::python::object();
        }
      }

      boost::python::object service(const std::string &name, bool _async=false) {
        GILScopedUnlock _unlock;
        if (_async)
          return boost::python::object(toPyFuture(_ses->service(name)));
        else {
          qi::Future<qi::ObjectPtr>  fut = _ses->service(name);
          qi::ObjectPtr obj;
          {
            obj = fut.value(); //throw on error.
          }
          qi::GenericValueRef r(obj);
          return r.to<boost::python::object>(); //throw on error
        }
      }

      boost::python::object services(bool _async=false) {
        GILScopedUnlock _unlock;
        if (_async)
          return boost::python::object(toPyFuture(_ses->services()));
        else {
          qi::Future< std::vector<ServiceInfo> >  fut = _ses->services();
          std::vector<ServiceInfo> si;
          {
            si = fut.value(); //throw on error.
          }
          qi::GenericValueRef r(si);
          return r.to<boost::python::object>(); //throw on error
        }
      }

      boost::python::object registerService(const std::string &name, boost::python::object obj, bool _async=false) {
        GILScopedUnlock _unlock;
        qi::Future<unsigned int> fut = _ses->registerService(name, qi::GenericValueRef(obj).toObject());
        if (_async)
          return boost::python::object(toPyFuture(fut));
        {
          fut.value();
        }
        return boost::python::object(fut.value()); //throw on error
      }

    private:
      boost::shared_ptr<qi::Session> _ses;
      int nSigConnected;
      int nSigDisconnected;

    public:
      boost::python::object connected;
      boost::python::object disconnected;
    };

    void export_pysession() {
      boost::python::class_<PySession>("Session")
          .def("connect", &PySession::connect, (boost::python::arg("url"), boost::python::arg("_async") = false))
          .def("close", &PySession::close, (boost::python::arg("_async") = false))
          .def("service", &PySession::service, (boost::python::arg("service"), boost::python::arg("_async") = false))
          .def("services", &PySession::services, (boost::python::arg("_async") = false))
          .def("register_service", &PySession::registerService, (boost::python::arg("name"), boost::python::arg("object"), boost::python::arg("_async") = false))
          .def_readonly("connected", &PySession::connected)
          .def_readonly("disconnected", &PySession::disconnected)
          ;
    }

  }
}
