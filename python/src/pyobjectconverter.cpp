/*
**
** Author(s):
**  - Julien Freche <jfreche@aldebaran-robotics.com>
**
** Copyright (C) 2012 Aldebaran Robotics
*/

#include <utility>

#include <qi/log.hpp>
#include <qitype/signature.hpp>
#include <qitype/genericobject.hpp>
#include <qitype/typedispatcher.hpp>
#include <qitype/type.hpp>

#include <src/pythonscopedref.hpp>

#include "pyobjectconverter.hpp"

using namespace qi;

struct ToPyObject
{
  ToPyObject(PyObject** result)
    : result(result)
  {
  }

  void visitUnknown(GenericValuePtr value)
  {
    /* Encapuslate the value in Capsule */
    *result = PyCapsule_New(value.value, NULL, NULL);
    checkForError();
  }

  void visitVoid()
  {
    *result = Py_None;
    Py_INCREF(Py_None);
    checkForError();
  }

  void visitInt(qi::int64_t value, bool isSigned, int byteSize)
  {
    *result = PyLong_FromLongLong(static_cast<long long>(value));
    checkForError();
  }

  void visitFloat(double value, int byteSize)
  {
    *result = PyFloat_FromDouble(value);
    checkForError();
  }

  void visitString(char* data, size_t len)
  {
    if (data)
    {
      *result = PyString_FromStringAndSize(data, len);
    }
    else
      *result = PyString_FromString("");
    checkForError();
  }

  void visitList(qi::GenericIterator it, qi::GenericIterator end)
  {
    *result = PyList_New(0);
    if (!result)
      throw std::runtime_error("Error in conversion: unable to alloc a python List");

    for (; it != end; ++it)
    {
      PyObject* current = PyObject_from_GenericValue((*it));
      if (PyList_Append(*result, current) != 0)
        throw std::runtime_error("Error in conversion: unable to append element to Python List");
    }
  }

  void visitMap(qi::GenericIterator it, qi::GenericIterator end)
  {
    *result = PyDict_New();
    if (!result)
      throw std::runtime_error("Error in conversion: unable to alloc a python Dict");

    for (; it != end; ++it)
    {
      PyObject* key = PyObject_from_GenericValue((*it)[0]);
      PyObject* value = PyObject_from_GenericValue((*it)[1]);
      if (PyDict_SetItem(*result, key, value) != 0)
        throw std::runtime_error("Error in conversion: unable to append element to Python List");
    }
  }

  void visitObject(qi::GenericObject obj)
  {
    /* FIXME: Build a pyObject */
    throw std::runtime_error("Error in conversion: qi::GenericObject not supported yet");
  }

  void visitPointer(qi::GenericValuePtr)
  {
    throw std::runtime_error("Error in conversion: Unable to convert pointer in Python");
  }

  void visitTuple(const std::vector<qi::GenericValuePtr>& tuple)
  {
    Py_ssize_t len = tuple.size();
    *result = PyTuple_New(len);
    if (!*result)
      throw std::runtime_error("Error in conversion: unable to alloc a python Tuple");

    for (Py_ssize_t i = 0; i < len; i++)
    {
      PyObject* current = PyObject_from_GenericValue(tuple[i]);
      if (PyTuple_SetItem(*result, i, current) != 0)
        throw std::runtime_error("Error in conversion : unable to set item in PyTuple");
    }
  }

  void visitDynamic(qi::GenericValuePtr pointee)
  {
    *result = PyObject_from_GenericValue(pointee);
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
    *result = PyByteArray_FromStringAndSize(b, buf.size());
    free(b);
    checkForError();
  }

  void visitIterator(qi::GenericValuePtr v)
  {
    visitUnknown(v);
  }

  void checkForError()
  {
    if (*result == NULL)
      throw std::runtime_error("Error in conversion to PyObject");
  }

  PyObject** result;
};

PyObject* PyObject_from_GenericValue(qi::GenericValuePtr val)
{
  PyObject* result = NULL;
  ToPyObject tpo(&result);
  qi::typeDispatch(tpo, val);
  return result;
}

void PyObject_from_GenericValue(qi::GenericValuePtr val, PyObject** target)
{
  ToPyObject tal(target);
  qi::typeDispatch(tal, val);
}

