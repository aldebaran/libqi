/*
**  Copyright (C) 2013 Aldebaran Robotics
**  See COPYING for the license
*/
#include <boost/python.hpp>
#include <qipython/pyapplication.hpp>
#include <qi/os.hpp>
#include <qi/log.hpp>
#include <qi/applicationsession.hpp>
#include <qipython/gil.hpp>
#include <boost/thread.hpp>
#include <qi/atomic.hpp>
#include <qipython/pysession.hpp>

qiLogCategory("qimpy");

namespace qi {
  namespace py {

    //convert a python list to an old C argc/argv pair.
    //the dtor destroy argc/argv
    class ArgumentConverter {
    public:
      ArgumentConverter() {
        argc = 0;
        argv = new char*[2];
        argv[0] = 0;
        argv[1] = 0;

      }

      ArgumentConverter(boost::python::list& args)
      {
        argc = boost::python::len(args);
        argv = new char*[argc + 1];
        for (int i = 0; i < argc; ++i) {
          std::string s = boost::python::extract<std::string>(args[i]);
          argv[i] = qi::os::strdup(s.c_str());
          qiLogVerbose() << "arg[:" << i << "]" << argv[i];
        }
      }

      void update(boost::python::list& args) {
        /* Update the input list */
        for (int i = boost::python::len(args); i > 0; --i) {
          args.pop(i - 1);
        }
        for (int i = 0; i < argc; ++i) {
          args.insert(i, std::string(argv[i]));
        }
      }

      ~ArgumentConverter() {
        for (int i = 0; i < argc; ++i) {
          /* Free C argument */
          free(argv[i]);
        }
        delete[] argv;
        argv = 0;
        argc = 0;
      }

      int    argc;
      char** argv;
    };

    void destroylater(boost::shared_ptr<qi::Application>, qi::Atomic<int> *doClose)
    {
      //wait til ~PyApplication have been destroyed
      while (!(**doClose))
        qi::os::msleep(1);
      delete doClose;
    }

    template<typename T> void destroyApplication(T& app)
    {
      qi::py::GILScopedUnlock _unlock;
      /* force the destruction to happend into the other thread */
      qi::Atomic<int>* goClosing = new qi::Atomic<int>;
      boost::thread(&destroylater, app, goClosing);
      app.reset();
      ++(*goClosing);
    }

    class PyApplication {
    public:
      PyApplication(boost::python::list args) {
        ArgumentConverter         ac(args);

        _app = boost::shared_ptr<qi::Application>(new qi::Application(ac.argc, ac.argv));
        ac.update(args);
      }

      //delay the destruction of _app to a thread, because the destruction can lock
      //app destroy eventloop that wait for all socket/server to be destroyed.
      //that way python can continue to destroy object, and eventually destroy the last socket/server
      //that will release the thread finally
      ~PyApplication() {
        destroyApplication(_app);
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
      boost::python::class_<PyApplication>("Application", boost::python::init<boost::python::list>())
          .def("run", &PyApplication::run)
          .def("stop", &PyApplication::stop);
    }

    class PyApplicationSession {
    public:
      PyApplicationSession(boost::python::list args, bool autoExit, const std::string& url) {
        ArgumentConverter         ac(args);
        ApplicationSessionOptions aso = qi::ApplicationSession::Option_None;

        if (!autoExit)
          aso = qi::ApplicationSession::Option_NoAutoExit;

        _app = boost::shared_ptr<qi::ApplicationSession>(new qi::ApplicationSession(ac.argc, ac.argv, aso, url));
        //update args (some can have been removed by the AppSes ctor
        ac.update(args);
        _ses = makePySession(_app->session());
      }

      ~PyApplicationSession() {
        destroyApplication(_app);
      }

      void run() {
        qi::py::GILScopedUnlock _unlock;
        _app->run();
      }

      void stop() {
        qi::py::GILScopedUnlock _unlock;
        _app->stop();
      }

      boost::python::object session() {
        return _ses;
      }

      boost::python::object url() {
        return boost::python::object(_app->url().str());
      }

      void start() {
        qi::py::GILScopedUnlock _unlock;
        _app->start();
      }

    private:
      boost::python::object                     _ses;
      boost::shared_ptr<qi::ApplicationSession> _app;
      boost::shared_ptr<qi::Session>            _sesPtr;
    };

    void export_pyapplicationsession() {
      boost::python::class_<PyApplicationSession>("ApplicationSession", boost::python::init<boost::python::list, bool, std::string>())
          .def("run", &PyApplicationSession::run,
               "run()\n"
               "Block until the end of the program. (call :py:func:`qi.ApplicationSession.stop` to end the program)")
          .def("stop", &PyApplicationSession::stop,
               "stop()\n"
               "Ask the application to stop, the run function will return.")
          .def("start", &PyApplicationSession::start,
               "start()\n"
               "Start the connection of the session, once this function is called everything is fully initialized and working.")
          .add_property("url", &PyApplicationSession::url,
                        "url\n"
                        "The url given to the Application. It's the url used to connect the session")
          .add_property("session", &PyApplicationSession::session,
                        "session\n"
                        "The session associated to the application")
          ;
    }
  } // !py
} // !qi
