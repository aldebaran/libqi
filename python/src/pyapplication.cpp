/*
**
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2013 Aldebaran Robotics
*/

#include "pyapplication.hpp"
#include <qi/os.hpp>
#include <qi/log.hpp>
#include <qi/application.hpp>
#include <boost/python.hpp>

qiLogCategory("qimpy");

namespace qi {
  namespace py {

    class PyApplication {
    public:
      PyApplication() {
        int argc = 0;
        char *argvstorage[2] = { 0, 0 };
        char **argv = argvstorage;
        // #2 Create c application
        _app = new qi::Application(argc, argv);
      }

      PyApplication(boost::python::tuple args)
      {
        PyObject *iter;
        int       argc, i;
        char      **argv;

        //TODO: remove THAT
        // #0 Initialize Python threads, for code in C/C++ that will call python object in a thread
        //PyEval_InitThreads();

        ///args.slice().size();
        argc = boost::python::len(args);
        argv = new char*[argc + 1];

        for (i = 0; i < argc; ++i) {
          std::string s = boost::python::extract<std::string>(args[i]);
          //TODO: leak
          argv[i] = qi::os::strdup(s.c_str());
          qiLogInfo() << "args:" << argv[i];
        }

        // #2 Create c application
        _app = new qi::Application(argc, argv);

        // #3 Free C arguements
        i = 0;
        delete[] argv;
      }

      void run() {
        _app->run();
      }

      void stop() {
        _app->stop();
      }

    private:
      qi::Application *_app;
    };

    void export_pyapplication() {
      boost::python::class_<PyApplication>("Application")
          .def(boost::python::init<boost::python::tuple>())
          .def("run", &PyApplication::run)
          .def("stop", &PyApplication::stop);
    }

  }
}
