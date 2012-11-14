/*
**
** Author(s):
**  - Julien Freche <jfreche@aldebaran-robotics.com>
**
** Copyright (C) 2012 Aldebaran Robotics
*/

#pragma once
#ifndef PYTHONSCOPEDREF_HPP_
# define PYTHONSCOPEDREF_HPP_

# include <Python.h>

class PythonScopedRef
{
  public:
    PythonScopedRef(PyObject* p)
      : _p (p)
    {
      Py_XINCREF(_p);
    }

    ~PythonScopedRef()
    {
      Py_XDECREF(_p);
    }

  private:
    PyObject* _p;
};

#endif // !PYTHONREFCOUNTER_HPP_
