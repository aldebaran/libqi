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

boost::python::object PyObject_from_AnyValue(qi::AnyReference val);
void PyObject_from_AnyValue(qi::AnyReference val,
    boost::python::object *target);

/**
 * Convert the PyObject* to boost::python::object assuming you own the
 * reference.
 *
 * This means that the refcounter has already been incremented for you (methods
 * that return "New reference" in Python doc).
 */
boost::python::object pyHandle(PyObject* obj) {
  return boost::python::object(boost::python::handle<>(obj));
}

/**
 * Convert the PyObject* to boost::python::object assuming you do *not* own the
 * reference.
 *
 * This means that the refcounter has *not* been incremented for you (methods
 * that return "Borrowed reference" in Python doc).
 */
boost::python::object pyBorrow(PyObject* obj) {
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
    result = pyHandle(PyCapsule_New(value.rawValue(), NULL, NULL));
  }

  void visitVoid()
  {
    //return None
    result = boost::python::object();
  }

  void visitInt(qi::int64_t value, bool isSigned, int byteSize)
  {
    if (byteSize == 0)
      result = pyHandle(PyBool_FromLong(static_cast<long>(value)));
    else
      result = pyHandle(PyLong_FromLongLong(static_cast<long long>(value)));
  }

  void visitFloat(double value, int byteSize)
  {
    result = pyHandle(PyFloat_FromDouble(value));
  }

  void visitString(char* data, size_t len)
  {
    if (data)
      result = pyHandle(PyString_FromStringAndSize(data, len));
    else
      result = pyHandle(PyString_FromString(""));
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
    result = pyHandle(PyByteArray_FromStringAndSize(b, buf.size()));
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

qi::AnyReference AnyReference_from_BoostObject(boost::python::object* obj);
qi::AnyReference AnyReference_from_PyObject(PyObject* obj);

typedef std::map<PyObject*, qi::AnyReference> LeakMap;
static LeakMap leakMap;
static boost::mutex leakMutex;

void cleanup_ref(PyObject* ref)
{
  boost::mutex::scoped_lock _lock(leakMutex);
  LeakMap::iterator it = leakMap.find(ref);
  if (it != leakMap.end())
  {
    it->second.destroy();
    leakMap.erase(it);
    Py_DECREF(ref);
  }
}

/**
 * We reimplement some primitives here, to manipulate PyObjects under the GIL.
 *
 * Note that this class type-erases PyObject, not PyObject*, so it does not
 * have pointer semantics.
 */
template <typename Base>
class PyObjectTypeInterface: public Base
{
public:
  virtual void* initializeStorage(void* ptr = 0)
  {
    if (ptr)
      return ptr;
    else
      throw std::runtime_error("can't initialize a PyObject out of nowhere");
  }

  virtual void* clone(void* storage)
  {
    // We need deepcopy here to have clone semantics. Not all PyObjects are
    // immutable (you can add elements to lists and dicts), so we need a copy
    // here and not an INCREF().
    qi::py::GILScopedLock _lock;
    static boost::python::object copyModule;
    if (copyModule.ptr() == Py_None)
      copyModule = pyHandle(PyImport_ImportModule("copy"));

    PyObject* p = (PyObject*)ptrFromStorage(&storage);
    boost::python::object ret = copyModule.attr("deepcopy")(pyBorrow(p));
    Py_XINCREF(ret.ptr());
    return ret.ptr();
  }

  virtual void destroy(void* storage)
  {
    qi::py::GILScopedLock _lock;
    PyObject* p = (PyObject*)ptrFromStorage(&storage);
    Py_XDECREF(p);
  }

  typedef qi::DefaultTypeImplMethods<PyObject> Methods;

  virtual const ::qi::TypeInfo& info()  { return Methods::info(); }
  virtual void* ptrFromStorage(void**s) { return Methods::ptrFromStorage(s); }
  virtual bool  less(void* a, void* b)  { return Methods::less(a, b); }
};

#define PYTYPE_GETSET(ctype, pyctype, pytype) \
  PYTYPE_GET(ctype, pyctype, pytype) \
  PYTYPE_SET(ctype, pyctype, pytype)

#define PYTYPE_GET(ctype, pyctype, pytype) \
  virtual ctype get(void* storage) \
  { \
    qi::py::GILScopedLock _lock; \
    PyObject* p = (PyObject*)PyObjectTypeInterface::ptrFromStorage(&storage); \
    return Py##pytype##_As##pyctype(p); \
  }
#define PYTYPE_SET(ctype, pyctype, pytype) \
  virtual void set(void** storage, ctype value) \
  { \
    throw std::runtime_error("cannot update python " #pytype); \
  }
#define PYTYPE_GETTYPE(Cl) \
  static Cl* getType() \
  { \
    static Cl typeInterface; \
    return &typeInterface; \
  }

// Interfaces for specialized types

class PyObjectIntTypeInterface: public PyObjectTypeInterface<qi::IntTypeInterface>
{
public:
  PYTYPE_GETSET(int64_t, Long, Int)
  virtual unsigned int size() { return sizeof(long); }
  virtual bool isSigned() { return true; }
  PYTYPE_GETTYPE(PyObjectIntTypeInterface)
};

class PyObjectLongTypeInterface: public PyObjectTypeInterface<qi::IntTypeInterface>
{
public:
  PYTYPE_GETSET(int64_t, LongLong, Long)
  virtual unsigned int size() { return sizeof(long); }
  virtual bool isSigned() { return true; }
  PYTYPE_GETTYPE(PyObjectLongTypeInterface)
};

class PyObjectFloatTypeInterface: public PyObjectTypeInterface<qi::FloatTypeInterface>
{
public:
  PYTYPE_GETSET(double, Double, Float)
  virtual unsigned int size() { return sizeof(double); }
  PYTYPE_GETTYPE(PyObjectFloatTypeInterface)
};

class PyObjectBoolTypeInterface: public PyObjectTypeInterface<qi::IntTypeInterface>
{
public:
  PYTYPE_GETSET(int64_t, Long, Int)
  virtual unsigned int size() { return 0; }
  virtual bool isSigned() { return false; }
  PYTYPE_GETTYPE(PyObjectBoolTypeInterface)
};

/////////////////////////////////////////////////////////////////////////
// String
/////////////////////////////////////////////////////////////////////////

class PyObjectStringTypeInterface: public PyObjectTypeInterface<qi::StringTypeInterface>
{
public:
  /// The returned buffer must not be modified
  virtual ManagedRawString get(void* storage);
  virtual void set(void** storage, const char* ptr, size_t sz);
  PYTYPE_GETTYPE(PyObjectStringTypeInterface)
};

PyObjectStringTypeInterface::ManagedRawString
  PyObjectStringTypeInterface::get(void* storage)
{
  qi::py::GILScopedLock _lock;
  PyObject* p = (PyObject*)PyObjectTypeInterface::ptrFromStorage(&storage);
  return ManagedRawString(RawString(PyString_AsString(p), PyString_Size(p)),
      Deleter());
}

void PyObjectStringTypeInterface::set(void** storage, const char* ptr, size_t sz)
{
  throw std::runtime_error("cannot update python string");
}

/////////////////////////////////////////////////////////////////////////
// Unicode
/////////////////////////////////////////////////////////////////////////

class PyObjectUnicodeStringTypeInterface: public PyObjectTypeInterface<qi::StringTypeInterface>
{
public:
  /// The returned buffer must not be modified
  virtual ManagedRawString get(void* storage);
  virtual void set(void** storage, const char* ptr, size_t sz);
  PYTYPE_GETTYPE(PyObjectUnicodeStringTypeInterface)
};

namespace
{
  void PyObjectDeleter(PyObject* obj)
  {
    Py_DECREF(obj);
  }
}

PyObjectUnicodeStringTypeInterface::ManagedRawString
  PyObjectUnicodeStringTypeInterface::get(void* storage)
{
  qi::py::GILScopedLock _lock;
  PyObject* p = (PyObject*)PyObjectTypeInterface::ptrFromStorage(&storage);
  PyObject* p2 = PyUnicode_AsUTF8String(p);
  return ManagedRawString(
      RawString(reinterpret_cast<char*>(PyString_AsString(p2)),
        PyString_Size(p2)),
      boost::bind(PyObjectDeleter, p2));
}

void PyObjectUnicodeStringTypeInterface::set(void** storage, const char* ptr, size_t sz)
{
  throw std::runtime_error("cannot update python string");
}

/////////////////////////////////////////////////////////////////////////
// ByteArray
/////////////////////////////////////////////////////////////////////////

class PyObjectByteArrayTypeInterface: public PyObjectTypeInterface<qi::StringTypeInterface>
{
public:
  virtual ManagedRawString get(void* storage);
  virtual void set(void** storage, const char* ptr, size_t sz);
  PYTYPE_GETTYPE(PyObjectByteArrayTypeInterface)
};

PyObjectUnicodeStringTypeInterface::ManagedRawString
  PyObjectByteArrayTypeInterface::get(void* storage)
{
  qi::py::GILScopedLock _lock;
  PyObject* p = (PyObject*)PyObjectTypeInterface::ptrFromStorage(&storage);
  return ManagedRawString(
      RawString(PyByteArray_AsString(p), PyByteArray_Size(p)), Deleter());
}

void PyObjectByteArrayTypeInterface::set(void** storage, const char* ptr, size_t sz)
{
  throw std::runtime_error("cannot update python String");
}

/////////////////////////////////////////////////////////////////////////
// List, Dict, Dynamic
/////////////////////////////////////////////////////////////////////////

class PyObjectListIteratorTypeInterface : public qi::IteratorTypeInterface
{
public:
  struct Iter
  {
    void* storage;
    unsigned int element;

    Iter() : storage(0), element(0) {}
    Iter(void* container, unsigned int elem = 0) :
      storage(container), element(elem)
    {}
    Iter(const Iter& o) :
      storage(o.storage),
      element(o.element)
    {}
    Iter& operator=(const Iter& o)
    {
      storage = o.storage;
      element = o.element;
      return *this;
    }

    bool operator==(const Iter& i)
    {
      return storage == i.storage && element == i.element;
    }
  };

  virtual qi::AnyReference dereference(void* storage);
  virtual void next(void** storage);
  virtual bool equals(void* s1, void* s2);

  virtual void* initializeStorage(void* ptr=0);
  virtual void* clone(void* storage);
  virtual void destroy(void* storage);

  typedef qi::DefaultTypeImplMethods<Iter> Methods;

  virtual const ::qi::TypeInfo& info()  { return Methods::info(); }
  virtual void* ptrFromStorage(void**s) { return Methods::ptrFromStorage(s); }
  virtual bool  less(void* a, void* b)  { return Methods::less(a, b); }

  PYTYPE_GETTYPE(PyObjectListIteratorTypeInterface)
};

class PyObjectListTypeInterface: public PyObjectTypeInterface<qi::ListTypeInterface>
{
public:
  virtual TypeInterface* elementType();
  virtual size_t size(void* storage);
  virtual qi::AnyIterator begin(void* storage);
  virtual qi::AnyIterator end(void* storage);
  virtual void pushBack(void** storage, void* valueStorage);
  virtual void* element(void* storage, int index);

  PYTYPE_GETTYPE(PyObjectListTypeInterface)
};

class PyObjectDictIteratorTypeInterface : public qi::IteratorTypeInterface
{
public:
  struct Iter
  {
    PyObject* container;
    Py_ssize_t ppos; // -1 means end iterator
    std::pair<boost::python::object, boost::python::object> curPair;

    Iter() : container(0), ppos(0) {}
    Iter(const Iter& o) :
      container(o.container),
      ppos(o.ppos),
      curPair(o.curPair)
    {
      qi::py::GILScopedLock _lock;
      Py_XINCREF(container);
    }
    Iter(PyObject* cont, bool end = false) :
      container(cont),
      ppos(end ? -1 : 0)
    {
      qi::py::GILScopedLock _lock;
      Py_XINCREF(container);
    }
    ~Iter()
    {
      qi::py::GILScopedLock _lock;
      Py_XDECREF(container);
    }
    Iter& operator=(const Iter& o)
    {
      if (this != &o)
      {
        qi::py::GILScopedLock _lock;
        Py_XDECREF(container);

        container = o.container;
        ppos = o.ppos;
        curPair = o.curPair;

        Py_XINCREF(container);
      }
      return *this;
    }

    bool operator==(const Iter& i)
    {
      return container == i.container && ppos == i.ppos;
    }
  };

  virtual qi::AnyReference dereference(void* storage);
  virtual void next(void** storage);
  virtual bool equals(void* s1, void* s2);

  virtual void* initializeStorage(void* ptr=0);
  virtual void* clone(void* storage);
  virtual void destroy(void* storage);

  typedef qi::DefaultTypeImplMethods<Iter> Methods;

  virtual const ::qi::TypeInfo& info()  { return Methods::info(); }
  virtual void* ptrFromStorage(void**s) { return Methods::ptrFromStorage(s); }
  virtual bool  less(void* a, void* b)  { return Methods::less(a, b); }

  PYTYPE_GETTYPE(PyObjectDictIteratorTypeInterface)
};

class PyObjectDictTypeInterface: public PyObjectTypeInterface<qi::MapTypeInterface>
{
public:
  virtual TypeInterface* elementType();
  virtual TypeInterface* keyType();
  virtual size_t size(void* storage);
  virtual qi::AnyIterator begin(void* storage);
  virtual qi::AnyIterator end(void* storage);
  virtual void insert(void** storage, void* keyStorage, void* valueStorage);
  virtual qi::AnyReference element(void** storage, void* keyStorage, bool autoInsert);

  PYTYPE_GETTYPE(PyObjectDictTypeInterface)
};

class PyObjectIterableTypeInterface: public PyObjectTypeInterface<qi::StructTypeInterface>
{
public:
  PyObjectIterableTypeInterface(unsigned int size) : _size(size) {}

  virtual std::vector<TypeInterface*> memberTypes();
  virtual std::vector<void*> get(void* storage);
  virtual void* get(void* storage, unsigned int index);
  virtual void set(void** storage, const std::vector<void*>&);
  virtual void set(void** storage, unsigned int index, void* valStorage);

  static TypeInterface* getType(unsigned int size)
  {
    static std::map<unsigned int, TypeInterface*>* typeInterfaces = 0;
    static boost::mutex* mutex = 0;
    QI_THREADSAFE_NEW(mutex, typeInterfaces);
    boost::mutex::scoped_lock _lock(*mutex);
    TypeInterface*& iface = (*typeInterfaces)[size];
    if (!iface)
      iface = new PyObjectIterableTypeInterface(size);
    return iface;
  }

private:
  unsigned int _size;
};

class PyObjectDynamicTypeInterface: public PyObjectTypeInterface<qi::DynamicTypeInterface>
{
public:
  virtual qi::AnyReference get(void* storage)
  {
    qi::py::GILScopedLock _lock;
    PyObject* p = (PyObject*)PyObjectTypeInterface::ptrFromStorage(&storage);
    return AnyReference_from_PyObject(p);
  }

  virtual void set(void** storage, qi::AnyReference src)
  {
    throw std::runtime_error("cannot update python Object");
  }

  PYTYPE_GETTYPE(PyObjectDynamicTypeInterface)
};

class PyBoostObjectDynamicTypeInterface: public qi::DynamicTypeInterface
{
public:
  virtual void* initializeStorage(void* ptr = 0)
  {
    if (ptr)
      return ptr;
    else
    {
      qi::py::GILScopedLock _lock;
      return new boost::python::object();
    }
  }

  virtual void* clone(void* storage)
  {
    qi::py::GILScopedLock _lock;
    boost::python::object* p = (boost::python::object*)ptrFromStorage(&storage);
    boost::python::object* o = new boost::python::object();
    *o = *p;
    return o;
  }

  virtual void destroy(void* storage)
  {
    qi::py::GILScopedLock _lock;
    boost::python::object* p = (boost::python::object*)ptrFromStorage(&storage);
    delete p;
  }

  virtual qi::AnyReference get(void* storage)
  {
    qi::py::GILScopedLock _lock;
    boost::python::object* p = (boost::python::object*)ptrFromStorage(&storage);
    return AnyReference_from_PyObject(p->ptr());
  }

  virtual void set(void** storage, qi::AnyReference src)
  {
    qi::py::GILScopedLock _lock;
    boost::python::object* p = (boost::python::object*)ptrFromStorage(storage);
    PyObject_from_AnyValue(src, p);
  }

  typedef qi::DefaultTypeImplMethods<boost::python::object> Methods;

  virtual const ::qi::TypeInfo& info()  { return Methods::info(); }
  virtual void* ptrFromStorage(void**s) { return Methods::ptrFromStorage(s); }
  virtual bool  less(void* a, void* b)  { return Methods::less(a, b); }

  PYTYPE_GETTYPE(PyBoostObjectDynamicTypeInterface)
};

// List implementation

qi::TypeInterface* PyObjectListTypeInterface::elementType()
{
  return PyObjectDynamicTypeInterface::getType();
}

size_t PyObjectListTypeInterface::size(void* storage)
{
  qi::py::GILScopedLock _lock;
  PyObject* p = (PyObject*)PyObjectTypeInterface::ptrFromStorage(&storage);
  return PyList_Size(p);
}

qi::AnyIterator PyObjectListTypeInterface::begin(void* storage)
{
  return qi::AnyReference(PyObjectListIteratorTypeInterface::getType(), new PyObjectListIteratorTypeInterface::Iter(storage));
}

qi::AnyIterator PyObjectListTypeInterface::end(void* storage)
{
  return qi::AnyReference(PyObjectListIteratorTypeInterface::getType(), new PyObjectListIteratorTypeInterface::Iter(storage, size(storage)));
}

void PyObjectListTypeInterface::pushBack(void** storage, void* valueStorage)
{
  qi::py::GILScopedLock _lock;
  PyObject* p = (PyObject*)PyObjectTypeInterface::ptrFromStorage(storage);
  PyObject* elem = (PyObject*)PyObjectTypeInterface::ptrFromStorage(&valueStorage);
  if (PyList_Append(p, elem) == -1)
  {
    PyErr_Clear();
    throw std::runtime_error("error during pushBack on Python list");
  }
}

void* PyObjectListTypeInterface::element(void* storage, int index)
{
  qi::py::GILScopedLock _lock;
  PyObject* p = (PyObject*)PyObjectTypeInterface::ptrFromStorage(&storage);
  return PyList_GetItem(p, index);
}

// List iterator implementation

qi::AnyReference PyObjectListIteratorTypeInterface::dereference(void* storage)
{
  Iter* ptr = (Iter*) ptrFromStorage(&storage);
  return qi::AnyReference(PyObjectDynamicTypeInterface::getType(), PyObjectListTypeInterface::getType()->element(ptr->storage, ptr->element));
}

void PyObjectListIteratorTypeInterface::next(void** storage)
{
  Iter* ptr = (Iter*)ptrFromStorage(storage);
  ++ptr->element;
}

bool PyObjectListIteratorTypeInterface::equals(void* s1, void* s2)
{
  Iter* p1 = (Iter*)ptrFromStorage(&s1);
  Iter* p2 = (Iter*)ptrFromStorage(&s2);
  return *p1 == *p2;
}

void* PyObjectListIteratorTypeInterface::initializeStorage(void* ptr)
{
  if (ptr)
    return ptr;
  else
    return new Iter;
}

void* PyObjectListIteratorTypeInterface::clone(void* storage)
{
  Iter* p = (Iter*)ptrFromStorage(&storage);
  return new Iter(*p);
}

void PyObjectListIteratorTypeInterface::destroy(void* storage)
{
  Iter* p = (Iter*)ptrFromStorage(&storage);
  delete p;
}

// Dict implementation

qi::TypeInterface* PyObjectDictTypeInterface::elementType()
{
  return PyObjectDynamicTypeInterface::getType();
}

qi::TypeInterface* PyObjectDictTypeInterface::keyType()
{
  return PyObjectDynamicTypeInterface::getType();
}

size_t PyObjectDictTypeInterface::size(void* storage)
{
  qi::py::GILScopedLock _lock;
  PyObject* p = (PyObject*)PyObjectTypeInterface::ptrFromStorage(&storage);
  return PyDict_Size(p);
}

qi::AnyIterator PyObjectDictTypeInterface::begin(void* storage)
{
  qi::py::GILScopedLock _lock;
  PyObject* p = (PyObject*)PyObjectTypeInterface::ptrFromStorage(&storage);
  PyObjectDictIteratorTypeInterface* typeInterface = PyObjectDictIteratorTypeInterface::getType();
  void* iter = new PyObjectDictIteratorTypeInterface::Iter(p);
  typeInterface->next(&iter);
  return qi::AnyReference(typeInterface, iter);
}

qi::AnyIterator PyObjectDictTypeInterface::end(void* storage)
{
  qi::py::GILScopedLock _lock;
  PyObject* p = (PyObject*)PyObjectTypeInterface::ptrFromStorage(&storage);
  return qi::AnyReference(PyObjectDictIteratorTypeInterface::getType(), new PyObjectDictIteratorTypeInterface::Iter(p, true));
}

void PyObjectDictTypeInterface::insert(void** storage, void* keyStorage, void* valueStorage)
{
  qi::py::GILScopedLock _lock;
  PyObject* p = (PyObject*)PyObjectTypeInterface::ptrFromStorage(storage);
  PyObject* key = (PyObject*)PyObjectTypeInterface::ptrFromStorage(&keyStorage);
  PyObject* value = (PyObject*)PyObjectTypeInterface::ptrFromStorage(&valueStorage);
  PyDict_SetItem(p, key, value);
}

qi::AnyReference PyObjectDictTypeInterface::element(void** storage, void* keyStorage, bool autoInsert)
{
  qi::py::GILScopedLock _lock;
  PyObject* p = (PyObject*)PyObjectTypeInterface::ptrFromStorage(storage);
  PyObject* key = (PyObject*)PyObjectTypeInterface::ptrFromStorage(&keyStorage);
  PyObject* value = PyDict_GetItem(p, key);
  if (!value)
  {
    if (!autoInsert)
      return qi::AnyReference();
    PyDict_SetItem(p, key, Py_None);
    value = Py_None;
  }
  return qi::AnyReference::from(value);
}

// Dict iterator implementation

qi::AnyReference PyObjectDictIteratorTypeInterface::dereference(void* storage)
{
  Iter* ptr = (Iter*)ptrFromStorage(&storage);
  assert(ptr->ppos != -1 && "attempting to dereference past-the-end iterator");
  return qi::AnyReference::from(ptr->curPair);
}

void PyObjectDictIteratorTypeInterface::next(void** storage)
{
  Iter* ptr = (Iter*) ptrFromStorage(storage);
  if (ptr->ppos == -1)
    return;

  qi::py::GILScopedLock _lock;
  PyObject* curKey;
  PyObject* curValue;
  if (!PyDict_Next(ptr->container, &ptr->ppos, &curKey, &curValue))
    // past the end
    ptr->ppos = -1;
  else
    ptr->curPair = std::make_pair(pyBorrow(curKey), pyBorrow(curValue));
}

bool PyObjectDictIteratorTypeInterface::equals(void* s1, void* s2)
{
  Iter* p1 = (Iter*)ptrFromStorage(&s1);
  Iter* p2 = (Iter*)ptrFromStorage(&s2);
  return *p1 == *p2;
}

void* PyObjectDictIteratorTypeInterface::initializeStorage(void* ptr)
{
  Iter* iter = new Iter;
  if (ptr)
  {
    Iter* iter2 = (Iter*)ptrFromStorage(&ptr);
    *iter = *iter2;
  }
  return iter;
}

void* PyObjectDictIteratorTypeInterface::clone(void* storage)
{
  Iter* p = (Iter*)ptrFromStorage(&storage);
  return new Iter(*p);
}

void PyObjectDictIteratorTypeInterface::destroy(void* storage)
{
  Iter* p = (Iter*)ptrFromStorage(&storage);
  delete p;
}

// Tuple implementation

std::vector<qi::TypeInterface*> PyObjectIterableTypeInterface::memberTypes()
{
  std::vector<TypeInterface*> ret;
  ret.reserve(_size);
  for (unsigned int i = 0; i < _size; ++i)
    ret.push_back(PyObjectDynamicTypeInterface::getType());
  return ret;
}

std::vector<void*> PyObjectIterableTypeInterface::get(void* storage)
{
  qi::py::GILScopedLock _lock;
  PyObject* tuple = (PyObject*)ptrFromStorage(&storage);
  boost::python::object iter(pyHandle(PyObject_GetIter(tuple)));
  std::vector<void*> res;
  res.reserve(_size);
  PyObject* itemTmp;
  boost::python::object item;
  while ((itemTmp = PyIter_Next(iter.ptr())))
  {
    item = pyHandle(itemTmp);
    res.push_back(item.ptr());
  }
  return res;
}

void* PyObjectIterableTypeInterface::get(void* storage, unsigned int index)
{
  assert(index < _size);

  qi::py::GILScopedLock _lock;
  PyObject* tuple = (PyObject*)ptrFromStorage(&storage);
  boost::python::object iter(pyHandle(PyObject_GetIter(tuple)));
  unsigned int i = 0;
  PyObject* itemTmp;
  boost::python::object item;
  while ((itemTmp = PyIter_Next(iter.ptr())))
  {
    item = pyHandle(itemTmp);
    if (i == index)
      return item.ptr();
    ++i;
  }
  throw std::runtime_error("index out of bound in Iterable::get");
}

void PyObjectIterableTypeInterface::set(void** storage, const std::vector<void*>&)
{
  throw std::runtime_error("cannot set python Iterable");
}

void PyObjectIterableTypeInterface::set(void** storage, unsigned int index, void* valStorage)
{
  throw std::runtime_error("cannot set python Iterable");
}

qi::AnyReference AnyReference_from_PyObject(PyObject* obj)
{
  qi::py::GILScopedLock _lock;
  if (obj == Py_None)
  {
    return qi::AnyReference(qi::typeOf<void>());
  }
  else if (PyInt_CheckExact(obj))
  {
    return qi::AnyReference(PyObjectIntTypeInterface::getType(), obj);
  }
  else if (PyLong_CheckExact(obj))
  {
    return qi::AnyReference(PyObjectLongTypeInterface::getType(), obj);
  }
  else if (PyFloat_CheckExact(obj))
  {
    return qi::AnyReference(PyObjectFloatTypeInterface::getType(), obj);
  }
  else if (PyBool_Check(obj))
  {
    return qi::AnyReference(PyObjectBoolTypeInterface::getType(), obj);
  }
  else if (PyString_CheckExact(obj))
  {
    return qi::AnyReference(PyObjectStringTypeInterface::getType(), obj);
  }
  else if (PyByteArray_CheckExact(obj))
  {
    return qi::AnyReference(PyObjectByteArrayTypeInterface::getType(), obj);
  }
  else if (PyTuple_CheckExact(obj))
  {
    qi::TypeInterface* typeInterface = PyObjectIterableTypeInterface::getType(PyTuple_Size(obj));
    return qi::AnyReference(typeInterface, obj);
  }
  else if (PyAnySet_CheckExact(obj))
  {
    qi::TypeInterface* typeInterface = PyObjectIterableTypeInterface::getType(PySet_Size(obj));
    return qi::AnyReference(typeInterface, obj);
  }
  else if (PyList_CheckExact(obj))
  {
    return qi::AnyReference(PyObjectListTypeInterface::getType(), obj);
  }
  else if (PyDict_CheckExact(obj))
  {
    return qi::AnyReference(PyObjectDictTypeInterface::getType(), obj);
  }
  else if (PyUnicode_CheckExact(obj))
  {
    return qi::AnyReference(PyObjectUnicodeStringTypeInterface::getType(), obj);
  }
  else if (obj == Py_Ellipsis)
  {
    throw std::runtime_error("Type not implemented");
  }
  else if (PyComplex_CheckExact(obj))
  {
    throw std::runtime_error("Type not implemented");
  }
  else if (PyBuffer_Check(obj))
  {
    throw std::runtime_error("Type not implemented");
  }
  else if (PyMemoryView_Check(obj))
  {
    throw std::runtime_error("Type not implemented");
  }
  else if (PyFile_Check(obj))
  {
    throw std::runtime_error("Type not implemented");
  }
  else if (PySlice_Check(obj))
  {
    throw std::runtime_error("Type not implemented");
  }
  else if (PyModule_CheckExact(obj) || PyClass_Check(obj)) {
    throw std::runtime_error("Unable to convert Python Module or Class to Anyobjue");
  }
  else // if (PyInstance_Check(obj)) // instance are old style python class
  {
    qi::AnyReference res = qi::AnyReference::from(qi::py::makeQiAnyObject(pyBorrow(obj))).clone();

    boost::python::object o = boost::python::make_function(&cleanup_ref);
    PyObject* weakRef = PyWeakref_NewRef(obj, o.ptr());
    {
      boost::mutex::scoped_lock _lock(leakMutex);
      leakMap[weakRef] = res;
    }

    return res;
  }
  throw std::runtime_error("Unreachable code reached"); // g++ won't compile without that line
}

/* Register PyObject* -> See the above comment for explanations */
QI_TYPE_REGISTER_CUSTOM(boost::python::object, PyBoostObjectDynamicTypeInterface);
QI_TYPE_REGISTER_CUSTOM(PyObject, PyObjectDynamicTypeInterface);

//register for common wrapper too. (yes we could do better than launching the big BFG, but BFG *does* work.
QI_TYPE_REGISTER_CUSTOM(boost::python::str, PyBoostObjectDynamicTypeInterface);
QI_TYPE_REGISTER_CUSTOM(boost::python::list, PyBoostObjectDynamicTypeInterface);
QI_TYPE_REGISTER_CUSTOM(boost::python::dict, PyBoostObjectDynamicTypeInterface);
QI_TYPE_REGISTER_CUSTOM(boost::python::tuple, PyBoostObjectDynamicTypeInterface);
