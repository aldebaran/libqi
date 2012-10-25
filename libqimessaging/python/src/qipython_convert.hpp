/*
**
** Author(s):
**  - Pierre ROULLON <proullon@aldebaran-robotics.com>
**
** Copyright (C) 2012 Aldebaran Robotics
*/

#ifndef _PYTHON_SRC_CONVERT_
#define _PYTHON_SRC_CONVERT_

#include <Python.h>

PyObject* qi_message_to_python(const char *signature, qi_message_t *msg);
int       qi_python_to_message(const char *signature, qi_message_t *msg, PyObject *data);

#endif // !_PYTHON_SRC_CONVERT_
