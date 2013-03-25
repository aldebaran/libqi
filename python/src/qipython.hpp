/*
**
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2011, 2012 Aldebaran Robotics
*/

#ifndef         QIPYTHON_H_
# define        QIPYTHON_H_

# include <qic/session.h>
# include <qic/object.h>
# include <qic/application.h>
# include <qic/future.h>
# include <string>
# include <Python.h>

extern "C" {

  qi_application_t* qipy_application_create(PyObject *args);

  PyObject*         qipy_object_call(qi_object_t* objectc, const char* strMethodName, PyObject* listParams);
  PyObject*         qipy_object_get_metaobject(qi_object_t *object);
  void              qipy_objectbuilder_bind_method(qi_object_builder_t *builder, const char *signature, PyObject *object);

  unsigned int      qipy_session_register_object(qi_session_t *session, char *name, PyObject *object, PyObject *attr);

  void              qipy_future_add_callback(qi_future_t* future, PyObject* pyfuture, PyObject* function);
  PyObject*         qipy_future_get_value(qi_future_t*future);
  void              qipy_promise_set_value(qi_promise_t* promise, PyObject* value);
  qi_promise_t*     qipy_promise_cancelable_create(PyObject* pypromise, PyObject* callable);

}

#endif      /* !QIPYTHON_H_ */
