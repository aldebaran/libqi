/*
**  Copyright (C) 2013 Aldebaran Robotics
**  See COPYING for the license
*/
#include <utility>
#include <qi/log.hpp>
#include <qitype/signature.hpp>
#include <qitype/anyobject.hpp>
#include <qitype/typedispatcher.hpp>
#include <qitype/typeinterface.hpp>
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


namespace qi { namespace py {
  static LeakStack _leakStack;

  LeakStack& leakStack()
  {
    return _leakStack;
  }

  LeakBlock::LeakBlock()
  {
    leakStack().resize(leakStack().size()+1);
  }
  LeakBlock::~LeakBlock()
  {
    LeakFrame& f = leakStack().back();
    for (unsigned i=0; i<f.size(); ++i)
      f[i].destroy();
    leakStack().pop_back();
  }
  void leakPush(qi::AnyReference ref)
  {
    /* There are cases we cannot catch: call return value handled
    * by serverresult, so outside of python methods scope
    */
    if (leakStack().empty())
      leakStack().resize(1);
    leakStack().back().push_back(ref);
  }
}}

boost::python::object PyObject_from_AnyValue(qi::AnyReference val);
void                PyObject_from_AnyValue(qi::AnyReference val, boost::python::object *target);
qi::AnyReference AnyValue_from_PyObject(PyObject* val);

boost::python::object toO(PyObject*obj) {
  return boost::python::object(boost::python::borrowed(obj));
}


struct ToPyObject
{
  ToPyObject(boost::python::object& result)
    : result(result)
  {
  }

  void visitUnknown(qi::AnyReference value)
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
    if (byteSize == 0)
      result = toO(PyBool_FromLong(static_cast<long>(value)));
    else
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

  void visitList(qi::AnyIterator it, qi::AnyIterator end)
  {
    boost::python::list l;

    for (; it != end; ++it)
    {
      l.append(PyObject_from_AnyValue((*it)));
    }
    result = l;
  }

  void visitMap(qi::AnyIterator it, qi::AnyIterator end)
  {
    boost::python::dict d;
    result = d;

    for (; it != end; ++it)
    {
      d[PyObject_from_AnyValue((*it)[0])] = PyObject_from_AnyValue((*it)[1]);
    }
  }

  void visitObject(qi::GenericObject obj)
  {
    throw std::runtime_error("Error in conversion: Unable to convert object without refcount to Python");
  }

  void visitAnyObject(qi::AnyObject& obj)
  {
    result = qi::py::makePyQiObject(obj);
  }

  void visitPointer(qi::AnyReference)
  {
    throw std::runtime_error("Error in conversion: Unable to convert pointer in Python");
  }

  void visitTuple(const std::string &name, const std::vector<qi::AnyReference>& tuple, const std::vector<std::string>&annotations)
  {
    size_t len = tuple.size();
    boost::python::list l;
    for (size_t i = 0; i < len; i++)
    {
      l.append(PyObject_from_AnyValue(tuple[i]));
    }
    //named tuple
    if (!annotations.size()){
      result = boost::python::tuple(l);
      return;
    }

    static boost::python::object collections = boost::python::import("collections");
    static boost::python::object namedtuple = collections.attr("namedtuple");
    boost::python::object        mytuple;
    boost::python::list          fields;
    for (unsigned int i = 0; i < annotations.size(); ++i) {
      fields.append(annotations.at(i));
    }
    std::string tname = name;
    if (tname.empty())
      tname = "Tuple";
    //verbose=false, rename=true: force invalid field to be renamed
    mytuple = namedtuple(tname, fields, false, true);
    result = mytuple(*boost::python::tuple(l));
  }

  void visitDynamic(qi::AnyReference pointee)
  {
    result = PyObject_from_AnyValue(pointee);
  }

  void visitRaw(qi::AnyReference value)
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

  void visitIterator(qi::AnyReference v)
  {
    visitUnknown(v);
  }

  boost::python::object& result;
};


boost::python::object PyObject_from_AnyValue(qi::AnyReference val)
{
  boost::python::object result;
  ToPyObject tpo(result);
  qi::typeDispatch(tpo, val);
  return result;
}

void PyObject_from_AnyValue(qi::AnyReference val, boost::python::object* target)
{
  ToPyObject tal(*target);
  qi::typeDispatch(tal, val);
}

qi::AnyReference AnyValue_from_PyObject_List(PyObject* val)
{
  std::vector<qi::AnyValue> res;
  Py_ssize_t len = PyList_Size(val);

  for (Py_ssize_t i = 0; i < len; i++)
  {
    PyObject* current = PyList_GetItem(val, i);
    res.push_back(qi::AnyValue(AnyValue_from_PyObject(current)));
  }

  return qi::AnyReference(res).clone();
}

qi::AnyReference AnyValue_from_PyObject_Map(PyObject* dict)
{
  std::map<qi::AnyValue, qi::AnyValue>& res = *new std::map<qi::AnyValue, qi::AnyValue>();
  PyObject *key, *value;
  Py_ssize_t pos = 0;

  while (PyDict_Next(dict, &pos, &key, &value))
  {
    qi::AnyReference newkey = AnyValue_from_PyObject(key);
    qi::AnyReference newvalue = AnyValue_from_PyObject(value);

    res[qi::AnyValue(newkey)] = newvalue;
  }

  return qi::AnyReference(res).clone();
}

