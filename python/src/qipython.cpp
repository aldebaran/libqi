/*
**
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**  - Pierre ROULLON <proullon@aldebaran-robotics.com>
**
** Copyright (C) 2011, 2012 Aldebaran Robotics
*/

#include <Python.h>

#include "qipython.hpp"

#include <qi/log.hpp>
#include <qi/os.hpp>

#include <qitype/genericobject.hpp>
#include <qitype/metaobject.hpp>
#include <qitype/genericobjectbuilder.hpp>
#include "src/value_p.h"
#include "src/future_p.h"

qiLogCategory("qipy");

void*     qi_raise(const char *exception_class, const char *error_message)
{
  PyObject* exc = NULL;
  char*     exc_class = qi::os::strdup(exception_class);

  // We require a class name.
  if (exception_class == NULL) {
    exc = PyExc_TypeError;
    PyErr_SetString(exc, "qi_raise must be called with a exception class name.");
    return NULL;
  }

  // Parsing name, to try and import exception.
  // mod_name, class_name = exception_name.rsplit('.', 1) ...
  // XXX: Use std::string for more safety?
  char* mod_name = qi::os::strdup(exception_class);
  char* class_name = mod_name;

  while (class_name && *class_name)
    class_name++;
  while (class_name && class_name != mod_name && *class_name != '.')
    class_name--;

  if (class_name == mod_name) {
    exc = PyExc_ValueError;
    PyErr_SetString(exc, mod_name);
    free(exc_class);
    free(mod_name);
    return NULL;
  } else
    *(class_name++) = '\0';

  // Importing module and trying to fetch the class.
  PyObject* modname = PyString_FromString(mod_name);
  PyObject* module = PyImport_Import(modname);

  if (module != NULL) {
    PyObject* mdict = PyModule_GetDict(module);
    PyObject* klass = PyDict_GetItemString(mdict, class_name);
    if (klass == NULL) // FIXME: || !PyClass_IsSubclass(klass, PyExc_Exception))
      exc = PyErr_NewException(exc_class, NULL, NULL);
    else
      exc = klass;
  } else {
    // Unknown exception: we create it but it won't be catchable.
    exc = PyErr_NewException(exc_class, NULL, NULL);
  }
  PyErr_SetString(exc, error_message);

  free(mod_name);
  free(exc_class);
  Py_DECREF(modname);
  Py_XDECREF(module);
  return NULL;
}

qi_application_t *qipy_application_create(PyObject *args)
{
  PyObject *iter;
  int       argc, i;
  char      **argv;

  // #0 Initialize Python threads, for code in C/C++ that will call python object in a thread
  PyEval_InitThreads();

  // #1 Get char** argv from PyObject *args
  iter = PyObject_GetIter(args);
  argc = PySequence_Size(args);
  if (argc <= 0)
  {
    qiLogError() << "Cannot convert system arguments : argc = " << argc;
    return 0;
  }

  argv = new char*[argc + 1];
  PyObject *it = PyIter_Next(iter);
  i = 0;
  while (it)
  {
    argv[i] = PyString_AsString(it);
    it = PyIter_Next(iter);
    i++;
  }

  // #2 Create c application
  qi_application_t *app = qi_application_create(&argc, argv);

  // #3 Free C arguements
  i = 0;
  delete[] argv;

  return app;
}

static void python_call_callback(const char *signature, qi_value_t *msg, qi_value_t *answer, void *data)
{
  PyObject* param;
  PyObject* ret;
  PyObject* func;

  qi::GenericValuePtr &gmsg = qi_value_cpp(msg);
  func = reinterpret_cast<PyObject *>(data);
  // Check if there is actually a python function in data
  if (!func || !PyCallable_Check(func))
  {
    qiLogError() << "Generic callback : No Python function to call.";
    return;
  }

  param = gmsg.as<PyObject *>();
  // Convert parameters (qi_message_t msg) in python tuple (PyObject param)
  if (!param)
  {
    qiLogError() << "Generic callback : Cannot convert parameters to python. [" << signature << "]";
    return;
  }

  // Clear Python errors
  PyErr_Clear();

  // Call python function.
  ret = PyObject_CallObject(func, param);
  // Force print of possible error, it won't print anything if no error occured.
  PyErr_Print();

  // Check if return value is None
  if (!ret || ret == Py_None)
  {
    Py_XDECREF(param);
    return; // My work here is done
  }

  qi::GenericValue &ganswer = qi_value_cpp(answer);
  ganswer = qi::GenericValue::from(ret);

  // Satisfaction.
  Py_XDECREF(func);
  Py_XDECREF(param);
  Py_XDECREF(ret);
  return;
}

