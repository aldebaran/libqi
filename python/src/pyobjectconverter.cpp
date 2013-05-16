/*
**
** Author(s):
**  - Julien Freche <jfreche@aldebaran-robotics.com>
**
** Copyright (C) 2012, 2013 Aldebaran Robotics
*/

#include <utility>
#include <qi/log.hpp>
#include <qitype/signature.hpp>
#include <qitype/genericobject.hpp>
#include <qitype/typedispatcher.hpp>
#include <qitype/type.hpp>
#include <qitype/metaobject.hpp>
#include <qitype/metamethod.hpp>
#include <boost/python.hpp>
#include "pyobject.hpp"
#include "pysignal.hpp"
#include "pyproperty.hpp"

#include  <boost/python.hpp>
#include <boost/python/raw_function.hpp>
#include "gil.hpp"

qiLogCategory("qipy.convert");

boost::python::object PyObject_from_GenericValue(qi::GenericValuePtr val);
void                PyObject_from_GenericValue(qi::GenericValuePtr val, boost::python::object *target);
qi::GenericValuePtr GenericValue_from_PyObject(PyObject* val);

boost::python::object toO(PyObject*obj) {
  return boost::python::object(boost::python::handle<>(obj));
}


struct ToPyObject
{
  ToPyObject(boost::python::object& result)
    : result(result)
  {
  }

  void visitUnknown(qi::GenericValuePtr value)
  {
    /* Encapuslate the value in Capsule */
    result = toO(PyCapsule_New(value.value, NULL, NULL));
  }

  void visitVoid()
  {
    //return None
    result = boost::python::object();
  }

  void visitInt(qi::int64_t value, bool isSigned, int byteSize)
  {
    result = toO(PyLong_FromLongLong(static_cast<long long>(value)));
  }

  void visitFloat(double value, int byteSize)
  {
    result = toO(PyFloat_FromDouble(value));
  }

  void visitString(char* data, size_t len)
  {
    if (data)
      result = toO(PyString_FromStringAndSize(data, len));
    else
      result = toO(PyString_FromString(""));
  }

  void visitList(qi::GenericIterator it, qi::GenericIterator end)
  {
    boost::python::list l;

    for (; it != end; ++it)
    {
      l.append(PyObject_from_GenericValue((*it)));
    }
    result = l;
  }

  void visitMap(qi::GenericIterator it, qi::GenericIterator end)
  {
    boost::python::dict d;
    result = d;

    for (; it != end; ++it)
    {
      d[PyObject_from_GenericValue((*it)[0])] = PyObject_from_GenericValue((*it)[1]);
    }
  }

  void visitObject(qi::GenericObject obj)
  {
    throw std::runtime_error("Error in conversion: Unable to convert object without refcount to Python");
  }

  void visitObjectPtr(qi::ObjectPtr& obj)
  {
    result = qi::py::makePyQiObject(obj);
  }

  void visitPointer(qi::GenericValuePtr)
  {
    throw std::runtime_error("Error in conversion: Unable to convert pointer in Python");
  }

  void visitTuple(const std::string &name, const std::vector<qi::GenericValuePtr>& tuple, const std::vector<std::string>&annotations)
  {
    size_t len = tuple.size();
    boost::python::list l;
    for (size_t i = 0; i < len; i++)
    {
      l.append(PyObject_from_GenericValue(tuple[i]));
    }
    result = boost::python::tuple(l);
  }

  void visitDynamic(qi::GenericValuePtr pointee)
  {
    result = PyObject_from_GenericValue(pointee);
  }

  void visitRaw(qi::GenericValuePtr value)
  {
    /* TODO: zerocopy, sub-buffers... */
    qi::Buffer& buf = value.as<qi::Buffer>();
    if (buf.subBuffers().size() != 0)
    {
      qiLogError("pyobjectconverter") << "buffer has subbuffers, "
                                      << "Python bytearray might be incomplete";
    }

    char* b = static_cast<char*>(malloc(buf.size()));
    buf.read(b, 0, buf.size());
    result = toO(PyByteArray_FromStringAndSize(b, buf.size()));
    free(b);
  }

  void visitIterator(qi::GenericValuePtr v)
  {
    visitUnknown(v);
  }

  boost::python::object& result;
};


boost::python::object PyObject_from_GenericValue(qi::GenericValuePtr val)
{
  boost::python::object result;
  ToPyObject tpo(result);
  qi::typeDispatch(tpo, val);
  return result;
}

void PyObject_from_GenericValue(qi::GenericValuePtr val, boost::python::object* target)
{
  ToPyObject tal(*target);
  qi::typeDispatch(tal, val);
}

qi::GenericValuePtr GenericValue_from_PyObject_List(PyObject* val)
{
  std::vector<qi::GenericValue> res;
  Py_ssize_t len = PyList_Size(val);

  for (Py_ssize_t i = 0; i < len; i++)
  {
    PyObject* current = PyList_GetItem(val, i);
    res.push_back(qi::GenericValue(GenericValue_from_PyObject(current)));
  }

  return qi::GenericValueRef(res).clone();
}

