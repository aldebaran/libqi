/*
**  Copyright (C) 2013 Aldebaran Robotics
**  See COPYING for the license
*/
#include "pyobject.hpp"
#include <boost/python.hpp>
#include <boost/python/raw_function.hpp>
#include "pyfuture.hpp"
#include "pysignal.hpp"
#include "pyproperty.hpp"
#include "gil.hpp"
#include "error.hpp"
#include <qitype/dynamicobjectbuilder.hpp>
#include <qitype/jsoncodec.hpp>

qiLogCategory("qipy.object");

namespace qi { namespace py {


    boost::python::object importInspect() {
      static bool plouf = false;
      static boost::python::object& obj = *new boost::python::object;
      if (!plouf) {
        obj = boost::python::import("inspect");
        plouf = true;
      }
      return obj;
    }

    static void cleanup(LeakBlock* block)
    {
      delete block;
    }

    struct PyQiFunctor {
    public:
      PyQiFunctor(const std::string &funName, qi::AnyObject obj)
        : _object(obj)
        , _funName(funName)
      {}

      boost::python::object operator()(boost::python::tuple pyargs, boost::python::dict pykwargs) {
        qi::py::LeakBlock*  leakBlock = new LeakBlock();
        qi::AnyValue val = qi::AnyValue::from(pyargs);
        bool async = boost::python::extract<bool>(pykwargs.get("_async", false));
        std::string funN = boost::python::extract<std::string>(pykwargs.get("_overload", _funName));
        qiLogDebug() << "calling a method: " << funN << " args:" << qi::encodeJSON(val);


        qi::Future<qi::AnyValue> fut;
        qi::Promise<qi::AnyValue> res(qi::FutureCallbackType_Sync);
        PyPromise pyprom(res);
        {
          //calling c++, so release the GIL.
          GILScopedUnlock _unlock;
          qi::Future<qi::AnyReference> fmeta = _object.metaCall(funN, val.asDynamic().asTupleValuePtr());
          //because futureAdapter support AnyRef containing Future<T>  (that will be converted to a Future<T>
          // instead of Future<Future<T>>
          fmeta.connect(boost::bind<void>(&detail::futureAdapter<qi::AnyValue>, _1, res));
          fut = res.future();
          fut.connect(&cleanup, leakBlock);
        }
        if (!async) {
          {
            //do not lock while waiting!
            GILScopedUnlock _unlock;
            fut.wait();
          }
          return fut.value().to<boost::python::object>();
        }
        return boost::python::object(pyprom.future());
      }

    public:
      qi::AnyObject _object;
      std::string   _funName;
    };

    void populateMethods(boost::python::object pyobj, qi::AnyObject obj) {
      qi::MetaObject::MethodMap           mm = obj.metaObject().methodMap();
      qi::MetaObject::MethodMap::iterator it;
      for (it = mm.begin(); it != mm.end(); ++it) {
        qi::MetaMethod &mem = it->second;
        //drop special methods
        if (mem.uid() < qiObjectSpecialMemberMaxUid)
          continue;
        qiLogDebug() << "adding method:" << mem.toString();
        boost::python::object fun = boost::python::raw_function(PyQiFunctor(mem.name().c_str(), obj));
        boost::python::api::setattr(pyobj, mem.name().c_str(), fun);
        // Fill __doc__ with Signature and description
        std::stringstream ssdocstring;
        ssdocstring << "Signature: " << mem.returnSignature().toString() << "\n";
        ssdocstring << mem.description();
        boost::python::object docstring(ssdocstring.str());
        boost::python::api::setattr(fun, "__doc__", docstring);
      }
    }

    void populateSignals(boost::python::object pyobj, qi::AnyObject obj) {
      qi::MetaObject::SignalMap           mm = obj.metaObject().signalMap();
      qi::MetaObject::SignalMap::iterator it;
      for (it = mm.begin(); it != mm.end(); ++it) {
        qi::MetaSignal &ms = it->second;
        //drop special methods
        if (ms.uid() < qiObjectSpecialMemberMaxUid)
          continue;
        qiLogDebug() << "adding signal:" << ms.toString();
        boost::python::object fun = qi::py::makePyProxySignal(obj, ms);
        boost::python::api::setattr(pyobj, ms.name(), fun);
      }
    }

