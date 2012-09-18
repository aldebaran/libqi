/*
**
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**  - Pierre ROULLON <proullon@aldebaran-robotics.com>
**
** Copyright (C) 2011 Aldebaran Robotics
*/

#include <Python.h>

#include "qipython.hpp"
#include <qimessaging/genericobject.hpp>
#include <qimessaging/metaobject.hpp>
#include <qimessaging/c/qi_c.h>
#include <qi/log.hpp>

// Windows trick
#ifdef WIN32
 #define strdup _strdup
#endif

static PyObject *qi_message_to_python(const char *signature, qi_message_t *msg);
static int       qi_python_to_message(const char *signature, qi_message_t *msg, PyObject *data);
static PyObject *qi_value_to_python(const char *sig, qi_message_t *msg);
static PyObject *qi_message_to_python_tuple(qi_signature_t *sig, qi_message_t *msg);

// qi::signatureSplit return an empty sigv[1] when signature is not a full signature (method::(sss))
// For python bindings, when signature is not valid, we assume parameter was method name.
static std::vector<std::string>  signatureSplit(const std::string &signature)
{
  std::vector<std::string> sigv;

  sigv = qi::signatureSplit(signature);

  if (sigv[1].compare("") == 0)
    sigv[1] = signature;

  return sigv;
}

static void*     qi_raise(const char *exception_class, const char *error_message)
{
  char *non_const = strdup(exception_class);
  PyObject *err = PyErr_NewException(non_const, 0, 0);
  PyErr_SetString(err, error_message);
  free(non_const);
  return NULL;
}