void qipy_objectbuilder_bind_method(qi_object_builder_t *builder, const char *signature, PyObject *method)
{
  Py_XINCREF(method);
  // #1 Register generic callback to object and give pointer to real python function in data parameter
  qi_object_builder_register_method(builder, signature, &python_call_callback, method);
}

PyObject* qipy_object_get_metaobject(qi_object_t *object)
{
  qi::ObjectPtr &obj = *(reinterpret_cast<qi::ObjectPtr *>(object));
  const qi::MetaObject &mo = obj->metaObject();
  qi::GenericValuePtr gvp = qi::GenericValueRef(mo);
  return gvp.as<PyObject*>();
}

unsigned int qipy_session_register_object(qi_session_t *session, char *name, PyObject *object, PyObject *attr)
{
  PyObject *iter, *method, *attribute, *sig;

  // Check if object is a class
  if (PyInstance_Check(object) == false)
  {
    qiLogError() << "Register object : Given object is not a class instance.";
    qi_raise("qimessaging.session.RegisterError",
             "Register object : Given object is not a class instance.");
    return 0;
  }

  // Declare qimessaging object builder
  qi::GenericObjectBuilder ob;

  // Get iterator on object attributes
  if (!(iter = PyObject_GetIter(attr)))
  {
    qiLogError() << "Register object : Given object attributes is not iterable.";
    qi_raise("qimessaging.session.RegisterError",
             "Register object : Given object attributes is not iterable.");
    return 0;
  }

  // For each attribute of class
  while ((attribute = PyIter_Next(iter)))
  {
    method = PyTuple_GetItem(attribute, 0);
    sig = PyTuple_GetItem(attribute, 1);

    // register method using code of qi_object_builder_register_method
    if (PyMethod_Check(method) == true)
    {
      std::string signature(PyString_AsString(sig));

      qipy_objectbuilder_bind_method((qi_object_builder_t*) &ob, signature.c_str(), method);
    }
  }

  // Get object from object builder and give it to session
  qi::ObjectPtr obj = ob.object();
  //return qi_session_register_service(session, name, (qi_object_t*) &obj);
  return 0xFA11;
}

PyObject* qipy_object_call(qi_object_t* objectc, const char *strMethodName, PyObject* listParams)
{
  PyObject    *it, *current;
  qi::GenericFunctionParameters params;
  std::string signature;
  qi::ObjectPtr object = *(reinterpret_cast<qi::ObjectPtr*>(objectc));

  it = PyObject_GetIter(listParams);
  current = PyIter_Next(it);
  while (current)
  {
    // The line below is ok because we know current is
    // a byvalue GenericValuePtr
    qi::GenericValuePtr val = qi::GenericValueRef(current);
    params.push_back(val);
    signature += val.signature();
    current = PyIter_Next(it);
  }
  std::string mname = strMethodName;
  mname += "::(" + signature + ")";

  qi::Future<qi::GenericValuePtr> fut = object->metaCall(mname, params);
  qi::Promise<PyObject *> out;

  if (fut.hasError())
  {
    out.setError(fut.error());
    qi_raise("qimessaging.genericobject.CallError", out.future().error().c_str());
    return 0;
  }

  return fut.value().to<PyObject*>();
}


static void python_future_callback(qi::Future<qi::GenericValue> &fut, PyObject* pyfuture, PyObject *pyfunc)
{
  // Check if there is actually a python function in data
  if (!pyfunc || !PyCallable_Check(pyfunc))
  {
    qiLogError() << "Generic callback : No Python function to call.";
    return;
  }

  // Clear Python errors
  PyErr_Clear();

  PyObject* param = PyTuple_New(1);
  PyTuple_SetItem(param, 0, pyfuture);

  // Call python function.
  PyObject* ret = PyObject_CallObject(pyfunc, param);

  // Force print of possible error, it won't print anything if no error occured.
  PyErr_Print();

  Py_XDECREF(pyfunc);
  Py_XDECREF(param);
  Py_XDECREF(ret);
}

void qipy_future_add_callback(qi_future_t* future, PyObject* pyfuture, PyObject* function) {
  qiLogInfo() << "qi_future_add_cb(" << future << ")";
  //callback fun with the future in argument.
  Py_XINCREF(pyfuture);
  Py_XINCREF(function);
  qi::Future<qi::GenericValue> *fut = qi_future_cpp(future);
  fut->connect(boost::bind<void>(&python_future_callback, _1, pyfuture, function));
}

PyObject*         qipy_future_get_value(qi_future_t*future) {
  qi::Future<qi::GenericValue> *fut = qi_future_cpp(future);
  return fut->value().to<PyObject*>();
}

void qipy_promise_set_value(qi_promise_t* promise, PyObject* value) {
  qiLogInfo() << "setvalue promise";
  qi::Promise<qi::GenericValue> *prom = qi_promise_cpp(promise);
  prom->setValue(qi::GenericValue::from(value));
}
