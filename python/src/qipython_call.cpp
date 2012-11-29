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

#include <qitype/genericobject.hpp>
#include <qitype/metaobject.hpp>
#include <qimessaging/c/qi_c.h>
#include <qi/log.hpp>

void*     qi_raise(const char *exception_class, const char *error_message);
//std::vector<std::string>  signatureSplit(const std::string &signature);

static PyObject* qi_generic_call_python(qi::ObjectPtr object, const std::string& strMethodName, PyObject* listParams)
{
  PyObject    *it, *current;
  qi::GenericFunctionParameters params;
  std::string signature;

  it = PyObject_GetIter(listParams);
  current = PyIter_Next(it);
  while (current)
  {
    qi::GenericValue val = qi::GenericValue::from(current);
    params.push_back(val);
    signature += val.signature();
    current = PyIter_Next(it);
  }

  qi::Future<qi::GenericValuePtr> fut = object->metaCall(strMethodName + "::(" + signature + ")", params);
  qi::Promise<PyObject *> out;

  if (fut.hasError())
  {
    out.setError(fut.error());
    qi_raise("_qimessaging.CallError", out.future().error().c_str());
    return 0;
  }

  return out.future().value();
}

// New qi_generic_call : Just cast and call qi_generic_call_python
PyObject* qi_generic_call(qi_object_t *object_c, char *method_name, PyObject *args)
{
  qi::ObjectPtr obj = *(reinterpret_cast<qi::ObjectPtr*>(object_c));

  return qi_generic_call_python(obj, method_name, args);
}

/*
 *static std::string                qi_pyobject_signature(PyObject *object, bool loop = true)
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
      sig << (char) QI_FLOAT;
    else if (PyNumber_Check(obj) && PyObject_Size(obj) == 1)
      sig << (char) QI_CHAR;
    else if (PyString_Check(obj))
      sig << (char) QI_STRING;
    else if (PyBool_Check(obj))
      sig << (char) QI_BOOL;
    else if (PyInt_Check(obj))
      sig << (char) QI_INT;
    else if (PyTuple_Check(obj))
      sig << (char) QI_TUPLE << qi_pyobject_signature(obj) << (char) QI_TUPLE_END;
    else if (PyList_Check(obj))
      sig << (char) QI_LIST << qi_pyobject_signature(obj, false) << (char) QI_LIST_END;
    else if (PyMapping_Check(obj))
      sig << (char) QI_MAP << qi_pyobject_signature(obj) << (char) QI_MAP_END;
    else
      sig << (char) QI_UNKNOWN;
    obj = loop == true ? PyIter_Next(it) : 0;
  }

  return sig.str();
}*/

/*
static qi::MetaMethod*             qi_get_convertible_method(qi::ObjectPtr &object, const char *name, PyObject *args, std::vector<qi::MetaMethod> &candidates)
{
  std::string  sig;
  std::string  parameters;
  const qi::MetaObject &meta = object->metaObject();
  std::vector<std::string> sigv;

  // #1 Deduce logical signature from parameters.
  parameters = qi_pyobject_signature(args);

  // #1.2 Concatenate elements.
  sig.append("(");
  sig.append(parameters);
  sig.append(")");

  // #2 Compare with actual choices.
  qi::Signature signature(sig);
  for (std::vector<qi::MetaMethod>::iterator it = candidates.begin(); it != candidates.end(); ++it)
  {
    // #2.1 Get parameter signature.
    sigv = signatureSplit((*it).signature());

    if (signature.isConvertibleTo(sigv[2]) == true)
      return const_cast<qi::MetaMethod*>(meta.method(meta.methodId((*it).signature())));
  }

  qi_raise("_qimessaging.CallError", "Cannot find any suitable method");
  return 0;
}*/

/* qi_gess_method v1
 * Doesn't use qi::GenericValue
 * I'm keeping this until i get all tests ok.
 *
static qi::MetaMethod*             qi_guess_method(qi_object_t *object_c, const char *sig, unsigned int nb_args, PyObject *args, std::vector<qi::MetaMethod> &candidates)
{
  qi::ObjectPtr                    &obj = *(reinterpret_cast<qi::ObjectPtr *>(object_c));
  const qi::MetaObject&            meta = obj->metaObject();
  std::string                      deducedSig, parameters;
  std::vector<std::string>         sigv;

  // #0 Debug log
  qiLogDebug("qimessaging.python.qi_generic_call") << "Still " << candidates.size() << " candidates for " << sig << std::endl;
  for (std::vector<qi::MetaMethod>::iterator it = candidates.begin(); it != candidates.end(); ++it)
    qiLogDebug("qimessaging.python.qi_generic_call") << "\t" << (*it).signature();
  qiLogVerbose("qimessaging.python.qi_generic_call") << "{Performance warning} Desambiguation test for " << sig << std::endl;

  // #1 Deduce logical signature from parameters.
  parameters = qi_pyobject_signature(args);

  // #1.2 Concatenate elements.
  deducedSig.append("(");
  deducedSig.append(parameters);
  deducedSig.append(")");

  // #1.3 Try to find matching method signature.
  for (std::vector<qi::MetaMethod>::iterator it = candidates.begin(); it != candidates.end(); ++it)
  {
    // #1.3.1 Get parameter signature.
    sigv = signatureSplit((*it).signature());

    // #1.3.2 Compare
    if (deducedSig.compare(sigv[2]) == 0)
      return const_cast<qi::MetaMethod*>(meta.method(meta.methodId((*it).signature())));
  }

  // #1.2 If there is no matching method, raise
  if (candidates.size() == 0)
  {
    qi_raise("_qimessaging.CallError", "Disambiguation test failure, No corresponding metamethod, please specify signature");
    return 0;
  }

  return qi_get_convertible_method(obj, sig, args, candidates);
}*/

