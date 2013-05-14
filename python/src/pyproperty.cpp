/*
**
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2013 Aldebaran Robotics
*/
#include "pyproperty.hpp"
#include <boost/python.hpp>
#include <qitype/property.hpp>

namespace qi { namespace py {

    class PyProperty : public qi::GenericProperty {
    public:
      PyProperty(const std::string &signature)
        : qi::GenericProperty(qi::Type::fromSignature(signature))
      {
      }

      ~PyProperty() {
      }

      boost::python::object value() const {
        return getValue().to<boost::python::object>();
      }

      //change the name to avoid warning "hidden overload in base class" : YES WE KNOW :)
      void setVal(boost::python::object obj) {
        qi::GenericProperty::setValue(qi::GenericValueRef(obj));
      }
    };

    boost::python::object makePyProperty(const std::string &signature) {
      return boost::python::object(PyProperty(signature));
    }

    qi::PropertyBase *getProperty(boost::python::object obj) {
      return boost::python::extract<PyProperty*>(obj);
    }

    void export_pyproperty() {
      boost::python::class_<PyProperty>("Property", boost::python::init<const std::string &>())
          .def("value", &PyProperty::value)
          .def("setValue", &PyProperty::setVal, (boost::python::arg("value")));
    }

  }
}
