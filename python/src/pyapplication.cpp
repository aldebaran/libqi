/*
**  Copyright (C) 2013 Aldebaran Robotics
**  See COPYING for the license
*/
#include "pyapplication.hpp"
#include <qi/os.hpp>
#include <qi/log.hpp>
#include <qimessaging/applicationsession.hpp>
#include <boost/python.hpp>
#include "gil.hpp"
#include <boost/thread.hpp>
#include <qi/atomic.hpp>
#include "pysession.hpp"

qiLogCategory("qimpy");

namespace qi {
  namespace py {
    static void initApp(boost::shared_ptr<qi::Application>& app, int& argc, char**& argv)
    {
      app = boost::shared_ptr<qi::Application>(new qi::Application(argc, argv));
    }

    static void initApp(boost::shared_ptr<qi::ApplicationSession>& app, int& argc, char**& argv)
    {
      app = boost::shared_ptr<qi::ApplicationSession>(new qi::ApplicationSession(argc, argv, qi::ApplicationSession::Option_NoAutoExit));
    }

    template<typename T> void initNoArg(T& app)
    {
      int argc = 0;
      static char *argvstorage[2] = { 0, 0 };
      char **argv = argvstorage;
      initApp(app, argc, argv);
    }

    template<typename T> void initWithArgs(T& app, boost::python::list& args)
    {
      int  argc, i;
      char **argv;
      argc = boost::python::len(args);
      argv = new char*[argc + 1];
      for (i = 0; i < argc; ++i) {
        std::string s = boost::python::extract<std::string>(args[i]);
        /*TODO: leak*/
        argv[i] = qi::os::strdup(s.c_str());
        qiLogInfo() << "args:" << argv[i];
      }
      /* #2 Create c application */
      initApp(app, argc, argv);
      /* Update the input list */
      for (int i = boost::python::len(args); i > 0; --i) {
        args.pop(i - 1);
      }
      for (int i = 0; i < app->argc(); ++i) {
        args.insert(i, std::string(argv[i]));
      }
      /* #3 Free C arguments */
      i = 0;
      delete[] argv;
    }

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
      PyApplication() {
        initNoArg(_app);
      }

      PyApplication(boost::python::list args) {
        initWithArgs(_app, args);
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
      boost::python::class_<PyApplication>("Application")
          .def(boost::python::init<boost::python::list>())
          .def("run", &PyApplication::run)
          .def("stop", &PyApplication::stop);
    }

    static void noDelete(qi::Session*) {
    }

    class PyApplicationSession {
    public:
      PyApplicationSession() {
        initNoArg(_app);
        initSes();
      }

      PyApplicationSession(boost::python::list args) {
        initWithArgs(_app, args);
        initSes();
      }

      void initSes() {
        _sesPtr = boost::shared_ptr<qi::Session>(&_app->session(), noDelete);
        _ses = makePySession(_sesPtr);
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
        qi::py::GILScopedLock _unlock;
        _app->start();
      }

    private:
      boost::python::object                     _ses;
      boost::shared_ptr<qi::ApplicationSession> _app;
      boost::shared_ptr<qi::Session>            _sesPtr;
    };

    void export_pyapplicationsession() {
      boost::python::class_<PyApplicationSession>("ApplicationSession")
          .def(boost::python::init<boost::python::list>())
          .def("run", &PyApplicationSession::run)
          .def("stop", &PyApplicationSession::stop)
          .def("session", &PyApplicationSession::session)
          .def("start", &PyApplicationSession::start)
          .def("url", &PyApplicationSession::url)
          ;
    }
  } // !py
} // !qi
