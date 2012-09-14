/*
**
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2011 Aldebaran Robotics
*/

#ifndef         QIPYTHON_H_
# define        QIPYTHON_H_

# include <qimessaging/c/qi_c.h>
# include <Python.h>

extern "C" {

  qi_application_t *py_application_create(PyObject *args);
  PyObject*         qi_generic_call(qi_object_t *object_c, char *method_name, PyObject *args);
  void              qi_bind_method(qi_object_builder_t *builder, char *signature, PyObject *object);
}

#endif      /* !QIPYTHON_H_ */
