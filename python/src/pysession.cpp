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

    template <typename T>
    PyFuture toPyFuture(qi::Future<T> fut) {
      qi::Promise<qi::GenericValue> gprom;
      qi::adaptFuture(fut, gprom);
      return gprom.future();
    }

    template <typename T>
    PyFuture toPyFuture(qi::FutureSync<T> fut) {
      qi::Promise<qi::GenericValue> gprom;
      qi::adaptFuture(fut.async(), gprom);
      return gprom.future();
    }


    void addMethod(qi::GenericObjectBuilder& gob, boost::python::object meth) {

    }


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
          return boost::python::object(fut.value()); //throw on error
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
          .def("register_service", &PySession::registerService, (boost::python::arg("name"), boost::python::arg("object"), boost::python::arg("_async") = false))
          ;
    }

  }
}