static PyObject *qi_value_to_python_list(const char *sig, qi_message_t *msg)
{
  int       size    = qi_message_read_int32(msg);
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


static PyObject *qi_value_to_python(const char *sig, qi_message_t *msg)
{
  PyObject *ret = 0;
  int retcode;

  switch (sig[0]) {
  case QI_BOOL:
    return PyBool_FromLong(qi_message_read_bool(msg));
  case QI_CHAR:
    return PyString_FromFormat("%c", qi_message_read_int8(msg));
  case QI_INT:
    return PyInt_FromLong(qi_message_read_int32(msg));
  case QI_FLOAT:
    return PyFloat_FromDouble(qi_message_read_float(msg));
  case QI_DOUBLE:
    return PyFloat_FromDouble(qi_message_read_double(msg));
  case QI_STRING:
  {
    char *str = qi_message_read_string(msg);
    if (!str)
      return 0;
    return PyString_FromString(str);
  }
  case QI_LIST:
    {
      qi_signature_t *subsig = qi_signature_create_subsignature(sig);
      retcode = qi_signature_next(subsig);
      if (retcode != 0)
        return 0;
      ret = qi_value_to_python_list(qi_signature_current(subsig), msg);
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
  {
    //TODO
    qiLogFatal("qimessaging.python.qi_value_to_python") << "Returning NULL instead of tuple value.";
    return 0;
  }
  default:
  {
    qiLogFatal("qimessaging.python.qi_value_to_python") << "Returning NULL, no matching type.";
    return 0;
  }
  }
  return 0;
}


static PyObject *qi_message_to_python(const char *signature, qi_message_t *msg)
{
  PyObject       *ret     = 0;
  qi_signature_t *sig     = 0;
  int             items   = 0;

  sig     = qi_signature_create(signature);
  items   = qi_signature_count(sig);

  if (items < 0)
    Py_RETURN_NONE;

  if (!*(qi_signature_current(sig))) {
    Py_INCREF(Py_None);
    Py_RETURN_NONE;
  }

  qi_signature_type type = qi_signature_current_type(sig);

  if (items == 1 && type != QI_TUPLE)
    return (qi_value_to_python(qi_signature_current(sig), msg));

  qi_signature_t *subsig = qi_signature_create_subsignature(signature);

  if (!subsig)
  {
    qiLogError("qimessaging.python.qi_message_to_python") << "Signature \"" << signature << "\" is malformed";
    Py_RETURN_NONE;
  }

  ret = qi_message_to_python_tuple(subsig, msg);
  qi_signature_destroy(sig);
  return ret;
}

static PyObject *qi_message_to_python_tuple(qi_signature_t *sig, qi_message_t *msg)
{
  PyObject       *ret     = 0;
  int             retcode = 0;
  int             items   = 0;
  int             i       = 0;

  items   = qi_signature_count(sig);

  if (items < 0)
    return 0;

  ret = PyTuple_New(items);
  if (!*(qi_signature_current(sig))) {
    Py_INCREF(Py_None);
    PyTuple_SetItem(ret, 0, Py_None);

    return ret;
  }
  while (retcode == 0) {
    PyObject *obj = qi_value_to_python(qi_signature_current(sig), msg);
    retcode = qi_signature_next(sig);
    if (retcode == 2) {
      Py_XDECREF(obj);
      Py_XDECREF(ret);
      return 0;
    }
    PyTuple_SetItem(ret, i, obj);
    if (retcode == 1) {
      qi_signature_destroy(sig);
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
      qi_message_write_int8(msg, PyInt_AsLong(data));
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
    qi_message_write_float(msg, PyFloat_AsDouble(data));
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
    {
      PyObject *iter = PyObject_GetIter(data);
      int       size = PySequence_Size(data);
      qi_message_write_int32(msg, size);
      PyObject *currentObj = PyIter_Next(iter);
      int i = 0;
      //TODO: assert size = iter count
      while (currentObj) {
        qi_signature_t *subsig = qi_signature_create_subsignature(sig);
        retcode = qi_signature_next(subsig);
        if (retcode != 0)
          return retcode;
        qi_value_to_message(qi_signature_current(subsig), currentObj, msg);
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
      qi_message_write_int32(msg, size);

      PyObject *key, *value;
      Py_ssize_t pos = 0;

      //TODO: assert size = iter count
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
    {
      qi_signature_t *subsig = qi_signature_create_subsignature(sig);
      retcode = qi_signature_next(subsig);
      if (retcode != 0)
        return retcode;
      qi_python_to_message(qi_signature_current(subsig), msg, data);
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
  qi_signature_t *sig;
  int             retcode = 0;

  if (Py_None == data || !data)
    return strlen(signature) == 2 ? 2 :0;

  if ((sig = qi_signature_create_subsignature(signature)) == 0)
    return qi_value_to_message(signature, data, msg);

  iter = PyObject_GetIter(data);
  //we dont want the exception to be propagated if data is not iterable
  PyErr_Clear();

  if (!iter || !PyIter_Check(iter) || Py_None == iter)
  {
    qi_signature_destroy(sig);
    return 2;
  }

  PyObject *currentObj = PyIter_Next(iter);
  while(retcode == 0 && currentObj)
  {
    if (qi_value_to_message(qi_signature_current(sig), currentObj, msg) != 0)
    {
      qi_signature_destroy(sig);
      return 2;
    }
    currentObj = PyIter_Next(iter);
    retcode = qi_signature_next(sig);
  }

  if (currentObj)
    Py_XDECREF(currentObj);
  Py_XDECREF(iter);
  qi_signature_destroy(sig);
  return retcode == 2 ? 2 : 0;
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

static std::string                qi_pyobject_signature(PyObject *object)
{
  PyObject    *it, *obj;
  std::stringstream sig;

  it = PyObject_GetIter(object);

  // #1 Check wether object is iterable.
  if (!it || !PyIter_Check(it) || Py_None == it)
    return sig.str();

  obj = PyIter_Next(it);
  while (obj)
  {
    if (PyFloat_Check(obj))
      sig << QI_FLOAT;
    else if (PyNumber_Check(obj) && PyObject_Size(obj) == 1)
      sig << QI_CHAR;
    else if (PyString_Check(obj))
      sig << QI_STRING;
    else if (PyBool_Check(obj))
      sig << QI_BOOL;
    else if (PyInt_Check(obj))
      sig << QI_INT;
    else if (PyTuple_Check(obj))
      sig << QI_TUPLE << qi_pyobject_signature(obj) << QI_TUPLE_END;
    else if (PyList_Check(obj))
      sig << QI_LIST << qi_pyobject_signature(obj) << QI_LIST_END;
    else if (PyMapping_Check(obj))
      sig << QI_MAP << qi_pyobject_signature(obj) << QI_MAP_END;
    else
      sig << QI_UNKNOWN;
    obj = PyIter_Next(it);
  }

  return sig.str();
}

static qi::MetaMethod*             qi_get_convertible_method(qi::Object *object, const char *name, PyObject *args, std::vector<qi::MetaMethod> &candidates)
{
  std::string  sig;
  std::string  parameters;
  qi::MetaObject &meta = object->metaObject();
  std::vector<std::string> sigv;

  // #1 Deduce logical signature from parameters.
  parameters = qi_pyobject_signature(args);

  // #1.1 Check possible errors.
  if (parameters.compare("") == 0)
  {
    qi_raise("_qimessaging.CallError", "Parameters are not iterable.");
    return 0;
  }

  // #1.2 Concatenate elements.
  sig.append("(");
  sig.append(parameters);
  sig.append(")");

  // #2 Compare with actual choices.
  qi::Signature signature(sig);
  for (std::vector<qi::MetaMethod>::iterator it = candidates.begin(); it != candidates.end(); ++it)
  {
    // #2.1 Get paramter signature.
    sigv = signatureSplit((*it).signature());

    if (signature.isConvertibleTo(sigv[2]) == true)
      return meta.method(meta.methodId((*it).signature()));
  }

  qi_raise("_qimessaging.CallError", "Cannot find any suitable method");
  return 0;
}

static qi::MetaMethod*             qi_guess_method(qi_object_t *object_c, const char *sig, unsigned int nb_args, PyObject *args, std::vector<qi::MetaMethod> &candidates)
{
  int nb_matching_methods = 0;
  qi::MetaMethod last_matching_method;
  qi::GenericObject*               obj = reinterpret_cast<qi::GenericObject*>(object_c);
  const qi::MetaObject&     meta = obj->metaObject();

  // #0 Debug log
  qiLogDebug("qimessaging.python.qi_generic_call") << "Still " << candidates.size() << " candidates for " << sig << std::endl;
  for (std::vector<qi::MetaMethod>::iterator it = candidates.begin(); it != candidates.end(); ++it)
    qiLogDebug("qimessaging.python.qi_generic_call") << "\t" << (*it).signature();
  qiLogVerbose("qimessaging.python.qi_generic_call") << "{Performance warning} Desambiguation test for " << sig << std::endl;

  // #1 Test to convert argument into message with all remaining candidates
  for (std::vector<qi::MetaMethod>::iterator it = candidates.begin(); it != candidates.end(); ++it)
  {
    const char *signature = (*it).signature().c_str();
    qi_message_t* msg = qi_message_create();

    if (qi_python_to_message(signature, msg, args) == 0)
    {
      nb_matching_methods++;
      last_matching_method = (*it);
      qiLogDebug("qimessaging.python.qi_generic_call") << "Worked : "  << signature;
    }
  }

  // #1.1 If there is only one signature matching, bingo !
  if (nb_matching_methods == 1)
    return const_cast<qi::MetaMethod*>(meta.method(meta.methodId(last_matching_method.signature())));

  // #1.2 If there is no matching method, raise
  if (candidates.size() == 0)
  {
    qi_raise("_qimessaging.CallError", "Disambiguation test failure, No corresponding metamethod, please specify signature");
    return 0;
  }

  return qi_get_convertible_method(obj, sig, args, candidates);
}

static qi::MetaMethod*      qi_get_method(qi_object_t *object_c, const char *signature, unsigned int nb_args, PyObject *args)
{
  std::vector<std::string>  sigInfo;
  qi::GenericObject*               obj = reinterpret_cast<qi::GenericObject*>(object_c);
  const qi::MetaObject            &meta = obj->metaObject();
  const qi::MetaMethod*           mm;

  // #1 : Check if user give us complete signature
  sigInfo = signatureSplit(signature);

  if (sigInfo[2].compare("") != 0)
  {
     if ((mm = meta.method(meta.methodId(signature))) == 0)
       qi_raise("_qimessaging.CallError", "No metamethod fetching signature");
    return const_cast<qi::MetaMethod*>(mm);
  }

  // #2 : Get all function with same name. If only one, bingo.
  std::vector<qi::MetaMethod> mml = meta.findMethod(signature);
  if (mml.size() == 1)
    return const_cast<qi::MetaMethod*>(meta.method(meta.methodId(mml[0].signature())));

  // #2.1 : If no function left, raise
  if (mml.size() == 0)
  {
    qi_raise("_qimessaging.CallError", "No function found");
    return 0;
  }

  // #3 : Get all function with same name and good number of argument. Again, if only one, bingo :)
  std::vector<qi::MetaMethod> candidates;
  for (std::vector<qi::MetaMethod>::iterator it = mml.begin(); it != mml.end(); ++it)
    if (nb_args == ((*it).signature().size() - sigInfo[1].size() - 4))
      candidates.push_back((*it));
  if (candidates.size() == 1)
    return const_cast<qi::MetaMethod*>(meta.method(meta.methodId(candidates[0].signature())));

  // #3.1 : If no function left, raise
  if (candidates.size() == 0)
  {
    qi_raise("_qimessaging.CallError", "No corresponding metamethod, please specify signature");
    return 0;
  }

  return qi_guess_method(object_c, signature, nb_args, args, candidates);
}

PyObject* qi_generic_call(qi_object_t *object_c, char *method_name, PyObject *args)
{
  qi::MetaMethod* mm;
  int nb_args;

  nb_args = PyObject_Size(args);
  if ((mm = qi_get_method(object_c, method_name, nb_args, args)) == NULL)
    return NULL; // Exception should have been raised

  qi_message_t* msg = qi_message_create();
  std::vector<std::string> sigv = signatureSplit(mm->signature());

  if (qi_python_to_message(sigv[2].c_str(), msg, args) != 0)
  {
    qiLogError("qimessaging.python.qi_generic_call") << "Cannot convert parameter to qi::message :" << sigv[2] << std::endl;
    Py_RETURN_NONE;
  }
  qi_future_t *fut = (qi_future_t *) qi_object_call(object_c, mm->signature().c_str(), msg);
  qi_future_wait(fut);

  return qi_message_to_python(mm->sigreturn().c_str(), (qi_message_t *) qi_future_get_value(fut));
}

static void __python_callback(const char *signature, qi_message_t *msg, qi_message_t *answer, void *data)
{
  std::vector<std::string> sigv = signatureSplit(signature);
  PyObject* param;
  PyObject* ret;
  PyObject* func;

  func = reinterpret_cast<PyObject *>(data);
  // #1 Check if there is actually a python function in data
  if (!func || !PyCallable_Check(func))
  {
    qiLogError("qimessaging.python.__python_callback") << "Generic callback : No python function to call.";
    return;
  }

  // #2 Convert parameters (qi_message_t msg) in python tuple (PyObject param)
  if ((param = qi_message_to_python(sigv[2].c_str(), msg)) == Py_None || !param)
  {
    qiLogError("qimessaging.python.__python_callback") << "Generic callback : Cannot convert parameter in pyhton. [" << sigv[2] << "]";
    return;
  }

  // #3.1 Call python function
  ret = PyObject_CallObject(func, param);

  // #3.2 Check if return value is None
  if (!ret || ret == Py_None)
    return; // My work here is done

  // #4.1 Convert return value (PyObject ret) in answer message (qi_message_t answer)
  qi_python_to_message(sigv[0].c_str(), answer, ret);

  // #4.2 Satisfaction.
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
  qi::Object *obj = reinterpret_cast<qi::Object *>(object);
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
