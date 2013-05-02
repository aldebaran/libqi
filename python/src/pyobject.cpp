/*
**
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2013 Aldebaran Robotics
*/
#include "pyobject.hpp"
#include <boost/python.hpp>
#include <boost/python/raw_function.hpp>
#include "pyfuture.hpp"
#include "pysignal.hpp"
#include "pyproperty.hpp"
#include "gil.hpp"
#include <qitype/genericobjectbuilder.hpp>

qiLogCategory("qipy.object");

namespace qi { namespace py {


    boost::python::object import_inspect() {
      static bool plouf = false;
      static boost::python::object obj;
      if (!plouf) {
        obj = boost::python::import("inspect");
        plouf = true;
      }
      return obj;
    }

    struct PyQiFunctor {
    public:
      PyQiFunctor(const std::string &funName, qi::ObjectPtr obj)
        : _object(obj)
        , _funName(funName)
      {}

      boost::python::object operator()(boost::python::tuple pyargs, boost::python::dict pykwargs) {
        qi::GenericValue val = qi::GenericValue::from(pyargs);
        bool async = boost::python::extract<bool>(pykwargs.get("_async", false));
        std::string funN = boost::python::extract<std::string>(pykwargs.get("_overload", _funName));
        qiLogDebug() << "calling a method: " << funN << " args:" << qi::encodeJSON(val);

        qi::Future<qi::GenericValuePtr> fut;
        {
          //calling c++, so release the GIL.
          GILScopedUnlock _unlock;
          fut = _object->metaCall(funN, val.asDynamic().asTupleValuePtr());
        }
        if (!async)
          return fut.value().to<boost::python::object>();
        else
          return qi::py::makeFuture(fut);
      }

    public:
      qi::ObjectPtr _object;
      std::string   _funName;
    };


    void populateMethods(boost::python::object pyobj, qi::ObjectPtr obj) {
      qi::MetaObject::MethodMap           mm = obj->metaObject().methodMap();
      qi::MetaObject::MethodMap::iterator it;
      for (it = mm.begin(); it != mm.end(); ++it) {
        qi::MetaMethod &mem = it->second;
        //drop special methods
        if (mem.uid() < qiObjectSpecialMethodMaxUid)
          continue;
        std::vector<std::string> vs = qi::signatureSplit(mem.signature());
        qiLogDebug() << "adding method:" << mem.signature();
        boost::python::object fun = boost::python::raw_function(PyQiFunctor(vs[1].c_str(), obj));
        boost::python::api::setattr(pyobj, vs[1].c_str(), fun);// result
      }
    }

    void populateSignals(boost::python::object pyobj, qi::ObjectPtr obj) {
      qi::MetaObject::SignalMap           mm = obj->metaObject().signalMap();
      qi::MetaObject::SignalMap::iterator it;
      for (it = mm.begin(); it != mm.end(); ++it) {
        qi::MetaSignal &ms = it->second;
        //drop special methods
        if (ms.uid() < qiObjectSpecialMethodMaxUid)
          continue;
        std::vector<std::string> vs = qi::signatureSplit(ms.signature());
        qiLogDebug() << "adding signal:" << ms.signature();
        boost::python::object fun = qi::py::makePySignal(ms.signature());
        boost::python::api::setattr(pyobj, vs[1].c_str(), fun);
      }
    }

    void populateProperties(boost::python::object pyobj, qi::ObjectPtr obj) {
      qi::MetaObject::PropertyMap           mm = obj->metaObject().propertyMap();
      qi::MetaObject::PropertyMap::iterator it;
      for (it = mm.begin(); it != mm.end(); ++it) {
        qi::MetaProperty &mp = it->second;
        //drop special methods
        if (mp.uid() < qiObjectSpecialMethodMaxUid)
          continue;
        std::vector<std::string> vs = qi::signatureSplit(mp.signature());
        qiLogDebug() << "adding property:" << mp.signature();
        boost::python::object fun = qi::py::makePyProperty(mp.signature());
        boost::python::api::setattr(pyobj, vs[1].c_str(), fun);
      }
    }


    class PyQiObject {
    public:
      PyQiObject()
      {}

      PyQiObject(const qi::ObjectPtr &obj)
        : _object(obj)
      {}

      boost::python::object call(boost::python::str pyname, boost::python::tuple pyargs, boost::python::dict pykws) {
        return PyQiFunctor(boost::python::extract<std::string>(pyname), _object)(pyargs, pykws);
      }

