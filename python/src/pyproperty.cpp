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
      PyProperty()
        : qi::GenericProperty(0)
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

    boost::python::object makePyProperty() {
      boost::python::object ret;
      return ret;
    }

    void export_pyproperty() {
      boost::python::class_<PyProperty>("Property")
          .def("value", &PyProperty::value)
          .def("setValue", &PyProperty::setVal, (boost::python::arg("value")));
    }

  }
}
