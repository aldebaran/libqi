/*
**
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2011 Aldebaran Robotics
*/
#include <qi/qi.h>
#include "qipython.hpp"
#include <Python.h>

static PyObject *qi_value_to_python_list(qi_signature_t *sig, qi_message_t *msg)
{
  int       size    = qi_message_read_int(msg);
  int       retcode = qi_signature_next(sig);
  int       i       = 0;
  PyObject *lst     = 0;

  if (retcode != 0)
    return 0;

  lst = PyList_New(size);
  for (;i < size; ++i) {
    PyList_Append(lst, qi_value_to_python(sig->current, msg));
  }
  return lst;
}


static PyObject *qi_value_to_python_dict(qi_signature_t *sig, qi_message_t *msg)
{
  int       size    = qi_message_read_int(msg);
  int       retcode = 0;
  int       i       = 0;
  PyObject *map     = 0;

  retcode = qi_signature_next(sig);
  if (retcode != 0)
    return 0;
  //need to store sk, because the next call to qi_signature_next will change sig->current
  char *sk = strdup(sig->current);

  retcode = qi_signature_next(sig);
  if (retcode != 0) {
    free(sk);
    return 0;
  }
  char *sv = sig->current;

  map = PyDict_New();
  for (;i < size; ++i) {
    PyDict_SetItem(map, qi_value_to_python(sk, msg), qi_value_to_python(sv, msg));
  }

  free(sk);
  free(sv);
  return map;
}

PyObject *qi_value_to_python(qi_signature_t *sig, qi_message_t *msg)
{
  PyObject *ret = 0;

  switch (sig->current[0]) {
  case QI_BOOL:
    return PyBool_FromLong(qi_message_read_bool(msg));
  case QI_CHAR:
    return PyInt_FromLong(qi_message_read_char(msg));
  case QI_INT:
    return PyInt_FromLong(qi_message_read_int(msg));
  case QI_FLOAT:
    return PyFloat_FromDouble(qi_message_read_float(msg));
  case QI_DOUBLE:
    return PyFloat_FromDouble(qi_message_read_double(msg));
  case QI_STRING:
    //TODO return PyString_FromStringAndSize(msg->readString(), val.size());
    return PyString_FromString(qi_message_read_string(msg));
  case QI_LIST:
      return qi_value_to_python_list(sig, msg);
  case QI_MAP:
      return qi_value_to_python_dict(sig, msg);
  default:
    return 0;
  }
  return 0;
}


PyObject *qi_message_to_python(const char *signature, qi_message_t *msg)
{
  PyObject       *ret     = 0;
  qi_signature_t *sig     = 0;
  int             retcode = 0;

  sig     = qi_signature_create(signature);
  retcode = qi_signature_next(sig);
  if (retcode != 0)
    return 0;
  if (!sig->current || !*(sig->current)) {
    Py_INCREF(Py_None);
    return Py_None;
  }
  while (retcode == 0) {
    PyObject *obj = qi_value_to_python(sig, msg);
    retcode = qi_signature_next(sig);
    if (retcode != 2) {
      Py_XDECREF(obj);
      return 0;
    }
    if (retcode == 1) {
      if (!ret)
        return obj;
      return ret;
    }
    if (!ret)
      ret = PyList_New(0);
    PyList_Append(ret, obj);
  }
  qi_signature_destroy(sig);
  return ret;
}

int qi_value_to_message(qi_signature_t *sig, PyObject *data, qi_message_t *msg)
{
  switch (sig->current[0]) {
  case QI_BOOL:
    qi_message_write_bool(msg, PyInt_AsLong(data));
    return 0;
  case QI_CHAR:
    qi_message_write_char(msg, PyInt_AsLong(data));
    return 0;
  case QI_INT:
    qi_message_write_int(msg, PyInt_AsLong(data));
    return 0;
  case QI_FLOAT:
    qi_message_write_float(msg, PyFloat_AsDouble(data));
    return 0;
  case QI_DOUBLE:
    qi_message_write_double(msg, PyFloat_AsDouble(data));
    return 0;
  case QI_STRING:
    qi_message_write_string(msg, PyString_AsString(data));
    return 0;
  case QI_LIST:
    {
      //qi_message_write_python_list()
      int size = PyList_Size(data);
      int i = 0;
      for (; i < size; ++i) {
        //TODO
        //qi_value_to_message()
      }
    }
    return 0;
      //return qi_value_to_python_list(sig, msg);
  case QI_MAP:
      break;
      ;
      //return qi_value_to_python_dict(sig, msg);
      return 0;
  default:
    return 1;
  }
  return 1;
}

int qi_python_to_message(const char *signature, qi_message_t *msg, PyObject *data)
{
  qi_signature_t *sig = qi_signature_create(signature);
  int             retcode;

  //if none => return
  if (Py_None == data)
  {
    if (strlen(signature)) {
      printf("WTF?\n");
      return 2;
    }
    return 0;
  }

  //if single type => convert and return
  if (!PyIter_Check(data)) {
    qi_value_to_message(sig, data, msg);
    return 0;
  }

  PyObject *currentObj = PyIter_Next(data);
  retcode = qi_signature_next(sig);
  while(retcode == 0 && currentObj) {
    qi_value_to_message(sig, currentObj, msg);
    currentObj = PyIter_Next(data);
    retcode = qi_signature_next(sig);
  };
  return retcode;
}

static void _qi_server_callback(qi_message_t *params, qi_message_t *ret, void *data)
{
  PyObject *func = static_cast<PyObject *>(data);
  printf("server callback\n");
  if (!PyCallable_Check(func)) {
    printf("Callable baby\n");
    PyObject *pyret;
    char *sig     = qi_message_read_string(params);
    char *callsig = qi_signature_get_params(sig);
    char *retsig  = qi_signature_get_return(sig);

    PyObject *args = qi_message_to_python(sig, params);
    pyret = PyObject_CallObject(func, args);

    qi_python_to_message(retsig, ret, pyret);
    free(sig);
    free(callsig);
    free(retsig);
  }
}

// advertise a python service. It create a static c callback (_qi_server_callback) that
// take the callable PyObject in parameter. Then the callback is responsible for calling
// the PyObject with good parameters.
void qi_server_advertise_python_service(qi_server_t *server, const char *name, PyObject *func)
{
  if (!PyCallable_Check(func)) {
    printf("Fail... func is not callable\n");
  }
  printf("register callback\n");
  qi_server_advertise_service(server, name, &_qi_server_callback, static_cast<void *>(func));
}

