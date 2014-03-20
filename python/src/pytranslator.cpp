/*
**  Copyright (C) 2014 Aldebaran Robotics
**  See COPYING for the license
*/

#include <boost/python.hpp>

#include <qipython/pytranslator.hpp>

namespace qi {
  namespace py {
   PyTranslator::PyTranslator(const std::string &name)
      : qi::Translator(name)
    { }

   std::string PyTranslator::translate1(const std::string &msg)
   {
     return qi::Translator::translate(msg);
   }

   std::string PyTranslator::translate2(const std::string &msg, const std::string &domain)
   {
     return qi::Translator::translate(msg, domain);
   }

   void export_pytranslator()
   {
     // If we don't tell boost it's non-copyable it will try to register a
     // converter for handling wrapped function
     // See https://wiki.python.org/moin/boost.python/class
     boost::python::class_<PyTranslator, boost::noncopyable>("Translator", boost::python::init<std::string>())
       .def("translate", &PyTranslator::translate1,
            "Translate a message from a domain to a locale")
       .def("translate", &PyTranslator::translate2,
            "Translate a message from a domain to a locale")
       .def("translate", &PyTranslator::translate,
            "Translate a message from a domain to a locale")
       .def("setCurrentLocale", &PyTranslator::setCurrentLocale,
            "Set the locale.")
       .def("setDefaultDomain", &PyTranslator::setDefaultDomain,
            "Set the domain.")
       .def("addDomain", &PyTranslator::addDomain,
            "Add a new domain.");
   }
  }
}