/*static qi::MetaMethod*      qi_get_method(qi_object_t *object_c, const char *signature, unsigned int nb_args, PyObject *args)
{
  qi::ObjectPtr               obj = *(reinterpret_cast<qi::ObjectPtr *>(object_c));
  std::vector<qi::MetaMethod> mms = obj->metaObject().findCompatibleMethod(signature);

  // If there is no unique compatible method
  if (mms.size() != 1)
  {
    // Code coming straight from AL::ALProxy
    std::stringstream ss;
    std::string type = mms.size() == 0 ? "method" : "overload";

    ss << "Can't find " << type << ": " << signature << std::endl
       << "  Candidate(s):" << std::endl;
    std::vector<qi::MetaMethod>           mml = obj->metaObject().findMethod(qi::signatureSplit(signature)[1]);
    std::vector<qi::MetaMethod>::const_iterator it;
    for (it = mml.begin(); it != mml.end(); ++it) {
      const qi::MetaMethod       &mm = *it;
      ss << "  " << mm.signature() << std::endl;
    }
    qi_raise("qimessaging.callError", ss.str().c_str());
    return 0;
  }

  int methodId = mms[0].uid();
  return const_cast<qi::MetaMethod*>(obj->metaObject().method(methodId));
}*/

/* qi_get_method v1
 * Doesn't use _object->metaObject().findCompatibleMethod(signature);
 * I'm keeping this until i get all tests ok.
 *
static qi::MetaMethod*      qi_get_method(qi_object_t *object_c, const char *signature, unsigned int nb_args, PyObject *args)
{
  std::vector<std::string>  sigInfo;
  qi::ObjectPtr            &obj = *(reinterpret_cast<qi::ObjectPtr *>(object_c));
  const qi::MetaObject     &meta = obj->metaObject();
  const qi::MetaMethod*     mm;
  qi_signature_t           *sig, *subsig;

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
  {
    // #3.1 Get candidates signature.
    sigInfo = signatureSplit((*it).signature());
    // #3.2 Create qi::Signature with it.
    sig = qi_signature_create(sigInfo[2].c_str());
    // #3.3 Create subsignature with previous signature to avoid the tuple where parameters are.
    subsig = qi_signature_create_subsignature(qi_signature_current(sig));

    if (nb_args == (unsigned int) qi_signature_count(subsig))
      candidates.push_back((*it));

    qi_signature_destroy(subsig);
    qi_signature_destroy(sig);
  }

  if (candidates.size() == 1)
    return const_cast<qi::MetaMethod*>(meta.method(meta.methodId(candidates[0].signature())));

  // #3.1 : If no metamethods match method name, raise.
  if (candidates.size() == 0)
  {
    qi_raise("_qimessaging.CallError", "No corresponding metamethod, please specify signature");
    return 0;
  }

  // #4 Guess method from remaining candidates.
  return qi_guess_method(object_c, signature, nb_args, args, candidates);
}*/

/* qi_generic_call v1
 *
 *
PyObject* qi_generic_call(qi_object_t *object_c, char *method_name, PyObject *args)
{
  qi::MetaMethod* mm;
  int nb_args;
  PyObject*       py;

  // Compute number of arguments.
  nb_args = PyTuple_Size(args);

  // Get suitable method.
  if ((mm = qi_get_method(object_c, method_name, nb_args, args)) == NULL)
    return NULL; // Exception should have been raised

  // Create argument qi::Message.
  qi_message_t* msg = qi_message_create();

  // Serialize Python arguments into qi::Message.
  std::vector<std::string> sigv = signatureSplit(mm->signature());
  if (qi_python_to_message(sigv[2].c_str(), msg, args) != 0)
  {
    qiLogError("qimessaging.python.qi_generic_call") << "Cannot convert parameter to qi::message :" << sigv[2] << std::endl;
    qi_message_destroy(msg);
    Py_RETURN_NONE;
  }

  // Call and wait for return value.
  qi_future_t *fut = (qi_future_t *) qi_object_call(object_c, mm->signature().c_str(), msg);
  qi_future_wait(fut);

  // If return value is expected, deserialize qi::Message into Python.
  if (mm->sigreturn().compare("v") != 0)
    py = qi_message_to_python(mm->sigreturn().c_str(), (qi_message_t *) qi_future_get_value(fut));
  else
  {
    // Remove log when it bother us.
    qiLogWarning("qimessaging.python") << "[" << mm->sigreturn() << "] No return value expected, returning None";
    py = Py_None;
    Py_XINCREF(py);
  }

  // Cleanup
  qi_message_destroy((qi_message_t *) qi_future_get_value(fut));
  qi_message_destroy(msg);
  qi_future_destroy(fut);
  return py;
}*/
