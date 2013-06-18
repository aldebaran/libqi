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
#include <qitype/genericobject.hpp>
namespace qi { namespace py {

    class PyProperty : public qi::GenericProperty {
    public:
      PyProperty(const std::string &signature)
        : qi::GenericProperty(qi::TypeInterface::fromSignature(signature))
      {
      }

      ~PyProperty() {
      }

      boost::python::object val() const {
        return value().to<boost::python::object>();
      }

      //change the name to avoid warning "hidden overload in base class" : YES WE KNOW :)
      void setVal(boost::python::object obj) {
        qi::GenericProperty::setValue(qi::GenericValueRef(obj));
      }
    };

    class PyProxyProperty {
    public:
      PyProxyProperty(qi::ObjectPtr obj, const qi::MetaProperty &signal)
        : _obj(obj)
        , _sigid(signal.uid()){
      }

      //TODO: support async
      boost::python::object value() const {
        return _obj->property(_sigid).value().to<boost::python::object>();
      }

      //TODO: support async
      void setValue(boost::python::object obj) {
        _obj->setProperty(_sigid, qi::GenericValue::from(obj));
      }

    private:
      qi::ObjectPtr _obj;
      unsigned int  _sigid;
    };

    boost::python::object makePyProperty(const std::string &signature) {
      return boost::python::object(PyProperty(signature));
    }

    qi::PropertyBase *getProperty(boost::python::object obj) {
      return boost::python::extract<PyProperty*>(obj);
    }

    boost::python::object makePyProxyProperty(const qi::ObjectPtr &obj, const qi::MetaProperty &prop) {
      return boost::python::object(PyProxyProperty(obj, prop));
    }

    void export_pyproperty() {
      boost::python::class_<PyProperty>("Property", boost::python::init<const std::string &>())
          .def("value", &PyProperty::val)
          .def("setValue", &PyProperty::setVal, (boost::python::arg("value")));

      boost::python::class_<PyProxyProperty>("_ProxyProperty", boost::python::no_init)
          .def("value", &PyProxyProperty::value)
          .def("setValue", &PyProxyProperty::setValue, (boost::python::arg("value")));
    }

  }
}
