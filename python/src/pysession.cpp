/*
**  Copyright (C) 2013 Aldebaran Robotics
**  See COPYING for the license
*/
#include <qipython/pysession.hpp>
#include <boost/python/stl_iterator.hpp>
#include <qi/type/dynamicobjectbuilder.hpp>
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
          fut = _ses->connect(qi::Url(url, "tcp", 9559));
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

      boost::python::str url() {
        return boost::python::str(_ses->url().str());
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
               ":param url: string. Address of the ServiceDirectory of the form 'tcp://IP:PORT'.\n"
               ":raise: a RuntimeError if the connection failed.\n"
               "\n"
               "Connect the session to a ServiceDirectory.")

          .def("close", &PySession::close, (boost::python::arg("_async") = false),
               "close() -> None\n"
               "Close the Session.")

          .def("listen", &PySession::listen, (boost::python::arg("url"), boost::python::arg("_async") = false),
               "listen(url) -> None\n"
               ":param url: string. Address of the form 'tcp://IP:PORT'.\n"
               ":raise: a RuntimeError if the listening failed.\n"
               "\n"
               "Listen for connections on that specific Url.")

          .def("listenStandalone", &PySession::listenStandalone, (boost::python::arg("url"), boost::python::arg("_async") = false),
               "listenStandalone(url) -> None\n"
               ":param url: string. Address of the form 'tcp://IP:PORT'.\n"
               ":raise: a RuntimeError if the listening failed.\n"
               "\n"
               "Create a session with a standalone ServiceDirectory that will listen for connections on the specified Url.")

          .def("endpoints", &PySession::endpoints,
               "endpoints() -> list\n"
               ":return: the current list of endpoints of the session.\n")

          .def("service", &PySession::service, (boost::python::arg("service"), boost::python::arg("_async") = false),
               "service(name) -> Object\n"
               ":param name: string. The human readable name of the service we want to get.\n"
               ":return: an Object representing a Service. The service could have been registered to the ServiceDirectory by this session or by another.\n"
               ":raise: a RuntimeError if the service is not found.\n"
               "\n")

          .def("services", &PySession::services, (boost::python::arg("_async") = false),
               "services() -> list\n"
               ":return: the list of all services registered on the ServiceDirectory.\n")

          .def("registerService", &PySession::registerService, (boost::python::arg("name"), boost::python::arg("object"), boost::python::arg("_async") = false),
               "registerService(name, object) -> int\n"
               ":param name: string. A human readable name associated to the service.\n"
               ":param object: a python Object.\n"
               ":return: the id associated to the service in the ServiceDirectory. This id can be used to unregister the service.\n"
               ":raise: a RuntimeError if the service is already registered.\n"
               "\n"
               "Register an Object as a service on the ServiceDirectory. The name should be unique on the ServiceDirectory."
              )

          .def("unregisterService", &PySession::unregisterService, (boost::python::arg("id"), boost::python::arg("_async") = false),
               "unregisterService(id) -> None\n"
               ":param id: int. The id associated to the service when it was registered.\n"
               ":raise: a RuntimeError if the service is not found.\n"
               "\n"
               "Unregister a service. The service should be owned by the current session. Use the Id returned by registerService.")

          .def("url", &PySession::url,
               "url() -> str\n"
               ":return: the url the session is connected to\n"
               ":raise: a RuntimeError if the session is not connected\n"
               )

          .def_readonly("connected", &PySession::connected)

          .def_readonly("disconnected", &PySession::disconnected)

          .def_readonly("serviceRegistered", &PySession::serviceRegistered)

          .def_readonly("serviceUnregistered", &PySession::serviceUnregistered)
          ;
    }

    boost::python::object makePySession(const qi::SessionPtr& ses)
    {
      return boost::python::object(PySession(ses));
    }

  }
}
