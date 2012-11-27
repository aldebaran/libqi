/*
**
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2011, 2012 Aldebaran Robotics
*/

#ifndef         QIPYTHON_H_
# define        QIPYTHON_H_

# include <qimessaging/c/qi_c.h>
# include <Python.h>

extern "C" {

  qi_application_t* py_application_create(PyObject *args);
  PyObject*         qi_generic_call(qi_object_t *object_c, char *method_name, PyObject *args);
  void              qi_bind_method(qi_object_builder_t *builder, const char *signature, PyObject *object);
  PyObject*         qi_object_methods_vector(qi_object_t *object);
  unsigned int      py_session_register_object(qi_session_t *session, char *name, PyObject *object, PyObject *attr);
  PyObject*         qi_get_sigreturn(qi_object_t *object, const char *signature);
}

#endif      /* !QIPYTHON_H_ */