qi::AnyReference AnyValue_from_PyObject_Iter(PyObject* iter)
{
  std::vector<qi::AnyReference> res;
  PyObject* item;
  while ((item = PyIter_Next(iter))) {
    qi::AnyReference currentConverted = AnyValue_from_PyObject(item);
    res.push_back(currentConverted);
  }
  return qi::makeGenericTuple(res);
}

qi::AnyReference AnyValue_from_PyObject_Iterable(PyObject* val)
{
  return AnyValue_from_PyObject_Iter(PyObject_GetIter(val));
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



qi::AnyReference AnyValue_from_PyObject(PyObject* val)
{
  qi::AnyReference res;
  // May be not needed but we keep a ref on Py_None for the comparison, better safe than sorry
  PythonScopedRef noneCounter(Py_None);

  try {

    if (PyString_CheckExact(val))
    {
      res = qi::AnyReference(std::string(PyString_AsString(val))).clone();
    }
    else if (PyUnicode_CheckExact(val))
    {
      PyObject *pstring = PyUnicode_AsUTF8String(val);
      res = qi::AnyReference(std::string(PyString_AsString(pstring))).clone();
      Py_DECREF(pstring);
    }
    else if (val == Py_None)
    {
      res = qi::AnyReference(qi::typeOf<void>());
    }
    else if (PyFloat_CheckExact(val))
    {
      res = qi::AnyReference(PyFloat_AsDouble(val)).clone();
    }
    else if (PyLong_CheckExact(val))
    {
      res = qi::AnyReference(PyLong_AsLong(val)).clone();
    }
    else if (PyInt_CheckExact(val))
    {
      res = qi::AnyReference(PyInt_AsLong(val)).clone();
    }
    else if (PyList_CheckExact(val))
    {
      res = AnyValue_from_PyObject_List(val);
    }
    else if (PyDict_CheckExact(val))
    {
      res = AnyValue_from_PyObject_Map(val);
    }
    else if (PyTuple_CheckExact(val) || PyAnySet_CheckExact(val)) // tuple or (frozen) set
    {
      res = AnyValue_from_PyObject_Iterable(val);
    }
    else if (PyBool_Check(val))
    {
      bool b = (PyInt_AsLong(val) != 0);
      res = qi::AnyReference(b).clone();
    }
    // TODO: implement type conversions
    else if (PyByteArray_CheckExact(val))
    {
      res = qi::AnyReference(PyByteArray_AsString(val)).clone();
    }
    else if (val == Py_Ellipsis)
    {
      throw std::runtime_error("Type not implemented");
      res = qi::AnyReference(qi::typeOf<void>());
    }
    else if (PyComplex_CheckExact(val))
    {
      res = qi::AnyReference(PyComplex_RealAsDouble(val)).clone();
    }
    else if (PyBuffer_Check(val))
    {
      throw std::runtime_error("Type not implemented");
      res = qi::AnyReference(qi::typeOf<void>());
    }
    else if (PyMemoryView_Check(val))
    {
      throw std::runtime_error("Type not implemented");
      res = qi::AnyReference(qi::typeOf<void>());
    }
    else if (PyFile_Check(val))
    {
      throw std::runtime_error("Type not implemented");
      res = qi::AnyReference(qi::typeOf<void>());
    }
    else if (PySlice_Check(val))
    {
      throw std::runtime_error("Type not implemented");
      res = qi::AnyReference(qi::typeOf<void>());
    }

    else if (PyModule_CheckExact(val) || PyClass_Check(val)) {
      throw std::runtime_error("Unable to convert Python Module or Class to AnyValue");
    }
    else // if (PyInstance_Check(val))   //instance are old style python class
    {
      res = qi::AnyReference(qi::py::makeQiAnyObject(boost::python::object(boost::python::borrowed(val)))).clone();
    }
  } catch (const boost::python::error_already_set &) {
    throw std::runtime_error("python type conversion failure");
  }
  qi::py::leakPush(res);
  return res;
}

class PyObjectTypeInterface: public qi::DynamicTypeInterface
{
public:
  virtual qi::AnyReference get(void* storage)
  {

    qi::py::GILScopedLock _lock;
    boost::python::object *p = (boost::python::object*) ptrFromStorage(&storage);
    return AnyValue_from_PyObject(p->ptr());
  }
  virtual void set(void** storage, qi::AnyReference src)
  {
    qi::py::GILScopedLock _lock;
    boost::python::object *p = (boost::python::object*) ptrFromStorage(storage);
    PyObject_from_AnyValue(src, p);
  }
  typedef qi::DefaultTypeImplMethods<boost::python::object> Methods;
  _QI_BOUNCE_TYPE_METHODS(Methods);
};

/* Register PyObject* -> See the above comment for explanations */
QI_TYPE_REGISTER_CUSTOM(boost::python::object, PyObjectTypeInterface);

//register for common wrapper too. (yes we could do better than launching the big BFG, but BFG *does* work.
QI_TYPE_REGISTER_CUSTOM(boost::python::str, PyObjectTypeInterface);
QI_TYPE_REGISTER_CUSTOM(boost::python::list, PyObjectTypeInterface);
QI_TYPE_REGISTER_CUSTOM(boost::python::dict, PyObjectTypeInterface);
QI_TYPE_REGISTER_CUSTOM(boost::python::tuple, PyObjectTypeInterface);
