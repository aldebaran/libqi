/*
**
** Author(s):
**  - Pierre ROULLON <proullon@aldebaran-robotics.com>
**
** Copyright (C) 2011, 2012 Aldebaran Robotics
*/

#include <Python.h>

#include "qipython.hpp"
#include "qipython_convert.hpp"
#include <qitype/genericobject.hpp>
#include <qitype/metaobject.hpp>
#include <qitype/signature.hpp>
#include <qimessaging/c/qi_c.h>
#include <qi/log.hpp>

void*  qi_raise(const char *exception_class, const char *error_message);
static PyObject* qi_value_to_python(const char *sig, qi_message_t *msg);
static int qi_value_to_message(const char *sig, PyObject *data, qi_message_t *msg);

static PyObject *qi_value_to_python_dict(const char *sigk, const char *sigv, qi_message_t *msg)
{
  int       size    = qi_message_read_int32(msg);
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

static PyObject *qi_value_to_python_list(const char *signature, qi_message_t *msg)
{
  int       size;
  PyObject *lst = 0;
  qi_signature_t* sig;

  // #1 Initlialize qi::Signature subsignature.
  if ((sig = qi_signature_create_subsignature(signature)) == 0)
  {
    qi_raise("SerializationError", "Signature is not valid.");
    return 0;
  }

  // #2 Get size of list
  size = qi_message_read_int32(msg);

  // #3 Create a new empty Python list.
  if ((lst = PyList_New(size)) == 0)
  {
    qi_raise("SerializationError", "Cannot instanciate Python list.");
    return 0;
  }

  // #4 For each element in message, deserialize it and add it into Python list.
  for (int i = 0; i < size; ++i)
  {
    PyObject *p = qi_value_to_python(qi_signature_current(sig), msg);
    PyList_SetItem(lst, i, p);
  }

  // #Cleanup
  qi_signature_destroy(sig);
  return lst;
}

static PyObject *qi_message_to_python_tuple(const char *signature, qi_message_t *msg)
{
  qi_signature_t *sig     = 0;
  PyObject       *ret     = 0;
  PyObject       *obj     = 0;
  int             retcode = 0;
  int             i       = 0;

  // Initialize qi::Signature subsignature.
  if ((sig = qi_signature_create_subsignature(signature)) == 0)
  {
    qi_raise("SerializationError", "Signature is not valid.");
    return 0;
  }

  // Instanciate empty Python tuple.
  ret = PyTuple_New(qi_signature_count(sig));

  // If signature is empty, return empty tuple to Python function.
  if (!*(qi_signature_current(sig)))
  {
    qi_signature_destroy(sig);
    return ret;
  }

  // For each element in signature and in message, add it in tuple.
  while (retcode == 0)
  {
    // Get Python element.
    if ((obj = qi_value_to_python(qi_signature_current(sig), msg)) == 0)
    {
      qi_raise("SerializationError", "Cannot deserialize tuple from qi::Message.");
      qi_signature_destroy(sig);
      return 0;
    }

    // Add it into tuple.
    PyTuple_SetItem(ret, i, obj);

    retcode = qi_signature_next(sig);
    ++i;
  }

  // Cleanup
  qi_signature_destroy(sig);
  return ret;
}

static PyObject *qi_value_to_python(const char *sig, qi_message_t *msg)
{
  PyObject *ret = 0;
  int retcode;

  switch (sig[0]) {
  case QI_BOOL:
    return PyBool_FromLong(qi_message_read_bool(msg));
  case QI_CHAR:
    return PyString_FromFormat("%c", qi_message_read_int8(msg));
  case QI_FLOAT:
    return PyFloat_FromDouble(qi_message_read_float(msg));
  case QI_INT:
    return PyInt_FromLong(qi_message_read_int32(msg));
  case QI_DOUBLE:
    return PyFloat_FromDouble(qi_message_read_double(msg));
  case QI_STRING:
  {
    char *str = qi_message_read_string(msg);
    if (!str)
      return 0;
    ret = PyString_FromString(str);
    ::free(str);
    return ret;
  }
  case QI_LIST:
    return qi_value_to_python_list(sig, msg);
  case QI_MAP:
    {
      const char *k;
      const char *v;
      qi_signature_t *subsig = qi_signature_create_subsignature(sig);
      retcode = qi_signature_next(subsig);
      if (retcode != 0)
        return 0;
      k = qi_signature_current(subsig);
      retcode = qi_signature_next(subsig);
      if (retcode != 0)
        return 0;
      v = qi_signature_current(subsig);
      ret = qi_value_to_python_dict(k, v, msg);
      qi_signature_destroy(subsig);
      return ret;
    }
  case QI_TUPLE:
    return qi_message_to_python_tuple(sig, msg);
  case QI_VOID:
    Py_RETURN_NONE;
  default:
  {
    qiLogFatal("qimessaging.python.qi_value_to_python") << "Returning NULL, no matching type.";
    return 0;
  }
  }
  return 0;
}


PyObject *qi_message_to_python(const char *signature, qi_message_t *msg)
{
  PyObject       *ret     = 0;
  qi_signature_t *sig     = 0;
  int             items   = 0;

  // #1 Create qi::Signature and count supposed number of items in qi::Messaging (msg).
  sig   = qi_signature_create(signature);
  items = qi_signature_count(sig);

  // #2 If there is no item, just return.
  if (sig == 0 || items < 0)
  {
    qiLogWarning("qimessaging.python") << "Signature is empty.";
    qi_signature_destroy(sig);
    Py_RETURN_NONE;
  }

  // #3 If signature is empty, just return.
  if (!*(qi_signature_current(sig)))
  {
    qiLogWarning("qimessaging.python") << "Signature is empty.";
    qi_signature_destroy(sig);
    Py_RETURN_NONE;
  }

  qi_signature_destroy(sig);
  ret = qi_value_to_python(signature, msg);
  return ret;
}

static int qi_list_to_message(const char *sig, PyObject *data, qi_message_t *msg)
{
  PyObject *iter;
  int       size;
  qi_signature_t *subsig;

  // #1 Compute size of Python list.
  size = PySequence_Size(data);

  // #2 Get an iterator on Python list
  if ((iter = PyObject_GetIter(data)) == 0)
  {
    qiLogError("qimessaging.python.qi_list_to_message") << "[QI_LIST] Element is not iterable.";
    return 2;
  }

  // #3 Initialize qi::Signature subsignature.
  if ((subsig = qi_signature_create_subsignature(sig)) == 0)
  {
    qiLogError("qimessaging.python.qi_list_to_message") << "[QI_LIST] Signature " << sig << " is not valid.";
    return 2;
  }

  // #4 Write size of list in message.
  qi_message_write_int32(msg, size);

  // #5 For each element of list, serialize it into qi::Message (msg).
  PyObject *currentObj = PyIter_Next(iter);
  while (currentObj && size > 0)
  {
    // #5.1 Serialize current element.
    if (qi_value_to_message(qi_signature_current(subsig), currentObj, msg) != 0)
    {
      qi_signature_destroy(subsig);
      Py_XDECREF(currentObj);
      Py_XDECREF(iter);
      return 2;
    }

    // #5.2 Get next element in list, signature doesn't change.
    Py_XDECREF(currentObj);
    currentObj = PyIter_Next(iter);
    size--;
  }

  // #Cleanup
  qi_signature_destroy(subsig);
  Py_XDECREF(currentObj);
  Py_XDECREF(iter);

  // #Return 0 if number of element written is actualy equal to declared number.
  return size == 0 ? 0 : 2;
}

static int qi_tuple_to_message(const char *sig, PyObject *data, qi_message_t *msg)
{
  int             retcode = 0;
  PyObject*       iter;
  qi_signature_t* subsig;

  // #1 Get subsignature of tuple
  if ((subsig = qi_signature_create_subsignature(sig)) == 0)
  {
    qiLogError("qimessaging.python.qi_value_to_message") << "[QI_TUPLE] Signature " << sig << " not valid.";
    return 2;
  }

  // #2 Declare an iterator on tuple
  iter = PyObject_GetIter(data);

  // #3 For each element in tuple, serialize it in message.
  PyObject *currentObj = PyIter_Next(iter);
  while(retcode == 0 && currentObj)
  {
    if (qi_value_to_message(qi_signature_current(subsig), currentObj, msg) != 0)
    {
      qiLogError("qimessaging.python.qi_tuple_to_message") << "Cannot serialize data with " << qi_signature_current(subsig) << " signature.";
      qi_signature_destroy(subsig);
      return 2;
    }
    currentObj = PyIter_Next(iter);
    retcode = qi_signature_next(subsig);
  }

  // #Cleanup
  qi_signature_destroy(subsig);
  return 0;
}

static int qi_value_to_message(const char *sig, PyObject *data, qi_message_t *msg)
{
  int retcode;

  switch (sig[0]) {
  case QI_BOOL:
    qi_message_write_bool(msg, (char) !!PyInt_AsLong(data));
    return 0;
  case QI_CHAR:
    if (PyNumber_Check(data))
      qi_message_write_int8(msg, static_cast<char>(PyInt_AsLong(data)));
    else if (PyString_Check(data)) {
      char *str = PyString_AsString(data);
      if (!str)
        return 1;
      qi_message_write_int8(msg, str[0]);
    }
    else
      return 1;
    return 0;
  case QI_INT:
    qi_message_write_int32(msg, PyInt_AsLong(data));
    return 0;
  case QI_FLOAT:
    qi_message_write_float(msg, static_cast<float>(PyFloat_AsDouble(data)));
    return 0;
  case QI_DOUBLE:
    qi_message_write_double(msg, PyFloat_AsDouble(data));
    return 0;
  case QI_STRING:
  {
    char *str = PyString_AsString(data);
    if (!str)
      return 1;
    qi_message_write_string(msg, str);
    return 0;
  }
  case QI_LIST:
    return qi_list_to_message(sig, data, msg);
  case QI_MAP:
    {
      int size = PyDict_Size(data);
      qi_message_write_int32(msg, size);

      PyObject *key, *value;
      Py_ssize_t pos = 0;

     while (PyDict_Next(data, &pos, &key, &value)) {
        const char *k = 0;
        const char *v = 0;
        qi_signature_t *subsig = qi_signature_create_subsignature(sig);
        retcode = qi_signature_next(subsig);
        if (retcode != 0)
          return retcode;
        k = qi_signature_current(subsig);

        retcode = qi_signature_next(subsig);
        if (retcode != 0)
          return retcode;
        v = qi_signature_current(subsig);

        qi_value_to_message(k, key, msg);
        qi_value_to_message(v, value, msg);
        qi_signature_destroy(subsig);
      }
      return 0;
    }
  case QI_TUPLE:
    return qi_tuple_to_message(sig, data, msg);
  case QI_MESSAGE:
      return 1;
  default:
    return 1;
  }
  return 1;
}

int qi_python_to_message(const char *signature, qi_message_t *msg, PyObject *data)
{
  // If there is no parameter but signature represent void '()' arguments, returns 0.
  if (Py_None == data || !data || ::strcmp(signature, "()") == 0)
  {
    // Remove log when it bother us.
    qiLogWarning("qimessaging.python") << "No parameter to serialize: " << signature;
    return ::strcmp(signature, "()") == 0 ? 0 : 2;
  }

  return qi_value_to_message(signature, data, msg);
}
