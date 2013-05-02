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

qiLogCategory("qi.py");

namespace qi { namespace py {


    class PySession {
    public:
      PySession()
        : _ses(new qi::Session)
      {
      }

      ~PySession() {
      }

      //return a future, or None (and throw in case of error)
      boost::python::object connect(const std::string &url, bool _async=false) {
        if (_async)
          return boost::python::object(toPyFuture(_ses->connect(url)));
        else {
          qi::Future<void> fut = _ses->connect(url);
          fut.value(); //throw on error
          return boost::python::object();
        }
      }

      //return a future, or None (and throw in case of error)
      boost::python::object close(bool _async=false) {
        if (_async)
          return boost::python::object(toPyFuture(_ses->close()));
        else {
          qi::Future<void> fut = _ses->close();
          fut.value(); //throw on error
          return boost::python::object();
        }
      }

      boost::python::object service(const std::string &name, bool _async=false) {
        if (_async)
          return boost::python::object(toPyFuture(_ses->service(name)));
        else {
          qi::Future<qi::ObjectPtr>  fut = _ses->service(name);
          qi::ObjectPtr obj = fut.value(); //throw on error.
          qi::GenericValueRef r(obj);
          return r.to<boost::python::object>(); //throw on error
        }
      }

      boost::python::object services(bool _async=false) {
        if (_async)
          return boost::python::object(toPyFuture(_ses->services()));
        else {
          qi::Future< std::vector<ServiceInfo> >  fut = _ses->services();
          std::vector<ServiceInfo> si = fut.value(); //throw on error.
          qi::GenericValueRef r(si);
          return r.to<boost::python::object>(); //throw on error
        }
      }

      boost::python::object registerService(const std::string &name, boost::python::object obj, bool _async=false) {
        qi::Future<unsigned int> fut = _ses->registerService(name, qi::GenericValueRef(obj).toObject());
        if (_async)
          return boost::python::object(toPyFuture(fut));
        return boost::python::object(fut.value()); //throw on error
      }

    private:
      boost::shared_ptr<qi::Session> _ses;
    };

    void export_pysession() {
      boost::python::class_<PySession>("Session")
          .def("connect", &PySession::connect, (boost::python::arg("url"), boost::python::arg("_async") = false))
          .def("close", &PySession::close, (boost::python::arg("_async") = false))
          .def("service", &PySession::service, (boost::python::arg("service"), boost::python::arg("_async") = false))
          .def("services", &PySession::services, (boost::python::arg("_async") = false))
          .def("register_service", &PySession::registerService, (boost::python::arg("name"), boost::python::arg("object"), boost::python::arg("_async") = false))
          ;
    }

  }
}
