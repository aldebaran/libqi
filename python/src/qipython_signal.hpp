/*
**
** Author(s):
**  - Julien Freche <jfreche@aldebaran-robotics.com>
**
** Copyright (C) 2012 Aldebaran Robotics
*/

#ifndef QI_PYTHON_SIGNAL_HPP_
# define QI_PYTHON_SIGNAL_HPP_

# include <qitype/signal.hpp>

// If included first doesn't compile with clang on OSX
# include <Python.h>

class qi_signal
{
  public:
    qi_signal();
    ~qi_signal();

    unsigned int connect(PyObject* callback);
    bool disconnect(unsigned int);
    bool disconnectAll();

    void trigger(PyObject* arg0 = NULL,
                 PyObject* arg1 = NULL,
                 PyObject* arg2 = NULL,
                 PyObject* arg3 = NULL,
                 PyObject* arg4 = NULL,
                 PyObject* arg5 = NULL,
                 PyObject* arg6 = NULL,
                 PyObject* arg7 = NULL,
                 PyObject* arg8 = NULL,
                 PyObject* arg9 = NULL);

  private:
    void callback(PyObject* callback, const std::vector<PyObject*>& args);

    std::map<unsigned int, PyObject*>               _callbackMap;
    qi::Signal<void (std::vector<PyObject*>)>       _sig;

    class EventLoopHandler
    {
      public:
        EventLoopHandler()
          : eventLoop ()
        {
          eventLoop.start();
        }

        ~EventLoopHandler()
        {
          eventLoop.stop();
          eventLoop.join();
        }

        qi::EventLoop eventLoop;
    };

    static EventLoopHandler                         _elHandler;
};

#endif // !QI_PYTHON_SIGNAL_HPP_
