/*
**  Copyright (C) 2014 Aldebaran Robotics
**  See COPYING for the license
*/

#include "pypath.hpp"

#include <boost/python.hpp>
#include <qi/path.hpp>
#include <qitype/anyvalue.hpp>

namespace qi {
  namespace py {
    static boost::python::list pylistdata1(const std::string& applicationName, const std::string& pattern)
    {
      std::vector<std::string> vect = qi::path::listData(applicationName, pattern);
      return qi::AnyReference::from(vect).to<boost::python::list>();
    }

    // With default pattern
    static boost::python::list pylistdata2(const std::string& applicationName)
    {
      std::vector<std::string> vect = qi::path::listData(applicationName);
      return qi::AnyReference::from(vect).to<boost::python::list>();
    }

    static boost::python::list pyconfpaths1(const std::string& applicationName)
    {
      std::vector<std::string> vect = qi::path::confPaths(applicationName);
      return qi::AnyReference::from(vect).to<boost::python::list>();
    }

    static boost::python::list pyconfpaths2()
    {
      std::vector<std::string> vect = qi::path::confPaths();
      return qi::AnyReference::from(vect).to<boost::python::list>();
    }

    static boost::python::list pydatapaths1(const std::string& applicationName)
    {
      std::vector<std::string> vect = qi::path::dataPaths(applicationName);
      return qi::AnyReference::from(vect).to<boost::python::list>();
    }

    static boost::python::list pydatapaths2()
    {
      std::vector<std::string> vect = qi::path::dataPaths();
      return qi::AnyReference::from(vect).to<boost::python::list>();
    }

    static boost::python::list pybinpaths()
    {
      std::vector<std::string> vect = qi::path::binPaths();
      return qi::AnyReference::from(vect).to<boost::python::list>();
    }

    static boost::python::list pylibpaths()
    {
      std::vector<std::string> vect = qi::path::libPaths();
      return qi::AnyReference::from(vect).to<boost::python::list>();
    }

    void export_pypath()
    {
      boost::python::def("sdkPrefix", &qi::path::sdkPrefix);
      boost::python::def("findBin", &qi::path::findBin);
      boost::python::def("findLib", &qi::path::findLib);
      boost::python::def("findConf", &qi::path::findConf);
      boost::python::def("findData", &qi::path::findData);
      boost::python::def("listData", &pylistdata1);
      boost::python::def("listData", &pylistdata2);
      boost::python::def("confPaths", &pyconfpaths1);
      boost::python::def("confPaths", &pyconfpaths2);
      boost::python::def("dataPaths", &pydatapaths1);
      boost::python::def("dataPaths", &pydatapaths2);
      boost::python::def("binPaths", &pybinpaths);
      boost::python::def("libPaths", &pylibpaths);
      boost::python::def("setWritablePath", &qi::path::setWritablePath);
      boost::python::def("userWritableDataPath", &qi::path::userWritableDataPath);
      boost::python::def("userWritableConfPath", &qi::path::userWritableConfPath);
    }
  }
}
