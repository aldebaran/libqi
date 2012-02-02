/*
**
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2011 Aldebaran Robotics
*/
#include <qimessaging/qi.h>
#include "qipython.hpp"
#include <Python.h>

static PyObject *qi_value_to_python(const char *sig, qi_message_t *msg);

static PyObject *qi_value_to_python_list(const char *sig, qi_message_t *msg)
{
  int       size    = qi_message_read_int(msg);
  int       i       = 0;
  PyObject *lst     = 0;

  lst = PyList_New(size);
  for (;i < size; ++i) {
    PyObject *p;
    p = qi_value_to_python(sig, msg);
    PyList_SetItem(lst, i, p);
  }
  return lst;
}


static PyObject *qi_value_to_python_dict(const char *sigk, const char *sigv, qi_message_t *msg)
{
  int       size    = qi_message_read_int(msg);
  int       i       = 0;
  PyObject *map     = 0;

  map = PyDict_New();
  for (;i < size; ++i) {
    PyObject *pk;
    PyObject *pv;

    pk = qi_value_to_python(sigk, msg);
    pv = qi_value_to_python(sigv, msg);
    PyDict_SetItem(map, pk, pv);
  }
  return map;
}

static PyObject *qi_value_to_python(const char *sig, qi_message_t *msg)
{
  PyObject *ret = 0;
  int retcode;

  switch (sig[0]) {
  case QI_BOOL:
    return PyBool_FromLong(qi_message_read_bool(msg));
  case QI_CHAR:
    return PyString_FromFormat("%c", qi_message_read_char(msg));
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
    {
      qi_signature_t *subsig = qi_signature_create_subsignature(sig);
      retcode = qi_signature_next(subsig);
      if (retcode != 0)
        return 0;
      ret = qi_value_to_python_list(subsig->current, msg);
      qi_signature_destroy(subsig);
      return ret;
    }
  case QI_MAP:
    {
      const char *k;
      const char *v;
      qi_signature_t *subsig = qi_signature_create_subsignature(sig);
      retcode = qi_signature_next(subsig);
      if (retcode != 0)
        return 0;
      k = subsig->current;
      retcode = qi_signature_next(subsig);
      if (retcode != 0)
        return 0;
      v = subsig->current;
      ret = qi_value_to_python_dict(k, v, msg);
      qi_signature_destroy(subsig);
      return ret;
    }
  case QI_TUPPLE:
      //TODO
      return 0;
  case QI_MESSAGE:
      //TODO
      return 0;
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
  int             items   = 0;
  int             i       = 0;

  sig     = qi_signature_create(signature);
  items   = qi_signature_count(sig);
  retcode = qi_signature_next(sig);
  if (retcode != 0 || items < 0)
    return 0;
  if (!sig->current || !*(sig->current)) {
    Py_INCREF(Py_None);
    return Py_None;
  }
  while (retcode == 0) {
    PyObject *obj = qi_value_to_python(sig->current, msg);
    retcode = qi_signature_next(sig);
    if (retcode == 2) {
      Py_XDECREF(obj);
      Py_XDECREF(ret);
      return 0;
    }
    if (retcode == 1 && !ret)
      return obj;
    if (!ret)
      ret = PyTuple_New(items);
    PyTuple_SetItem(ret, i, obj);
    if (retcode == 1) {
      break;
    }
    ++i;
  }
  qi_signature_destroy(sig);
  return ret;
}

//same as above but always return a tuple
PyObject *qi_message_to_python_tuple(const char *signature, qi_message_t *msg)
{
  PyObject       *ret     = 0;
  qi_signature_t *sig     = 0;
  int             retcode = 0;
  int             items   = 0;
  int             i       = 0;

  sig     = qi_signature_create(signature);
  items   = qi_signature_count(sig);
  retcode = qi_signature_next(sig);
  if (retcode != 0 || items < 0)
    return 0;
  ret = PyTuple_New(items);
  if (!sig->current || !*(sig->current)) {
    Py_INCREF(Py_None);
    PyTuple_SetItem(ret, 0, Py_None);
    return ret;
  }
  while (retcode == 0) {
    PyObject *obj = qi_value_to_python(sig->current, msg);
    retcode = qi_signature_next(sig);
    if (retcode == 2) {
      Py_XDECREF(obj);
      Py_XDECREF(ret);
      return 0;
    }
    PyTuple_SetItem(ret, i, obj);
    Py_XDECREF(obj);
    if (retcode == 1) {
      return ret;
    }
    ++i;
  }
  qi_signature_destroy(sig);
  return ret;
}

static int qi_value_to_message(const char *sig, PyObject *data, qi_message_t *msg)
{
  int retcode;
  switch (sig[0]) {
  case QI_BOOL:
    qi_message_write_bool(msg, PyInt_AsLong(data));
    return 0;
  case QI_CHAR:
    if (PyNumber_Check(data))
      qi_message_write_char(msg, PyInt_AsLong(data));
    else if (PyString_Check(data)) {
      char *str = PyString_AsString(data);
      if (!str)
        return 1;
      qi_message_write_char(msg, str[0]);
    }
    else
    {
      printf("WTF\n");
      return 1;
    }
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
      PyObject *iter = PyObject_GetIter(data);
      int       size = PySequence_Size(data);
      qi_message_write_int(msg, size);

      PyObject *currentObj = PyIter_Next(iter);
      int i = 0;
      //TODO: assert size = iter count
      while (currentObj) {
        qi_signature_t *subsig = qi_signature_create_subsignature(sig);
        retcode = qi_signature_next(subsig);
        if (retcode != 0)
          return retcode;
        qi_value_to_message(subsig->current, currentObj, msg);
        qi_signature_destroy(subsig);
        Py_XDECREF(currentObj);
        currentObj = PyIter_Next(iter);
        ++i;
      }
      Py_XDECREF(currentObj);
      Py_XDECREF(iter);
      if (i != size) {
        printf("aie aie aie\n");
        return 1;
      }
    }
    return 0;
      //return qi_value_to_python_list(sig, msg);
  case QI_MAP:
    {
      int size = PyDict_Size(data);
      qi_message_write_int(msg, size);

      PyObject *key, *value;
      Py_ssize_t pos = 0;

      //TODO: assert size = iter count
      while (PyDict_Next(data, &pos, &key, &value)) {
        char *k = 0;
        char *v = 0;
        qi_signature_t *subsig = qi_signature_create_subsignature(sig);
        retcode = qi_signature_next(subsig);
        if (retcode != 0)
          return retcode;
        k = subsig->current;

        retcode = qi_signature_next(subsig);
        if (retcode != 0)
          return retcode;
        v = subsig->current;

        qi_value_to_message(k, key, msg);
        qi_value_to_message(v, value, msg);
        qi_signature_destroy(subsig);
      }
      return 0;
    }
  case QI_TUPPLE:
    {
      qi_signature_t *subsig = qi_signature_create_subsignature(sig);
      retcode = qi_signature_next(subsig);
      if (retcode != 0)
        return retcode;
      qi_python_to_message(subsig->current, msg, data);
      qi_signature_destroy(subsig);
      return 0;
    }
  case QI_MESSAGE:
      return 1;
  default:
    return 1;
  }
  return 1;
}