    void populateProperties(boost::python::object pyobj, qi::AnyObject obj) {
      qi::MetaObject::PropertyMap           mm = obj.metaObject().propertyMap();
      qi::MetaObject::PropertyMap::iterator it;
      for (it = mm.begin(); it != mm.end(); ++it) {
        qi::MetaProperty &mp = it->second;
        //drop special methods
        if (mp.uid() < qiObjectSpecialMemberMaxUid)
          continue;
        qiLogDebug() << "adding property:" << mp.toString();
        boost::python::object fun = qi::py::makePyProxyProperty(obj, mp);
        boost::python::api::setattr(pyobj, mp.name().c_str(), fun);
      }
    }


    class PyQiObject {
    public:
      PyQiObject()
      {}

      PyQiObject(const qi::AnyObject &obj)
        : _object(obj)
      {}

      boost::python::object call(boost::python::str pyname, boost::python::tuple pyargs, boost::python::dict pykws) {
        return PyQiFunctor(boost::python::extract<std::string>(pyname), _object)(pyargs, pykws);
      }

      boost::python::object metaObject() {
        return qi::AnyReference(_object.metaObject()).to<boost::python::object>();
      }

      qi::AnyObject object() {
        return _object;
      }

    private:
      qi::AnyObject _object;
    };

    boost::python::object makePyQiObject(qi::AnyObject obj, const std::string &name) {
      boost::python::object result = boost::python::object(qi::py::PyQiObject(obj));
      qi::py::populateMethods(result, obj);
      qi::py::populateSignals(result, obj);
      qi::py::populateProperties(result, obj);
      return result;
    }


    //TODO: DO NOT DUPLICATE
    static qi::AnyReference pyCallMethod(const std::vector<qi::AnyReference>& cargs, boost::python::object callable) {
      qi::AnyReference gvret;
      try {
        qi::py::LeakBlock leakBlock;
        qi::py::GILScopedLock _lock;
        boost::python::list   args;
        boost::python::object ret;

        std::vector<qi::AnyReference>::const_iterator it = cargs.begin();
        ++it; //drop the first arg which is DynamicObject*
        for (; it != cargs.end(); ++it) {
          qiLogDebug() << "argument: " << qi::encodeJSON(*it);
          args.append(it->to<boost::python::object>());
        }
        qiLogDebug() << "before method call";
        try {
          ret = callable(*boost::python::tuple(args));
        } catch (const boost::python::error_already_set &e) {
          std::string err = PyFormatError();
          qiLogDebug() << "call exception:" << err;
          throw std::runtime_error(err);
        }

        //convert python future to future, to allow the middleware to make magic with it.
        //serverresult will support async return value for call. (call returning future)
        boost::python::extract< PyFuture* > extractor(ret);
        if (extractor.check()) {
          PyFuture* pfut = extractor();
          if (pfut) { //pfut == 0, can mean ret is None.
            qiLogDebug() << "Future detected";
            qi::Future<qi::AnyValue> fut = *pfut;
            return qi::AnyReference(fut).clone();
          }
        }

        gvret = qi::AnyReference(ret).clone();
        qiLogDebug() << "method returned:" << qi::encodeJSON(gvret);
      } catch (const boost::python::error_already_set &e) {
        throw std::runtime_error("python failed");
      }
      return gvret;
    }

    //callback just needed to keep a ref on obj
    static void doNothing(qi::GenericObject *go, boost::python::object obj)
    {
      (void)go;
      (void)obj;
    }

    //get the signature for the function
    //if vargs => return m
    //else: return (m...m) with the good number of m
    std::string generateDefaultParamSignature(const std::string &key, boost::python::object& argspec)
    {
      //argspec[0] = args
      int argsz = boost::python::len(argspec[0]);

      //argspec[1] = name of vargs
      if (argspec[1])
      {
        //m is accept everything
        return "m";
      }

      if (argsz == 0) {
        std::stringstream serr;
        serr << "Method " << key << " is missing the self argument.";
        throw std::runtime_error(serr.str());
      }

      //drop self.
      argsz = argsz - 1;

      std::stringstream ss;

      ss << "(";
      for (int i = 0; i < argsz; ++i)
        ss << "m";
      ss << ")";
      return ss.str();
    }

