/*
**  Copyright (C) 2013 Aldebaran Robotics
**  See COPYING for the license
*/
#include <boost/python.hpp>
#include <map>

#include <qi/anyobject.hpp>
#include <qi/type/objectfactory.hpp>
#include <qipython/gil.hpp>
#include <qipython/pyobjectfactory.hpp>
#include <qipython/pyobject.hpp>
#include <qipython/error.hpp>
#include <boost/python/raw_function.hpp>

qiLogCategory("qipy.factory");

namespace qi {
  namespace py {
    class PyCreateException : std::exception
    {
    public:
      PyCreateException(const std::string &name) : _msg(name + "not found") {}
      virtual ~PyCreateException() throw() {}
      char const * what() const throw() { return _msg.c_str(); }

    private:
      std::string _msg;
    };

    void translate_pycreateexception(PyCreateException const& e)
    {
      PyErr_SetString(PyExc_RuntimeError, e.what());
    }

    static qi::AnyObject pyconstruct_object(boost::python::object class_)
    {
      GILScopedLock lock;
      qi::AnyObject obj;
      PY_CATCH_ERROR(obj = makeQiAnyObject(class_()));
      return obj;
    }

    static void pyregister_object_factory(boost::python::str class_name, boost::python::object class_)
    {
      std::string name = boost::python::extract<std::string>(class_name);
      boost::function<qi::AnyObject()> func = boost::bind(&qi::py::pyconstruct_object, class_);
      qi::registerObjectFactory(name, AnyFunction::from(func));
    }

    static boost::python::object pycreate_object_args(boost::python::tuple pyargs,
                                                      boost::python::dict kwargs)
    {
      int           len        = boost::python::len(pyargs);
      std::string   objectName = boost::python::extract<std::string>(pyargs[0]);
      qi::AnyObject object;

      qi::AnyReferenceVector args;
      for (int i = 1; i < len; ++i)
        args.push_back(AnyReference::from(boost::python::object(pyargs[i])));
      object = qi::createObject(objectName, args);

      if(!object)
        throw PyCreateException(objectName);
      return makePyQiObject(object, objectName);
    }

    void export_pyobjectfactory()
    {
      boost::python::register_exception_translator<PyCreateException>(&translate_pycreateexception);
      boost::python::def("createObject", boost::python::raw_function(&pycreate_object_args, 1));
      boost::python::def("registerObjectFactory", &qi::py::pyregister_object_factory);
    }
  }
}
