/*
**  Copyright (C) 2013 Aldebaran Robotics
**  See COPYING for the license
*/


#include <qipython/pylog.hpp>

#include <boost/python.hpp>

#include <qi/application.hpp>
#include <qi/log.hpp>
#include <qi/os.hpp>

namespace qi {
  namespace py {
    static void pylog(int level,
                      const std::string& name,
                      const std::string& message,
                      const std::string& file,
                      const std::string& func,
                      int line)
    {
      qi::log::log((qi::LogLevel)level,
                   name.c_str(),
                   message.c_str(),
                   file.c_str(),
                   func.c_str(),
                   line);
    }

    static void pysetlevel(int level)
    {
      qi::log::setVerbosity((qi::LogLevel)level);
    }

    static void pysetcontext(int context)
    {
      qi::log::setContext(context);
    }

    static void pysetfilters(const std::string& filters)
    {
      qi::log::setVerbosity(filters);
    }



    void export_pylog()
    {
      boost::python::def("pylog", &pylog);
      boost::python::def("setFilters", &pysetfilters);
      boost::python::def("setContext", &pysetcontext);
      boost::python::def("setLevel", &pysetlevel);
    }
  }
}
