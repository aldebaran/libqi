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
#include "qipython_convert.hpp"
#include <qitype/genericobject.hpp>
#include <qitype/metaobject.hpp>
#include <qimessaging/c/qi_c.h>
#include <qi/log.hpp>

// Windows trick
#ifdef WIN32
 #define strdup _strdup
#endif

// qi::signatureSplit return an empty sigv[1] when signature is not a full signature (method::(sss))
// For python bindings, when signature is not valid, we assume parameter was method name.
std::vector<std::string>  signatureSplit(const std::string &signature)
{
  std::vector<std::string> sigv;

  sigv = qi::signatureSplit(signature);

  if (sigv[1].compare("") == 0)
    sigv[1] = signature;

  return sigv;
}

void*     qi_raise(const char *exception_class, const char *error_message)
{
  char *non_const = strdup(exception_class);
  PyObject *err = PyErr_NewException(non_const, 0, 0);
  PyErr_SetString(err, error_message);
  free(non_const);
  return NULL;
}

qi_application_t *py_application_create(PyObject *args)
{
  PyObject *iter;
  int       argc, i;
  char      **argv;

  // #1 Get char** argv from PyObject *args
  iter = PyObject_GetIter(args);
  argc = PySequence_Size(args);
  if (argc <= 0)
  {
    qiLogError("qimessaging.python.py_application_create") << "Cannot convert system arguments : argc = " << argc;
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

static void __python_callback(const char *signature, qi_message_t *msg, qi_message_t *answer, void *data)
{
  std::vector<std::string> sigv = signatureSplit(signature);
  PyObject* param;
  PyObject* ret;
  PyObject* func;

  func = reinterpret_cast<PyObject *>(data);
  // Check if there is actually a python function in data
  if (!func || !PyCallable_Check(func))
  {
    qiLogError("qimessaging.python.__python_callback") << "Generic callback : No Python function to call.";
    return;
  }

  // Convert parameters (qi_message_t msg) in python tuple (PyObject param)
  if ((param = qi_message_to_python(sigv[2].c_str(), msg)) == Py_None || !param)
  {
    qiLogError("qimessaging.python.__python_callback") << "Generic callback : Cannot convert parameter in pyhton. [" << sigv[2] << "]";
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
    Py_XDECREF(func);
    Py_XDECREF(param);
    return; // My work here is done
  }

  // Convert return value (PyObject ret) in answer message (qi_message_t answer)
  qi_python_to_message(sigv[0].c_str(), answer, ret);

  // Satisfaction.
  Py_XDECREF(func);
  Py_XDECREF(param);
  Py_XDECREF(ret);
  return;
}

void qi_bind_method(qi_object_builder_t *builder, char *signature, PyObject *method)
{
  // #1 Register generic callback to object and give pointer to real python function in data parameter
  qi_object_builder_register_method(builder, signature, &__python_callback, method);
}

PyObject* qi_object_methods_vector(qi_object_t *object)
{
  qi::ObjectPtr &obj = *(reinterpret_cast<qi::ObjectPtr *>(object));

  std::string signature("(");
  qi_message_t* message = qi_message_create();
  qi::MetaObject::MethodMap mmm = obj->metaObject().methodMap();
  std::map<std::string, std::string> parsedMap;

  // #1 Merge same name.
  for (qi::MetaObject::MethodMap::iterator it = mmm.begin(); it != mmm.end(); ++it)
  {
    // #1.1 Split signature to get name.
    std::vector<std::string> sigv = signatureSplit((*it).second.signature());
    // #1.2 If there is already a signature with same name, replace with name only.
    if (parsedMap.find(sigv[1]) != parsedMap.end())
      parsedMap[sigv[1]] = sigv[1];
    else
      parsedMap[sigv[1]] = (*it).second.signature();
  }

  // #2 Serialise methods
  for (std::map<std::string, std::string>::iterator it = parsedMap.begin(); it != parsedMap.end(); ++it)
    qi_message_write_string(message, (*it).second.c_str());

  // #3 Create Python tuple signature
  signature.append(parsedMap.size(), 's');
  signature.append(1, ')');

  // #4 Return Python object from qi_message_t
  PyObject *ret = qi_message_to_python(signature.c_str(), message);

  qi_message_destroy(message);
  return ret;
}
