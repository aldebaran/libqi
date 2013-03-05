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

#include <qi/log.hpp>
#include <qi/os.hpp>

#include <qitype/genericobject.hpp>
#include <qitype/metaobject.hpp>
#include <qitype/genericobjectbuilder.hpp>

#include <qimessaging/c/qi_c.h>

qiLogCategory("qimessaging.python");

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

qi_application_t *py_application_create(PyObject *args)
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
    qiLogError() << "Generic callback : No Python function to call.";
    return;
  }

  // Convert parameters (qi_message_t msg) in python tuple (PyObject param)
  if ((param = qi_message_to_python(sigv[2].c_str(), msg)) == Py_None || !param)
  {
    qiLogError() << "Generic callback : Cannot convert parameters to python. [" << sigv[2] << "]";
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

  // Convert return value (PyObject ret) in answer message (qi_message_t answer)
  qi_python_to_message(sigv[0].c_str(), answer, ret);

  // Satisfaction.
  Py_XDECREF(param);
  Py_XDECREF(ret);
  return;
}

void qi_bind_method(qi_object_builder_t *builder, const char *signature, PyObject *method)
{
  // #1 Register generic callback to object and give pointer to real python function in data parameter
  qi_object_builder_register_method(builder, signature, &__python_callback, method);
}

PyObject* qi_get_sigreturn(qi_object_t *object, const char *signature)
{
  qi::ObjectPtr &obj = *(reinterpret_cast<qi::ObjectPtr *>(object));
  std::string sigreturns;
  std::vector<qi::MetaObject::CompatibleMethod> mm = obj->metaObject().findCompatibleMethod(std::string(signature));

  for (std::vector<qi::MetaObject::CompatibleMethod>::iterator it = mm.begin(); it != mm.end(); ++it)
  {
    if (sigreturns.empty() == false)
      sigreturns.append(",").append(it->first.sigreturn());
    else
      sigreturns = it->first.sigreturn();
  }

  qi::GenericValuePtr val(&sigreturns);
  return val.to<PyObject*>();
}

PyObject* qi_get_object_description(qi_object_t *object)
{
  qi::ObjectPtr& obj = *(reinterpret_cast<qi::ObjectPtr *>(object));

  qi::GenericValuePtr val = qi::GenericValuePtr::ref(obj->metaObject().description());
  return val.to<PyObject*>();
}

PyObject* qi_get_method_description(qi_object_t *object, const char *signature)
{
  qi::ObjectPtr &obj = *(reinterpret_cast<qi::ObjectPtr *>(object));
  std::vector<qi::MetaMethod> mm = obj->metaObject().findMethod(std::string(signature));
  std::string description;

  if (mm.begin() != mm.end()) {
    description = mm.front().description();
  }

  qi::GenericValuePtr val = qi::GenericValuePtr::ref(description);
  return val.to<PyObject*>();
}

PyObject* qi_get_sigreturn_description(qi_object_t *object, const char *sig) {
  qi::ObjectPtr &obj = *(reinterpret_cast<qi::ObjectPtr *>(object));
  std::vector<qi::MetaMethod> mm = obj->metaObject().findMethod(std::string(sig));
  std::string description;

  if (mm.begin() != mm.end()) {
    description = mm.front().returnDescription();
  }

  qi::GenericValuePtr val = qi::GenericValuePtr::ref(description);
  return val.to<PyObject*>();
}

PyObject* qi_get_parameters_descriptions(qi_object_t *object, const char *sig) {
  qi::ObjectPtr &obj = *(reinterpret_cast<qi::ObjectPtr *>(object));
  std::vector<qi::MetaMethod> mm = obj->metaObject().findMethod(std::string(sig));
  std::vector<std::string> res;

  if (mm.begin() != mm.end()) {
    qi::MetaMethod m = mm.front();
    qi::MetaMethodParameterVector v = m.parameters();
    qi::MetaMethodParameterVector::iterator it;
    for (it = v.begin(); it != v.end(); ++it) {
      res.push_back(it->name() + ": " + it->description());
    }
  }

  qi::GenericValuePtr val = qi::GenericValuePtr::ref(res);
  return val.to<PyObject*>();
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
    // #1.2 If there is already a signature with same name, add signature after a coma.
    if (parsedMap.find(sigv[1]) != parsedMap.end())
      parsedMap[sigv[1]] = parsedMap[sigv[1]].append(",").append((*it).second.signature());
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

unsigned int py_session_register_object(qi_session_t *session, char *name, PyObject *object, PyObject *attr)
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

      qi_bind_method((qi_object_builder_t*) &ob, signature.c_str(), method);
    }
  }

  // Get object from object builder and give it to session
  qi::ObjectPtr obj = ob.object();
  return qi_session_register_service(session, name, (qi_object_t*) &obj);
}
