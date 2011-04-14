/*
**
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2011 Aldebaran Robotics
*/

#ifndef         QIPYTHON_H_
# define        QIPYTHON_H_

# include <qimessaging/qi.h>
# include <Python.h>

extern "C" {

  void      qi_server_advertise_python_service(qi_server_t *server, const char *name, PyObject *func);
  PyObject *qi_client_python_call(qi_client_t *client, const char *signature, PyObject *args);

  PyObject *qi_message_to_python(const char *signature, qi_message_t *msg);
  int       qi_python_to_message(const char *signature, qi_message_t *msg, PyObject *data);

}

#endif      /* !QIPYTHON_H_ */
