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
      boost::python::def("setFilters", &pysetfilters,
                         "setFilters(filters) -> None\n"
                         ":param filters: List of rules separated by colon.\n"
                         "\n"
                         "Set log filtering options.\n"
                         "Each rule can be:\n\n"
                         "  +CAT: enable category CAT\n\n"
                         "  -CAT: disable category CAT\n\n"
                         "  CAT=level : set category CAT to level\n\n"
                         "Each category can include a '*' for globbing.\n"
                         "\n"
                         ".. code-block:: python\n"
                         "\n"
                         "  qi.logging.setFilter(\"qi.*=debug:-qi.foo:+qi.foo.bar\")\n"
                         "\n"
                         "(all qi.* logs in info, remove all qi.foo logs except qi.foo.bar)\n");

      boost::python::def("setContext", &pysetcontext,
                         "setContext(context) -> None\n"
                         ":param context: A bitfield (add values descibe below).\n"
                         "\n"
                         "  1  : Verbosity                            \n\n"
                         "  2  : ShortVerbosity                       \n\n"
                         "  4  : Date                                 \n\n"
                         "  8  : ThreadId                             \n\n"
                         "  16 : Category                             \n\n"
                         "  32 : File                                 \n\n"
                         "  64 : Function                             \n\n"
                         "  128: EndOfLine                            \n\n"
                         "  Some useful values for context are:       \n\n"
                         "  26 : (verb+threadId+cat)                  \n\n"
                         "  30 : (verb+threadId+date+cat)             \n\n"
                         "  126: (verb+threadId+date+cat+file+fun)    \n\n"
                         "  254: (verb+threadId+date+cat+file+fun+eol)\n\n"
                         "\n");

      boost::python::def("setLevel", &pysetlevel,
                         "setLevel(level) -> None\n"
                         ":param level: The logger level need to be choose between "
                         "FATAL, ERROR, WARNING, INFO, VERBOSE, DEBUG\n"
                         "\n"
                         "Sets the threshold for the logger to level. "
                         "Logging messages which are less severe than level will be ignored. "
                         "Note that the logger is created with level INFO.");
    }
  }
}
