/*
**  Copyright (C) 2013 Aldebaran Robotics
**  See COPYING for the license
*/
#include "pysession.hpp"
#include <qimessaging/session.hpp>
#include <boost/python.hpp>
#include <boost/python/stl_iterator.hpp>
#include <qitype/dynamicobjectbuilder.hpp>
#include "pyfuture.hpp"
#include "gil.hpp"
#include "pysignal.hpp"
#include "pyobject.hpp"

qiLogCategory("qi.py");

namespace qi { namespace py {


qi::AnyReference triggerBouncer(qi::SignalBase *sig, const std::vector<qi::AnyReference>& args) {
  sig->trigger(args);
  return qi::AnyReference();
}

    class PySession {
    public:
      PySession()
        : _ses(new qi::Session)
        , nSigConnected(0)
        , nSigDisconnected(0)
        , connected(makePySignal())
        , disconnected(makePySignal())
      {
        // Get SignalBase from our PySignals
        qi::SignalBase *conn = getSignal(connected);
        qi::SignalBase *disconn = getSignal(disconnected);
        // Connect our PySignals with qi::Session signals, have to use a dynamic generic function to trigger
        nSigConnected = _ses->connected.connect(qi::AnyFunction::fromDynamicFunction(boost::bind(&triggerBouncer, conn, _1)));
        nSigDisconnected = _ses->disconnected.connect(qi::AnyFunction::fromDynamicFunction(boost::bind(&triggerBouncer, disconn, _1)));
      }

      ~PySession() {
        {
          GILScopedUnlock _unlock;
          _ses->connected.disconnect(nSigConnected);
          _ses->disconnected.disconnect(nSigDisconnected);
          _ses.reset();
        }
      }


      //return a future, or None (and throw in case of error)
      boost::python::object connect(const std::string &url, bool _async=false) {
        qi::Future<void> fut;
        {
          GILScopedUnlock _unlock;
          fut = _ses->connect(url);
        }
        return toPyFutureAsync(fut, _async);
      }

      //return a future, or None (and throw in case of error)
      boost::python::object close(bool _async=false) {
        qi::Future<void> fut;
        {
          GILScopedUnlock _unlock;
          fut = _ses->close();
        }
        return toPyFutureAsync(fut, _async);
      }

      boost::python::object service(const std::string &name, bool _async=false) {
        qi::Future<qi::AnyObject> fut;
        {
          GILScopedUnlock _unlock;
          fut = _ses->service(name);
        }
        return toPyFutureAsync(fut, _async);
      }

      boost::python::object services(bool _async=false) {
        qi::Future< std::vector<ServiceInfo> > fut;
        {
          GILScopedUnlock _unlock;
          fut = _ses->services();
        }
        return toPyFutureAsync(fut, _async);
      }

      boost::python::object registerService(const std::string &name, boost::python::object obj, bool _async=false) {
        qi::py::LeakBlock block;
        qi::AnyObject anyobj = qi::AnyReference(obj).toObject();
        qi::Future<unsigned int> fut;
        {
          GILScopedUnlock _unlock;
          fut = _ses->registerService(name, anyobj);
        }
        return toPyFutureAsync(fut, _async);
      }

      boost::python::object unregisterService(const unsigned int &id, bool _async=false) {
        qi::Future<void> fut;
        {
          GILScopedUnlock _unlock;
          fut = _ses->unregisterService(id);
        }
        return toPyFutureAsync(fut, _async);
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
          .def("connect", &PySession::connect, (boost::python::arg("url"), boost::python::arg("_async") = false),
               "connect(url) -> None\n"
               "Connect the session to a ServiceDirectory")

          .def("close", &PySession::close, (boost::python::arg("_async") = false),
               "close() -> None\n"
               "Close the Session")

          //TODO: endpoints()
          //TODO: listen(url)

          .def("service", &PySession::service, (boost::python::arg("service"), boost::python::arg("_async") = false),
               "service(name) -> Object\n"
               "return an Object representing a Service, the service could have been registered to the service directory "
               "by this session or by another.")

          .def("services", &PySession::services, (boost::python::arg("_async") = false),
               "services() -> list\n"
               "return the list of all services registered on the ServiceDirectory")

          .def("registerService", &PySession::registerService, (boost::python::arg("name"), boost::python::arg("object"), boost::python::arg("_async") = false),
               "registerService(name, object) -> int\n"
               "Register an Object as a service on the ServiceDirectory, the name should be unique on the servicedirectory."
               "This function returns the id associated to the service in the ServiceDirectory, the id can be used to unregister the service.")

          .def("unregisterService", &PySession::unregisterService, (boost::python::arg("id"), boost::python::arg("_async") = false),
               "unregisterService(id) -> None\n"
               "Unregister a service. The service should be owned by the current session. Use the Id returned by registerService.")

          .def_readonly("connected", &PySession::connected)

          .def_readonly("disconnected", &PySession::disconnected)

          //todo: serviceRegistered
          //todo: serviceUnregistered
          ;
    }

  }
}
