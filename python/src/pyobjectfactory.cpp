/**
* @author Aldebaran Robotics
* Aldebaran Robotics (c) 2012 All Rights Reserved
*/

#include <boost/python.hpp>
#include <map>

#include <qitype/genericobject.hpp>
#include <qitype/objectfactory.hpp>
#include "pyobjectfactory.hpp"
#include "pyobject.hpp"
#include "error.hpp"

qiLogCategory("qipy.factory");

namespace qi {
  namespace py {
    class PyCreateException : std::exception
    {
    public:
      PyCreateException(std::string &name) : _name(name) {}
      virtual ~PyCreateException() throw() {}
      char const * what() const throw() { return std::string(_name + " not found").c_str(); }

    private:
      std::string _name;
    };

    void translate_pycreateexception(PyCreateException const& e)
    {
      PyErr_SetString(PyExc_RuntimeError, e.what());
    }

    static boost::python::object pycreate_object(boost::python::str name)
    {
      std::string objectName = boost::python::extract<std::string>(name);
      qi::AnyObject object = qi::createObject(objectName);

      if(!object)
        throw PyCreateException(objectName);

      return makePyQiObject(object, objectName);
    }

    static qi::AnyObject pyconstruct_object(boost::python::object class_)
    {
      qi::AnyObject obj;
      PY_CATCH_ERROR(obj = makeQiAnyObject(class_()));
      return obj;
    }

    static void pyregister_object_factory(boost::python::str class_name, boost::python::object class_)
    {
      std::string name = boost::python::extract<std::string>(class_name);
      boost::function<qi::AnyObject (const std::string&)> func = boost::bind(&qi::py::pyconstruct_object, class_);
      qi::registerObjectFactory(name, func);
    }

    void export_pyobjectfactory()
    {
      boost::python::register_exception_translator<PyCreateException>(&translate_pycreateexception);
      boost::python::def("createObject", &qi::py::pycreate_object);
      boost::python::def("registerObjectFactory", &qi::py::pyregister_object_factory);
    }
  }
}
