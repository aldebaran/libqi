/*
**
** Author(s):
**  - Pierre ROULLON <proullon@aldebaran-robotics.com>
**
** Copyright (C) 2012 Aldebaran Robotics
*/

#include <Python.h>

#include "qipython.hpp"
#include "qipython_convert.hpp"

#include <qi/future.hpp>
#include <qitype/genericobject.hpp>
#include <qitype/metaobject.hpp>
#include <qimessaging/c/qi_c.h>
#include <qi/log.hpp>

qiLogCategory("qimessaging.python");

void*     qi_raise(const char *exception_class, const char *error_message);

static PyObject* qi_generic_call_python(qi::ObjectPtr object, const std::string& strMethodName, PyObject* listParams)
{
  PyObject    *it, *current;
  qi::GenericFunctionParameters params;
  std::string signature;

  it = PyObject_GetIter(listParams);
  current = PyIter_Next(it);
  while (current)
  {
    // The line below is ok because we know current is
    // a byvalue GenericValuePtr
    qi::GenericValuePtr val = qi::GenericValuePtr::ref(current);
    params.push_back(val);
    signature += val.signature();
    current = PyIter_Next(it);
  }

  qi::Future<qi::GenericValuePtr> fut = object->metaCall(strMethodName + "::(" + signature + ")", params);
  qi::Promise<PyObject *> out;

  if (fut.hasError())
  {
    out.setError(fut.error());
    qi_raise("qimessaging.genericobject.CallError", out.future().error().c_str());
    return 0;
  }

  return fut.value().to<PyObject*>();
}

// New qi_generic_call : Just cast and call qi_generic_call_python
PyObject* qi_generic_call(qi_object_t *object_c, char *method_name, PyObject *args)
{
  qi::ObjectPtr obj = *(reinterpret_cast<qi::ObjectPtr*>(object_c));

  qiLogVerbose() << "Calling " << method_name;
  return qi_generic_call_python(obj, method_name, args);
}
