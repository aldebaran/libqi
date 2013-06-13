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
        , nSigConnected(0)
        , nSigDisconnected(0)
        , connected(makePySignal("[m]"))
        , disconnected(makePySignal("[m]"))
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
        return toPyFutureAsync(_ses->connect(url), _async);
      }

      //return a future, or None (and throw in case of error)
      boost::python::object close(bool _async=false) {
        GILScopedUnlock _unlock;
        return toPyFutureAsync(_ses->close(), _async);
      }

      boost::python::object service(const std::string &name, bool _async=false) {
        GILScopedUnlock _unlock;
        return toPyFutureAsync(_ses->service(name), _async);
      }

      boost::python::object services(bool _async=false) {
        GILScopedUnlock _unlock;
        return toPyFutureAsync(_ses->services(), _async);
      }

      boost::python::object registerService(const std::string &name, boost::python::object obj, bool _async=false) {
        GILScopedUnlock _unlock;
        return toPyFutureAsync(_ses->registerService(name, qi::GenericValueRef(obj).toObject()), _async);
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
