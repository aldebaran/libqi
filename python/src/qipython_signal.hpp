/*
**
** Author(s):
**  - Julien Freche <jfreche@aldebaran-robotics.com>
**
** Copyright (C) 2012 Aldebaran Robotics
*/

#ifndef QI_PYTHON_SIGNAL_HPP_
# define QI_PYTHON_SIGNAL_HPP_

# include <Python.h>

# include <qitype/signal.hpp>

class signal
{
  public:
    signal();
    ~signal();

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
};

#endif // !QI_PYTHON_SIGNAL_HPP_