int qi_python_to_message(const char *signature, qi_message_t *msg, PyObject *data)
{
  PyObject       *iter;
  qi_signature_t *sig = qi_signature_create(signature);
  int             retcode;

  //if none => return
  if (Py_None == data || !data)
  {
    qi_signature_destroy(sig);
    if (strlen(signature)) {
      return 2;
    }
    return 0;
  }

  iter = PyObject_GetIter(data);
  //we dont want the exception to be propagated if data is not iterable
  PyErr_Clear();
  //if single type => convert and return
  if (!iter || !PyIter_Check(iter)) {
    qi_value_to_message(signature, data, msg);
    qi_signature_destroy(sig);
    Py_XDECREF(iter);
    return 0;
  }

  PyObject *currentObj = PyIter_Next(iter);
  retcode = qi_signature_next(sig);
  while(retcode == 0 && currentObj) {
    qi_value_to_message(sig->current, currentObj, msg);
    Py_XDECREF(currentObj);
    currentObj = PyIter_Next(iter);
    retcode = qi_signature_next(sig);
  };
  Py_XDECREF(currentObj);
  Py_XDECREF(iter);
  qi_signature_destroy(sig);
  return retcode;
}

static void _qi_server_callback(const char *complete_sig, qi_message_t *params, qi_message_t *ret, void *data)
{
  PyObject *func = static_cast<PyObject *>(data);
  printf("server callback\n");
  if (PyCallable_Check(func))
  {
    char      callsig[100];
    char      retsig[100];
    PyObject *pyret;

    qi_signature_get_params(complete_sig, callsig, 100);
    qi_signature_get_return(complete_sig, retsig, 100);

    PyObject *args = qi_message_to_python_tuple(callsig, params);
    pyret = PyObject_CallObject(func, args);
    qi_python_to_message(retsig, ret, pyret);

  } else {
    printf("NOT Callable baby %p\n", func);
  }
}

// advertise a python service. It create a static c callback (_qi_server_callback) that
// take the callable PyObject in parameter. Then the callback is responsible for calling
// the PyObject with good parameters.
void qi_server_advertise_python_service(qi_server_t *server, const char *name, PyObject *func)
{
  if (!PyCallable_Check(func)) {
    printf("Fail... func is not callable\n");
    return;
  }
  printf("register callback %p\n", func);
  //increase the ref, because we store the object
  Py_XINCREF(func);
  //TODO: ctaf
  //qi_server_advertise_service(server, name, &_qi_server_callback, static_cast<void *>(func));
}

PyObject *qi_client_python_call(qi_client_t *client, const char *signature, PyObject *args) {
  char      callsig[100];
  char      retsig[100];
  PyObject *pyret;
  qi_message_t *params;
  qi_message_t *ret;

  qi_signature_get_params(signature, callsig, 100);
  qi_signature_get_return(signature, retsig, 100);
  params = qi_message_create();
  qi_message_write_string(params, signature);
  ret = qi_message_create();

  qi_python_to_message(callsig, params, args);
  qi_client_call(client, signature, params, ret);
  pyret = qi_message_to_python(retsig, ret);
  return pyret;
}


