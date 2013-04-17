/*
**
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2013 Aldebaran Robotics
*/
#include "pyobject.hpp"
#include <boost/python.hpp>
#include <boost/python/raw_function.hpp>
#include "pyfuture.hpp"

qiLogCategory("qipy.object");

namespace qi { namespace py {


    struct PyQiFunctor {
    public:
      PyQiFunctor(const std::string &funName, qi::ObjectPtr obj)
        : _object(obj)
        , _funName(funName)
      {}

      boost::python::object operator()(boost::python::tuple pyargs, boost::python::dict pykwargs) {
        qiLogInfo("qipy") << "calling a method!";
        qi::GenericValue val = qi::GenericValue::from(pyargs);
        //TODO: do we need async?
        pykwargs.has_key("_async");
        bool async = boost::python::extract<bool>(pykwargs.get("_async", false));
        std::string funN = boost::python::extract<std::string>(pykwargs.get("_overload", _funName));

        qiLogInfo("qipy") << "calling a method1!" << qi::encodeJSON(val);
        qi::Future<qi::GenericValuePtr> fut = _object->metaCall(funN, val.asDynamic().asTupleValuePtr());
        //_object->
        qiLogInfo("qipy") << "calling a method2: ret:" << qi::encodeJSON(fut.value());
        if (!async)
          return fut.value().to<boost::python::object>();
        else
          return qi::py::makeFuture(fut);
      }

    public:
      qi::ObjectPtr _object;
      std::string   _funName;
    };



    void addMethod(boost::python::object pyobj, qi::ObjectPtr obj, const qi::MetaMethod &mem) {
      qiLogInfo() << "adding method:" << mem.signature();
      std::vector<std::string> vs = qi::signatureSplit(mem.signature());
      boost::python::object fun = boost::python::raw_function(PyQiFunctor(vs[1].c_str(), obj));
      boost::python::api::setattr(pyobj, vs[1].c_str(), fun);// result
    }

    void populateMethods(boost::python::object pyobj, qi::ObjectPtr obj) {
      qi::MetaObject::MethodMap           mm = obj->metaObject().methodMap();
      qi::MetaObject::MethodMap::iterator it;
      for (it = mm.begin(); it != mm.end(); ++it) {
        qi::MetaMethod &mem = it->second;
        //drop special methods
        if (mem.uid() < 10)
          continue;
        std::vector<std::string> vs = qi::signatureSplit(mem.signature());
        addMethod(pyobj, obj, mem);
      }
    }


    class PyQiObject {
    public:
      PyQiObject()
      {}

      PyQiObject(const qi::ObjectPtr &obj)
        : _object(obj)
      {}

      boost::python::object call(boost::python::str pyname, boost::python::tuple pyargs, boost::python::dict pykws) {
        return PyQiFunctor(boost::python::extract<std::string>(pyname), _object)(pyargs, pykws);
      }

      boost::python::object metaObject() {
        return qi::GenericValueRef(_object->metaObject()).to<boost::python::object>();
      }

    public:
      qi::ObjectPtr _object;
    };

    boost::python::object makePyQiObject(qi::ObjectPtr obj, const std::string &name) {
      boost::python::object result = boost::python::object(qi::py::PyQiObject(obj));
      qi::py::populateMethods(result, obj);
      return result;
    }

    void export_pyobject() {
      boost::python::class_<qi::py::PyQiObject>("Object")
          .def("call", &qi::py::PyQiObject::call, (boost::python::arg("name"), boost::python::arg("args") = boost::python::tuple(), boost::python::arg("kargs") = boost::python::dict()))
          .def("metaObject", &qi::py::PyQiObject::metaObject);
    }

  }
}