      boost::python::object metaObject() {
        return qi::GenericValueRef(_object->metaObject()).to<boost::python::object>();
      }

    public:
      qi::ObjectPtr _object;
    };

    boost::python::object makePyQiObject(qi::ObjectPtr obj, const std::string &name) {
      boost::python::object result = boost::python::object(qi::py::PyQiObject(obj));
      qi::py::populateMethods(result, obj);
      qi::py::populateSignals(result, obj);
      qi::py::populateProperties(result, obj);
      return result;
    }


    //TODO: DO NOT DUPLICATE
    static qi::GenericValuePtr pysignalCb(const std::vector<qi::GenericValuePtr>& cargs, boost::python::object callable) {
      qi::py::GILScopedLock _lock;
      boost::python::list   args;
      boost::python::object ret;

      std::vector<qi::GenericValuePtr>::const_iterator it = cargs.begin();
      ++it; //drop the first arg which is DynamicObject*
      for (; it != cargs.end(); ++it) {
        qiLogDebug() << "argument: " << qi::encodeJSON(*it);
        args.append(it->to<boost::python::object>());
      }
      qiLogDebug() << "before method call";
      ret = callable(*boost::python::tuple(args));
      qi::GenericValuePtr gvret = qi::GenericValueRef(ret).clone();
      qiLogDebug() << "method returned:" << qi::encodeJSON(gvret);
      return gvret;
    }

    //callback just needed to keep a ref on obj
    static void doNothing(qi::GenericObject *go, boost::python::object obj)
    {
      (void)go;
      (void)obj;
    }

    qi::ObjectPtr makeQiObjectPtr(boost::python::object obj)
    {
      qi::GenericObjectBuilder gob;
      boost::python::object attrs(boost::python::handle<>(PyObject_Dir(obj.ptr())));

      for (unsigned i = 0; i < boost::python::len(attrs); ++i) {
        std::string key = boost::python::extract<std::string>(attrs[i]);
        boost::python::object m = obj.attr(attrs[i]);
        if (PyMethod_Check(m.ptr())) {
          qi::MetaMethodBuilder mmb(key);
          boost::python::object desc = m.attr("__doc__");
          if (desc)
            mmb.setDescription(boost::python::extract<std::string>(desc));
          boost::python::object inspect = import_inspect();
          //returns: (args, varargs, keywords, defaults)
          boost::python::object tu = inspect.attr("getargspec")(m);
          int argsz = boost::python::len(tu[0]);

          argsz = argsz > 0 ? argsz - 1 : 0;

          if (argsz < 0) {
            qiLogError() << "Method " << key << " is missing the self argument.";
            continue;
          }
          std::stringstream ss;
          ss << key << "::(";
          for (int i = 0; i < argsz; ++i)
            ss << "m";
          ss << ")";
          qiLogDebug() << "Adding method: " << ss.str();
          mmb.setSignature(ss.str());
          mmb.setSigreturn("m");
          gob.xAdvertiseMethod(mmb, qi::makeDynamicGenericFunction(boost::bind(pysignalCb, _1, m)));
          continue;
        }

        //store a pointer on PySignal class
        static boost::python::object asignal = qi::py::makePySignal("(i)").attr("__class__");
        if (PyObject_IsInstance(m.ptr(), asignal.ptr())) {
          qiLogDebug() << "Adding signal:" << key;
          //TODO: register the signal, and get the it to link it to the python one.
          int sig = gob.xAdvertiseEvent(key + "::(i)");
          //TODO: make py.trigger call cpp.trigger
          //TODO: make cpp.callback call py.trigger
          continue;
        }

        //TODO: check for Property
        static boost::python::object aproperty = qi::py::makePyProperty("(i)").attr("__class__");
        if (PyObject_IsInstance(m.ptr(), aproperty.ptr())) {
          qiLogDebug() << "Adding property:" << key;
          continue;
        }

      }
      //this is a useless callback, needed to keep a ref on obj.
      //when the GO is destructed, the ref is released.
      return gob.object(boost::bind<void>(&doNothing, _1, obj));
    }

    void export_pyobject() {
      boost::python::class_<qi::py::PyQiObject>("Object")
          .def("call", &qi::py::PyQiObject::call, (boost::python::arg("name"), boost::python::arg("args") = boost::python::tuple(), boost::python::arg("kargs") = boost::python::dict()))
          .def("metaObject", &qi::py::PyQiObject::metaObject);

      //import inspect in our current namespace
      import_inspect();
    }
  }
}
