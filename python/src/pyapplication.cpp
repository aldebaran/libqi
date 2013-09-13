/*
**  Copyright (C) 2013 Aldebaran Robotics
**  See COPYING for the license
*/
#include "pyapplication.hpp"
#include <qi/os.hpp>
#include <qi/log.hpp>
#include <qi/application.hpp>
#include <boost/python.hpp>
#include "gil.hpp"
#include <boost/thread.hpp>
#include <qi/atomic.hpp>

qiLogCategory("qimpy");

namespace qi {
  namespace py {
    void destroylater(boost::shared_ptr<qi::Application>, qi::Atomic<int> *doClose)
    {
      //wait til ~PyApplication have been destroyed
      while (!(**doClose))
        qi::os::msleep(1);
      delete doClose;
    }

    class PyApplication {
    public:
      PyApplication() {
        int argc = 0;
        char *argvstorage[2] = { 0, 0 };
        char **argv = argvstorage;
        // #2 Create c application
        _app = boost::shared_ptr<qi::Application>(new qi::Application(argc, argv));
      }

      PyApplication(boost::python::tuple args)
      {
        int       argc, i;
        char      **argv;
        argc = boost::python::len(args);
        argv = new char*[argc + 1];

        for (i = 0; i < argc; ++i) {
          std::string s = boost::python::extract<std::string>(args[i]);
          //TODO: leak
          argv[i] = qi::os::strdup(s.c_str());
          qiLogInfo() << "args:" << argv[i];
        }

        // #2 Create c application
        _app = boost::shared_ptr<qi::Application>(new qi::Application(argc, argv));

        // #3 Free C arguments
        i = 0;
        delete[] argv;
      }


      //delay the destruction of _app to a thread, because the destruction can lock
      //app destroy eventloop that wait for all socket/server to be destroyed.
      //that way python can continue to destroy object, and eventually destroy the last socket/server
      //that will release the thread finally
      ~PyApplication() {
        qi::py::GILScopedUnlock _unlock;
        //force the destruction to happend into the other thread
        qi::Atomic<int>* goClosing = new qi::Atomic<int>;
        boost::thread(&destroylater, _app, goClosing);
        _app.reset();
        ++(*goClosing);
      }

      void run() {
        qi::py::GILScopedUnlock _unlock;
        _app->run();
      }

      void stop() {
        qi::py::GILScopedUnlock _unlock;
        _app->stop();
      }

    private:
      boost::shared_ptr<qi::Application> _app;
    };

    void export_pyapplication() {
      boost::python::class_<PyApplication>("Application")
          .def(boost::python::init<boost::python::tuple>())
          .def("run", &PyApplication::run)
          .def("stop", &PyApplication::stop);
    }

  }
}