qi::GenericValuePtr GenericValue_from_PyObject_Map(PyObject* dict)
{
  std::map<qi::GenericValue, qi::GenericValue>& res = *new std::map<qi::GenericValue, qi::GenericValue>();
  PyObject *key, *value;
  Py_ssize_t pos = 0;

  while (PyDict_Next(dict, &pos, &key, &value))
  {
    qi::GenericValuePtr newkey = GenericValue_from_PyObject(key);
    qi::GenericValuePtr newvalue = GenericValue_from_PyObject(value);

    res[qi::GenericValue(newkey)] = newvalue;
  }

  return qi::GenericValueRef(res).clone();
}

qi::GenericValuePtr GenericValue_from_PyObject_Tuple(PyObject* val)
{
  std::vector<qi::GenericValuePtr> res;
  Py_ssize_t len = PyTuple_Size(val);

  for (Py_ssize_t i = 0; i < len; i++)
  {
    PyObject* current = PyTuple_GetItem(val, i);
    qi::GenericValuePtr currentConverted = GenericValue_from_PyObject(current);
    res.push_back(currentConverted);
  }

  return qi::makeGenericTuple(res);
}

class PythonScopedRef
{
  public:
    PythonScopedRef(PyObject* p)
      : _p (p) {
      Py_XINCREF(_p);
    }
    ~PythonScopedRef() {
      Py_XDECREF(_p);
    }
  private:
    PyObject* _p;
};

qi::GenericValuePtr GenericValue_from_PyObject(PyObject* val)
{
  qi::GenericValuePtr res;
  // May be not needed but we keep a ref on Py_None for the comparison, better safe than sorry
  PythonScopedRef noneCounter(Py_None);

  if (PyString_CheckExact(val))
  {
    res = qi::GenericValueRef(std::string(PyString_AsString(val))).clone();
  }
  else if (PyUnicode_CheckExact(val))
  {
    PyObject *pstring = PyUnicode_AsUTF8String(val);
    res = qi::GenericValueRef(std::string(PyString_AsString(pstring))).clone();
    Py_DECREF(pstring);
  }
  else if (val == Py_None)
  {
    res = qi::GenericValuePtr(qi::typeOf<void>());
  }
  else if (PyFloat_CheckExact(val))
  {
    res = qi::GenericValueRef(PyFloat_AsDouble(val)).clone();
  }
  else if (PyLong_CheckExact(val))
  {
    res = qi::GenericValueRef(PyLong_AsLong(val)).clone();
  }
  else if (PyInt_CheckExact(val))
  {
    res = qi::GenericValueRef(PyInt_AsLong(val)).clone();
  }
  else if (PyList_CheckExact(val))
  {
    res = GenericValue_from_PyObject_List(val);
  }
  else if (PyDict_CheckExact(val))
  {
    res = GenericValue_from_PyObject_Map(val);
  }
  else if (PyTuple_CheckExact(val))
  {
    res = GenericValue_from_PyObject_Tuple(val);
  }
  else if (PyBool_Check(val))
  {
    bool b = (PyInt_AsLong(val) != 0);
    res = qi::GenericValueRef(b).clone();
  }
  else if (PyModule_CheckExact(val) || PyClass_Check(val)) {
    throw std::runtime_error("Unable to convert Python Module or Class to GenericValue");
  }
  else // if (PyInstance_Check(val))   //instance are old style python class
  {
    res = qi::GenericValueRef(qi::py::makeQiObjectPtr(boost::python::object(boost::python::borrowed(val)))).clone();
  }
  return res;
}

class PyObjectType: public qi::TypeDynamic
{
public:
  virtual qi::GenericValuePtr get(void* storage)
  {
    boost::python::object *p = (boost::python::object*) ptrFromStorage(&storage);
    return GenericValue_from_PyObject(p->ptr());
  }
  virtual void set(void** storage, qi::GenericValuePtr src)
  {
    boost::python::object *p = (boost::python::object*) ptrFromStorage(storage);
    PyObject_from_GenericValue(src, p);
  }
  typedef qi::DefaultTypeImplMethods<boost::python::object> Methods;
  _QI_BOUNCE_TYPE_METHODS(Methods);
};

/* Register PyObject* -> See the above comment for explanations */
QI_TYPE_REGISTER_CUSTOM(boost::python::object, PyObjectType);

//register for common wrapper too. (yes we could do better than launching the big BFG, but BFG *does* work.
QI_TYPE_REGISTER_CUSTOM(boost::python::str, PyObjectType);
QI_TYPE_REGISTER_CUSTOM(boost::python::list, PyObjectType);
QI_TYPE_REGISTER_CUSTOM(boost::python::dict, PyObjectType);
QI_TYPE_REGISTER_CUSTOM(boost::python::tuple, PyObjectType);
