/*
**  Copyright (C) 2013 Aldebaran Robotics
**  See COPYING for the license
*/
#include <qipython/pysession.hpp>
#include <boost/python/stl_iterator.hpp>
#include <qitype/dynamicobjectbuilder.hpp>
#include <qipython/gil.hpp>
#include <qipython/pyfuture.hpp>
#include <qipython/pysignal.hpp>
#include <qipython/pyobject.hpp>

qiLogCategory("qi.py");

namespace qi { namespace py {


    qi::AnyReference triggerBouncer(qi::SignalBase *sig, const std::vector<qi::AnyReference>& args) {
      sig->trigger(args);
      return qi::AnyReference();
    }

    static void doNothingSB(SignalBase*) {}

    class PySession {
    public:
      PySession()
        : _ses(new qi::Session())
      {
        setupSignal();
      }

      explicit PySession(const std::string& url)
        : _ses(new qi::Session())
      {
        setupSignal();
        //throw on error
        connect(url);
      }
      explicit PySession(const qi::SessionPtr& session)
        : _ses(session)
      {
        setupSignal();
      }

      void setupSignal() {
        //we dont want to loose destroy the signal, they are owned by _ses
        boost::shared_ptr<SignalBase> sbc(&_ses->connected, &doNothingSB);
        boost::shared_ptr<SignalBase> sbd(&_ses->disconnected, &doNothingSB);
        boost::shared_ptr<SignalBase> sbr(&_ses->serviceRegistered, &doNothingSB);
        boost::shared_ptr<SignalBase> sbu(&_ses->serviceUnregistered, &doNothingSB);
        connected = makePySignalFromBase(sbc);
        disconnected = makePySignalFromBase(sbd);
        serviceRegistered = makePySignalFromBase(sbr);
        serviceUnregistered = makePySignalFromBase(sbu);
      }

      ~PySession() {
        {
          GILScopedUnlock _unlock;
          //do not disconnect signal here, cause PySession is just a wrapper around a Session.
          //if we own the session then destructing the session will unregister signal,
          //otherwize they will be available until the real session is shutdown. (think about ApplicationSession)

          //reset the session (the dtor can block)
          _ses = qi::SessionPtr();
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

      boost::python::object listen(const std::string &url, bool _async=false) {
        qi::Future<void> fut;
        {
          GILScopedUnlock _unlock;
          fut = _ses->listen(url);
        }
        return toPyFutureAsync(fut, _async);
      }

      boost::python::object listenStandalone(const std::string &url, bool _async=false) {
        qi::Future<void> fut;
        {
          GILScopedUnlock _unlock;
          fut = _ses->listenStandalone(url);
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
        qi::AnyObject anyobj = qi::AnyReference::from(obj).toObject();
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

      boost::python::object endpoints() {
        boost::python::list ret;
        std::vector<qi::Url>  eps = _ses->endpoints();
        for (unsigned int i = 0; i < eps.size(); ++i) {
          ret.append(eps.at(i).str());
        }
        return ret;
      }

    private:
      qi::SessionPtr _ses;

    public:
      boost::python::object connected;
      boost::python::object disconnected;
      boost::python::object serviceRegistered;
      boost::python::object serviceUnregistered;
    };

    void export_pysession() {
      boost::python::class_<PySession>("Session", boost::python::init<>())
          .def(boost::python::init<const std::string&>())
          .def("connect", &PySession::connect, (boost::python::arg("url"), boost::python::arg("_async") = false),
               "connect(url) -> None\n"
               "Connect the session to a ServiceDirectory")

          .def("close", &PySession::close, (boost::python::arg("_async") = false),
               "close() -> None\n"
               "Close the Session")

          .def("listen", &PySession::listen, (boost::python::arg("url"), boost::python::arg("_async") = false),
               "listen(url) -> None\n"
               "Listen on that specific Url")

          .def("listenStandalone", &PySession::listenStandalone, (boost::python::arg("url"), boost::python::arg("_async") = false),
               "listenStandalone(url) -> None\n"
               "Create a session with a standalone ServiceDirectory")

          .def("endpoints", &PySession::endpoints,
               "endpoints() -> list\n"
               "Return the current list of endpoints of the session")

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

          .def_readonly("serviceRegistered", &PySession::serviceRegistered)

          .def_readonly("serviceUnregistered", &PySession::serviceUnregistered)
          ;
    }

    boost::python::object makePySession(const qi::SessionPtr& ses)
    {
      GILScopedLock _lock;
      return boost::python::object(PySession(ses));
    }

  }
}