qi::GenericValuePtr GenericValue_from_PyObject_List(PyObject* val)
{
  std::vector<GenericValue>& res = *new std::vector<GenericValue>();
  Py_ssize_t len = PyList_Size(val);

  for (Py_ssize_t i = 0; i < len; i++)
  {
    PyObject* current = PyList_GetItem(val, i);
    res.push_back(GenericValue(GenericValue_from_PyObject(current)));
  }

  return qi::GenericValueRef(res);
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

    res[GenericValue(newkey)] = newvalue;
  }

  return qi::GenericValueRef(res);
}

qi::GenericValuePtr GenericValue_from_PyObject_Tuple(PyObject* val)
{
  std::vector<GenericValuePtr>& res = *new std::vector<GenericValuePtr>();
  Py_ssize_t len = PyTuple_Size(val);

  for (Py_ssize_t i = 0; i < len; i++)
  {
    PyObject* current = PyTuple_GetItem(val, i);
    qi::GenericValuePtr currentConverted = GenericValue_from_PyObject(current);
    res.push_back(currentConverted);
  }

  return qi::makeGenericTuple(res);
}

qi::GenericValuePtr GenericValue_from_PyObject(PyObject* val)
{
  qi::GenericValuePtr res;
  // May be not needed but we keep a ref on Py_None for the comparison, better safe than sorry
  PythonScopedRef noneCounter(Py_None);

  if (PyString_CheckExact(val))
  {
    res = qi::GenericValueRef(*new std::string(PyString_AsString(val)));
  }
  else if (PyUnicode_CheckExact(val))
  {
    PyObject *pstring = PyUnicode_AsUTF8String(val);
    res = qi::GenericValueRef(*new std::string(PyString_AsString(pstring)));
    Py_DECREF(pstring);
  }
  else if (val == Py_None)
  {
    res = qi::GenericValuePtr(qi::typeOf<void>());
  }
  else if (PyFloat_CheckExact(val))
  {
    res = qi::GenericValueRef(PyFloat_AsDouble(val));
  }
  else if (PyLong_CheckExact(val))
  {
    res = qi::GenericValueRef(PyLong_AsLong(val));
  }
  else if (PyInt_CheckExact(val))
  {
    res = qi::GenericValueRef(PyInt_AsLong(val));
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
    res = qi::GenericValueRef(b);
  }
  else
  {
    throw std::runtime_error("Unable to convert PyObject in GenericValue");
  }

  return res;
}


/*
 * Define this struct to add PyObject* to the type system.
 * That way we can manipulate PyObject* transparently.
 * - We have to override clone and destroy here to be compliant
 *   with the python reference counting. Otherwise, the value could
 *   be Garbage Collected as we try to manipulate it.
 * - We register the type as 'PyObject*' since python methods manipulates
 *   objects only by pointer, never by value and we do not want to copy
 *   a PyObject.
 */
class PyObjectType: public qi::TypeDynamic
{
public:

  virtual const TypeInfo& info()
  {
    static TypeInfo* result = 0;
    if (!result)
      result = new TypeInfo(typeid(PyObject*));
    return *result;
  }

  virtual void* initializeStorage(void* ptr = 0)
  {
    void** myptr = static_cast<void**>(ptr);
    if (myptr)
      return *myptr;

    return 0;
  }

  virtual void* ptrFromStorage(void** s)
  {
    return s;
  }

  virtual qi::GenericValuePtr get(void* storage)
  {
    return GenericValue_from_PyObject(*((PyObject**)ptrFromStorage(&storage)));
  }

  virtual void set(void** storage, qi::GenericValuePtr src)
  {
    PyObject** target = (PyObject**)ptrFromStorage(storage);
    PyObject_from_GenericValue(src, target);

    /* Increment the ref counter since we now store the PyObject */
    Py_XINCREF(*target);
  }

  virtual void* clone(void* obj)
  {
    Py_XINCREF((PyObject*)obj);
    return obj;
  }

  virtual void destroy(void* obj)
  {
    Py_XDECREF((PyObject*)obj);
  }
  virtual bool less(void* a, void* b)
  {
    PyObject** pa = (PyObject**)ptrFromStorage(&a);
    PyObject** pb = (PyObject**)ptrFromStorage(&b);
    int res = PyObject_Compare(*pa, *pb);
    if (PyErr_Occurred())
      return *pa < *pb;
    else
      return res < 0;
  }
};

/* Register PyObject* -> See the above comment for explanations */
QI_TYPE_REGISTER_CUSTOM(PyObject*, PyObjectType);
