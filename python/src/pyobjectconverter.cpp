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

  void visitUnknown(qi::Type* type, void* storage)
  {
    /* Encapuslate the value in Capsule */
    *result = PyCapsule_New(storage, NULL, NULL);
    checkForError();
  }

  void visitVoid(qi::Type*)
  {
    *result = Py_None;
    checkForError();
  }

  void visitInt(qi::TypeInt* type, qi::int64_t value, bool isSigned, int byteSize)
  {
    *result = PyLong_FromLong(value);
    checkForError();
  }

  void visitFloat(qi::TypeFloat* type, double value, int byteSize)
  {
    *result = PyFloat_FromDouble(value);
    checkForError();
  }

  void visitString(qi::TypeString* type, void* storage)
  {
    if (storage)
    {
      std::pair<const char*, size_t> strSized = type->get(storage);
      *result = PyString_FromStringAndSize(strSized.first, strSized.second);
    }
    else
      *result = PyString_FromString("");
    checkForError();
  }

  void visitList(qi::GenericListPtr value)
  {
    *result = PyList_New(0);
    if (!result)
      throw std::runtime_error("Error in conversion: unable to alloc a python List");

    qi::GenericListIteratorPtr it, end;

    it = value.begin();
    end = value.end();
    for (; it != end; it++)
    {
      PyObject* current = PyObject_from_GenericValue((*it));
      if (PyList_Append(*result, current) != 0)
        throw std::runtime_error("Error in conversion: unable to append element to Python List");
    }
    it.destroy();
    end.destroy();
  }

  void visitMap(qi::GenericMapPtr value)
  {
    *result = PyDict_New();
    if (!result)
      throw std::runtime_error("Error in conversion: unable to alloc a python Dict");

    qi::GenericMapIteratorPtr it, end;

    it = value.begin();
    end = value.end();
    for (; it != end; it++)
    {
      PyObject* key = PyObject_from_GenericValue((*it).first);
      PyObject* value = PyObject_from_GenericValue((*it).second);
      if (PyDict_SetItem(*result, key, value) != 0)
        throw std::runtime_error("Error in conversion: unable to append element to Python List");
    }
    it.destroy();
    end.destroy();
  }

  void visitObject(qi::GenericObject obj)
  {
    /* FIXME: Build a pyObject */
    throw std::runtime_error("Error in conversion: qi::GenericObject not supported yet");
  }

  void visitPointer(qi::TypePointer* type, void* storage, qi::GenericValuePtr pointee)
  {
    throw std::runtime_error("Error in conversion: Unable to convert pointer in Python");
  }

  void visitTuple(qi::TypeTuple* type, void* storage)
  {
    const std::vector<qi::GenericValuePtr>& tuple = type->getValues(storage);
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

  void visitDynamic(qi::Type* type, qi::GenericValuePtr pointee)
  {
    *result = PyObject_from_GenericValue(pointee);
  }

  void visitRaw(qi::TypeRaw *type, qi::Buffer *buf)
  {
    /* Encapuslate the buffer in Capsule */
    *result = PyCapsule_New(buf, "qi::Buffer", NULL);
    checkForError();
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
  qi::typeDispatch(tpo, val.type, &val.value);
  return result;
}

void PyObject_from_GenericValue(qi::GenericValuePtr val, PyObject** target)
{
  ToPyObject tal(target);
  qi::typeDispatch(tal, val.type, &val.value);
}

qi::GenericValuePtr GenericValue_from_PyObject_List(PyObject* val)
{
  std::vector<GenericValuePtr>& res = *new std::vector<GenericValuePtr>();
  Py_ssize_t len = PyList_Size(val);

  for (Py_ssize_t i = 0; i < len; i++)
  {
    PyObject* current = PyList_GetItem(val, i);
    res.push_back(GenericValue_from_PyObject(current));
  }

  return qi::GenericValuePtr::from(res);
}

qi::GenericValuePtr GenericValue_from_PyObject_Map(PyObject* dict)
{
  std::map<qi::GenericValuePtr, qi::GenericValuePtr>& res = *new std::map<qi::GenericValuePtr, qi::GenericValuePtr>();
  PyObject *key, *value;
  Py_ssize_t pos = 0;

  while (PyDict_Next(dict, &pos, &key, &value))
  {
    qi::GenericValuePtr newkey = GenericValue_from_PyObject(key);
    qi::GenericValuePtr newvalue = GenericValue_from_PyObject(value);

    res[newkey] = newvalue;
  }

  return qi::GenericValuePtr::from(res);
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
    res = qi::GenericValuePtr::from(std::string(PyString_AsString(val)));
  }
  else if (val == Py_None)
  {
    res = qi::GenericValuePtr(qi::typeOf<void>());
  }
  else if (PyFloat_CheckExact(val))
  {
    res = qi::GenericValuePtr::from(PyFloat_AsDouble(val));
  }
  else if (PyLong_CheckExact(val))
  {
    res = qi::GenericValuePtr::from(PyLong_AsLong(val));
  }
  else if (PyInt_CheckExact(val))
  {
    res = qi::GenericValuePtr::from(PyInt_AsLong(val));
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
    res = qi::GenericValuePtr::from((bool)PyInt_AsLong(val));
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

  virtual std::pair<qi::GenericValuePtr, bool> get(void* storage)
  {
    qi::GenericValuePtr res = GenericValue_from_PyObject(*((PyObject**)ptrFromStorage(&storage)));

    return std::make_pair(res, true);
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
};

/* Register PyObject* -> See the above comment for explanations */
QI_TYPE_REGISTER_CUSTOM(PyObject*, PyObjectType);
