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

static PyObject *qi_value_to_python_list(const char *parent_sig, qi_message_t *msg)
{
  int             size    = qi_message_read_int(msg);
  char           *ssubsig = strdup(parent_sig);
  int             len     = strlen(ssubsig);

  if (len < 3)
  {
    free(ssubsig);
    return 0;
  }

  //we dont want the first and last char, remove them
  ssubsig[len - 1] = 0;
  qi_signature_t *subsig = qi_signature_create(ssubsig + 1);
  char *current = qi_signature_get_next(subsig);

  int       i   = 0;
  PyObject *lst = PyList_New(size);

  for (;i < size; ++i) {
    PyList_Append(lst, qi_value_to_python(current, msg));
  }

  if (current)
    free(current);
  free(ssubsig);
  qi_signature_destroy(subsig);
  return lst;
}


static PyObject *qi_value_to_python_dict(const char *parent_sig, qi_message_t *msg)
{
  int             size    = qi_message_read_int(msg);
  char           *ssubsig = strdup(parent_sig);
  int             len     = strlen(ssubsig);

  if (len < 3)
  {
    free(ssubsig);
    return 0;
  }

  //we dont want the first and last char, remove them
  ssubsig[len - 1] = 0;
  qi_signature_t *subsig = qi_signature_create(ssubsig + 1);
  char *sk = qi_signature_get_next(subsig);
  char *sv = qi_signature_get_next(subsig);

  int       i   = 0;
  PyObject *map = PyDict_New();

  for (;i < size; ++i) {
    PyDict_SetItem(map, qi_value_to_python(sk, msg), qi_value_to_python(sv, msg));
  }

  if (sk)
    free(sk);
  if (sv)
    free(sv);
  free(ssubsig);
  qi_signature_destroy(subsig);
  return map;
}

PyObject *qi_value_to_python(const char *sig, qi_message_t *msg)
{
  PyObject *ret = 0;

  switch (sig[0]) {
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
  PyObject       *ret = 0;
  qi_signature_t *sig = 0;
  char           *current = 0;

  sig     = qi_signature_create(signature);
  current = qi_signature_get_next(sig);

  if (!current) {
    Py_INCREF(Py_None);
    return Py_None;
  }
  while (current) {
    PyObject *obj = qi_value_to_python(current, msg);
    free(current);
    current = qi_signature_get_next(sig);
    if (!current && !ret) {
      return obj;
    }
    if (!ret)
      ret = PyList_New(0);
    PyList_Append(ret, obj);
  }

  qi_signature_destroy(sig);
  return ret;
}

void qi_value_to_message(const char *signature, PyObject *data, qi_message_t *msg)
{
  switch (signature[0]) {
  case QI_BOOL:
    qi_message_write_bool(msg, PyInt_AsLong(data));
    return;
  case QI_CHAR:
    qi_message_write_char(msg, PyInt_AsLong(data));
    return;
  case QI_INT:
    qi_message_write_int(msg, PyInt_AsLong(data));
    return;
  case QI_FLOAT:
    qi_message_write_float(msg, PyFloat_AsDouble(data));
    return;
  case QI_DOUBLE:
    qi_message_write_double(msg, PyFloat_AsDouble(data));
    return;
  case QI_STRING:
    qi_message_write_string(msg, PyString_AsString(data));
    return;
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
    break;
      //return qi_value_to_python_list(sig, msg);
  case QI_MAP:
      break;
      ;
      //return qi_value_to_python_dict(sig, msg);
  default:
    return;
  }
}

void qi_python_to_message(const char *signature, qi_message_t *msg, PyObject *data)
{
  qi_signature_t *sig = qi_signature_create(signature);
  char           *current;

  //if none => return
  if (Py_None == data)
  {
    if (strlen(signature))
      printf("WTF?\n");
    return;
  }

  //if single type => convert and return
  if (!PyIter_Check(data))
  {
    qi_value_to_message(signature, data, msg);
    return;
  }

  PyObject *currentObj = PyIter_Next(data);
  current              = qi_signature_get_next(sig);
  while(current && currentObj) {
    free(current);
    current = qi_signature_get_next(sig);
    qi_value_to_message(current, currentObj, msg);
    currentObj = PyIter_Next(data);
    free(current);
  };

  if (current || currentObj) {
    printf("WTF?\n");
  }
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

