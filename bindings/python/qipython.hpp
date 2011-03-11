/*
**
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2011 Aldebaran Robotics
*/

#ifndef         QIPYTHON_H_
# define        QIPYTHON_H_

# include <qi/qi.h>
# include <Python.h>

extern "C" {
  void qi_server_advertise_python_service(qi_server_t *server, const char *name, PyObject *func);

  PyObject *qi_value_to_python(const char *sig, qi_message_t *msg);
}

#endif      /* !QIPYTHON_H_ */
