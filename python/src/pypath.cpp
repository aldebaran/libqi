/*
**  Copyright (C) 2014 Aldebaran Robotics
**  See COPYING for the license
*/

#include <qipython/pypath.hpp>

#include <boost/python.hpp>
#include <qi/path.hpp>
#include <qi/anyvalue.hpp>

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
      boost::python::def("sdkPrefix", &qi::path::sdkPrefix,
                         "sdkPrefix() -> string\n"
                         ":return: The SDK prefix path. It is always a complete, native path.\n");

      boost::python::def("findBin", &qi::path::findBin,
                         "findBin(name) -> string\n"
                         ":param name: string. The full name of the binary, or just the name.\n"
                         ":return: the complete, native path to the file found. An empty string otherwise.\n"
                         "\n"
                         "Look for a binary in the system.");

      boost::python::def("findLib", &qi::path::findLib,
                         "findLib(name) -> string\n"
                         ":param name: string. The full name of the library, or just the name.\n"
                         ":return: the complete, native path to the file found. An empty string otherwise.\n"
                         "\n"
                         "Look for a library in the system.");

      boost::python::def("findConf", &qi::path::findConf,
                         "findConf(application, file) -> string\n"
                         ":param application: string. The name of the application.\n"
                         ":param file: string. The name of the file to look for."
                         " You can specify subdirectories using '/' as a separator.\n"
                         ":return: the complete, native path to the file found. An empty string otherwise.\n"
                         "\n"
                         "Look for a configuration file in the system.");

      boost::python::def("findData", &qi::path::findData,
                         "findData(application, file) -> string\n"
                         ":param application: string. The name of the application.\n"
                         ":param file: string. The name of the file to look for."
                         " You can specify subdirectories using a '/' as a separator.\n"
                         ":return: the complete, native path to the file found. An empty string otherwise.\n"
                         "\n"
                         "Look for a file in all dataPaths(application) directories. Return the first match.");

      boost::python::def("listData", &pylistdata1,
                         "listData(application, pattern) -> [string]\n"
                         ":param application: string. The name of the application.\n"
                         ":param patten: string. Wildcard pattern of the files to look for."
                         " You can specify subdirectories using a '/' as a separator."
                         " * by default.\n"
                         ":return: a list of the complete, native paths of the files that matched.\n"
                         "\n"
                         "List data files matching the given pattern in all dataPaths(application) directories."
                         " For each match, return the occurence from the first dataPaths prefix."
                         " Directories are discarded.");
      boost::python::def("listData", &pylistdata2);

      boost::python::def("confPaths", &pyconfpaths1,
                         "confPaths(application) -> [string]\n"
                         ":param application: string. Name of the application. \"\" by default.\n"
                         ":return: The list of configuration directories.\n"
                         "\n"
                         "Get the list of directories used when searching for configuration files"
                         " for the given application.\n"
                         "\n"
                         ".. warning::\n"
                         "   You should not assume those directories exist,"
                         " nor that they are writable."
                         "\n");

      boost::python::def("confPaths", &pyconfpaths2);

      boost::python::def("dataPaths", &pydatapaths1,
                         "dataPaths(application) -> [string]\n"
                         ":param application: string. Name of the application. \"\" by default.\n"
                         ":return: The list of data directories.\n"
                         "\n"
                         "Get the list of directories used when searching for configuration files"
                         " for the given application.\n"
                         "\n"
                         ".. warning::\n"
                         "   You should not assume those directories exist,"
                         " nor that they are writable."
                         "\n");

      boost::python::def("dataPaths", &pydatapaths2);

      boost::python::def("binPaths", &pybinpaths,
                         "binPaths() -> [string]\n"
                         ":return: The list of directories used when searching for binaries.\n"
                         "\n"
                         ".. warning::\n"
                         "   You should not assume those directories exist,"
                         " nor that they are writable."
                         "\n");

      boost::python::def("libPaths", &pylibpaths,
                         "libPaths() -> [string]\n"
                         ":return: The list of directories used when searching for libraries.\n"
                         "\n"
                         ".. warning::\n"
                         "   You should not assume those directories exist,"
                         " nor that they are writable."
                         "\n");

      boost::python::def("setWritablePath", &qi::path::detail::setWritablePath,
                         "setWritablePath(path) -> None\n"
                         ":param path: string. A path on the system.\n"
                         "\n"
                         "Set the writable files path for users. "
                         "Use an empty path to reset it to its initial value.");

      boost::python::def("userWritableDataPath", &qi::path::userWritableDataPath,
                         "userWritableDataPath(application, file) -> string\n"
                         ":param application: string. Name of the application.\n"
                         ":param file: string. Name of the file.\n"
                         ":return: The file path.\n"
                         "\n"
                         "Get the writable data files path for users.");

      boost::python::def("userWritableConfPath", &qi::path::userWritableConfPath,
                         "userWritableConfPath(application, file) -> string\n"
                         ":param application: string. Name of the application.\n"
                         ":param file: string. Name of the file.\n"
                         ":return: The file path.\n"
                         "\n"
                         "Get the writable configuration files path for users.");
    }
  }
}
