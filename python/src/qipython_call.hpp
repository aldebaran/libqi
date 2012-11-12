/*
**
** Author(s):
**  - Pierre ROULLON <proullon@aldebaran-robotics.com>
**
** Copyright (C) 2012 Aldebaran Robotics
*/

#ifndef _PYTHON_SRC_CALL_
#define _PYTHON_SRC_CALL_

#include <Python.h>

PyObject* qi_generic_call(qi_object_t *object_c, char *method_name, PyObject *args);

#endif // !_PYTHON_SRC_CALL_