    void registerMethod(qi::DynamicObjectBuilder& gob, const std::string& key, boost::python::object& method, const std::string& qisig)
    {
      qi::MetaMethodBuilder mmb;
      mmb.setName(key);
      boost::python::object desc = method.attr("__doc__");
      boost::python::object pyqiretsig = boost::python::getattr(method, "__qi_return_signature__", boost::python::object());
      if (desc)
        mmb.setDescription(boost::python::extract<std::string>(desc));
      boost::python::object inspect = importInspect();
      //returns: (args, varargs, keywords, defaults)
      boost::python::object tu = inspect.attr("getargspec")(method);


      std::string defparamsig = generateDefaultParamSignature(key, tu);

      qiLogDebug() << "Adding method: " << defparamsig;
      if (!qisig.empty())
        mmb.setParametersSignature(qisig);
      else
        mmb.setParametersSignature(defparamsig);

      std::string qiretsig;
      if (pyqiretsig) {
        qiretsig = boost::python::extract<std::string>(pyqiretsig);
      }

      if (!qiretsig.empty())
        mmb.setReturnSignature(qiretsig);
      else
        mmb.setReturnSignature("m");

      // Throw on error
      gob.xAdvertiseMethod(mmb, qi::AnyFunction::fromDynamicFunction(boost::bind(pyCallMethod, _1, method)));
    }

    qi::AnyObject makeQiAnyObject(boost::python::object obj)
    {
      //is that a qi::AnyObject?
      boost::python::extract<PyQiObject*> isthatyoumum(obj);

      if (isthatyoumum.check()) {
        qiLogDebug() << "this PyObject is already a qi::AnyObject. Just returning it.";
        return isthatyoumum()->object();
      }

      qi::DynamicObjectBuilder gob;
      //let the GIL handle the thread-safety for us
      gob.setThreadingModel(ObjectThreadingModel_MultiThread);
      GILScopedLock _lock;
      boost::python::object attrs(boost::python::borrowed(PyObject_Dir(obj.ptr())));

      for (int i = 0; i < boost::python::len(attrs); ++i) {
        std::string key = boost::python::extract<std::string>(attrs[i]);
        boost::python::object m = obj.attr(attrs[i]);

        boost::python::object pyqiname = boost::python::getattr(m, "__qi_name__", boost::python::object());
        boost::python::object pyqisig = boost::python::getattr(m, "__qi_signature__", boost::python::object());
        std::string qisig;

        if (pyqisig)
          qisig = boost::python::extract<std::string>(pyqisig);

        if (qisig == "DONOTBIND")
          continue;
        //override name by the one provide by @bind
        if (pyqiname)
          key = boost::python::extract<std::string>(pyqiname);

        if (PyMethod_Check(m.ptr())) {
          registerMethod(gob, key, m, qisig);

          continue;
        }

        //store a pointer on PySignal class
        static boost::python::object asignal = qi::py::makePySignal("(i)").attr("__class__");
        if (PyObject_IsInstance(m.ptr(), asignal.ptr())) {
          qiLogDebug() << "Adding signal:" << key;
          gob.advertiseSignal(key, qi::py::getSignal(m));
          continue;
        }

        //TODO: check for Property
        static boost::python::object aproperty = qi::py::makePyProperty("(i)").attr("__class__");
        if (PyObject_IsInstance(m.ptr(), aproperty.ptr())) {
          qiLogDebug() << "Adding property:" << key;
          gob.advertiseProperty(key, qi::py::getProperty(m));
          continue;
        }

      }
      //this is a useless callback, needed to keep a ref on obj.
      //when the GO is destructed, the ref is released.
      return gob.object(boost::bind<void>(&doNothing, _1, obj));
    }

    static boost::python::object pyobjectParamShrinker(boost::python::tuple args, boost::python::dict kwargs) {
      PyQiObject& pys = boost::python::extract<PyQiObject&>(args[0]);
      boost::python::list l;
      for (int i = 2; i < boost::python::len(args); ++i)
        l.append(args[i]);
      return pys.call(boost::python::extract<boost::python::str>(args[1]), boost::python::tuple(l), kwargs);
    }

    static boost::python::object pyobjectParamShrinkerAsync(boost::python::tuple args, boost::python::dict kwargs) {
      PyQiObject& pys = boost::python::extract<PyQiObject&>(args[0]);
      boost::python::list l;
      for (int i = 2; i < boost::python::len(args); ++i)
        l.append(args[i]);
      kwargs.attr("_async") = true;
      return pys.call(boost::python::extract<boost::python::str>(args[1]), boost::python::tuple(l), kwargs);
    }


    void export_pyobject() {
      boost::python::class_<qi::py::PyQiObject>("Object", boost::python::no_init)
          .def("call", boost::python::raw_function(&pyobjectParamShrinker, 1))
          .def("async", boost::python::raw_function(&pyobjectParamShrinkerAsync, 1))
          //TODO: .def("post")
          //TODO: .def("setProperty")
          //TODO: .def("property")
          .def("metaObject", &qi::py::PyQiObject::metaObject);
      //import inspect in our current namespace
      importInspect();
    }
  }
}
