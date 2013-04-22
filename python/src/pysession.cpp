/*
**
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2013 Aldebaran Robotics
*/
#include "pysession.hpp"
#include <qimessaging/session.hpp>
#include <boost/python.hpp>
#include <boost/python/stl_iterator.hpp>
#include <qitype/genericobjectbuilder.hpp>
#include "pyfuture.hpp"
#include "gil.hpp"

qiLogCategory("qi.py");

namespace qi { namespace py {

    template <typename T>
    PyFuture toPyFuture(qi::Future<T> fut) {
      qi::Promise<qi::GenericValue> gprom;
      qi::adaptFuture(fut, gprom);
      return gprom.future();
    }

    template <typename T>
    PyFuture toPyFuture(qi::FutureSync<T> fut) {
      qi::Promise<qi::GenericValue> gprom;
      qi::adaptFuture(fut.async(), gprom);
      return gprom.future();
    }


    void addMethod(qi::GenericObjectBuilder& gob, boost::python::object meth) {

    }

    //TODO: DO NOT DUPLICATE
    static qi::GenericValuePtr pysignalCb(const std::vector<qi::GenericValuePtr>& cargs, boost::python::object callable) {
      qi::py::GILScopedLock _lock;
      boost::python::list   args;
      boost::python::object ret;

      std::vector<qi::GenericValuePtr>::const_iterator it;
      for (it = cargs.begin(); it != cargs.end(); ++it) {
        args.append(it->to<boost::python::object>());
      }
      ret = callable(*boost::python::tuple(args));
      return qi::GenericValueRef(ret).clone();
    }


    qi::ObjectPtr makeQiPyObjectPtr(boost::python::object obj)
    {
      qi::ObjectPtr ret;
      qi::GenericObjectBuilder gob;
      boost::python::object attrs(boost::python::handle<>(PyObject_Dir(obj.ptr())));

      //boost::python::list                                    l(attrs);
      //boost::python::stl_input_iterator<boost::python::list> it(l);
      //boost::python::stl_input_iterator<boost::python::list> end;

      for (unsigned i = 0; i < boost::python::len(attrs); ++i) {
      //for (; it != end; ++it) {
        std::string key = boost::python::extract<std::string>(attrs[i]);
        boost::python::object m = obj.attr(attrs[i]);

        //boost::python::object value = (*it)[1];
        std::cout << "ATTR:" << key << std::endl;
        if (PyMethod_Check(m.ptr())) {
          std::cout << "adding method:" << key << std::endl;
          qi::MetaMethodBuilder mmb;
          boost::python::object desc = m.attr("__doc__");

          if (desc)
            mmb.setDescription(boost::python::extract<std::string>(desc));
          gob.xAdvertiseMethod(mmb, qi::makeDynamicGenericFunction(boost::bind(pysignalCb, _1, m)));
        }
        //check for Signal
        //PyObject_IsInstance(m.ptr(), );
        //check for Property
      }


//      +def buildFunctionListFromObject(obj):
//      +  # Construct a list of tuple with function and signature associated
//      +  functionsList = []
//      +  attrs = dir(obj)
//      +
//      +  for attr in attrs:
//      +    if (attr.startswith("_")):
//      +      continue
//      +    fun = getattr(obj, attr)
//      +    signature = getFunctionSignature(fun)
//      +    if (signature is not None):
//      +      functionsList.append((fun, signature))
//      +
//      +  return functionsList


//      // Declare qimessaging object builder
//      -  qi::GenericObjectBuilder ob;
//      -
//      -  // Get iterator on object attributes
//      -  if (!(iter = PyObject_GetIter(attr)))
//      -  {
//      -    qiLogError() << "Register object : Given object attributes is not iterable.";
//      -    qi_raise("qimessaging.session.RegisterError",
//      -             "Register object : Given object attributes is not iterable.");
//      -    return 0;
//      -  }
//      -
//      -  // For each attribute of class
//      -  while ((attribute = PyIter_Next(iter)))
//      -  {
//      -    method = PyTuple_GetItem(attribute, 0);
//      -    sig = PyTuple_GetItem(attribute, 1);
//      -
//      -    // register method using code of qi_object_builder_register_method
//      -    if (PyMethod_Check(method) == true)
//      -    {
//      -      std::string signature(PyString_AsString(sig));
//      -
//      -      qipy_objectbuilder_bind_method((qi_object_builder_t*) &ob, signature.c_str(), method);
//      -    }
//      -  }
      return ret;
    }

    class PySession {
    public:
      PySession()
        : _ses(new qi::Session)
      {
      }

      ~PySession() {
      }

      //return a future, or None (and throw in case of error)
      boost::python::object connect(const std::string &url, bool _async=false) {
        if (_async)
          return boost::python::object(toPyFuture(_ses->connect(url)));
        else {
          qi::Future<void> fut = _ses->connect(url);
          fut.value(); //throw on error
          return boost::python::object();
        }
      }

      //return a future, or None (and throw in case of error)
      boost::python::object close(bool _async=false) {
        if (_async)
          return boost::python::object(toPyFuture(_ses->close()));
        else {
          qi::Future<void> fut = _ses->close();
          fut.value(); //throw on error
          return boost::python::object();
        }
      }

      boost::python::object service(const std::string &name, bool _async=false) {
        if (_async)
          return boost::python::object(toPyFuture(_ses->service(name)));
        else {
          qi::Future<qi::ObjectPtr>  fut = _ses->service(name);
          return boost::python::object(fut.value()); //throw on error
        }
      }

      boost::python::object registerService(const std::string &name, boost::python::object obj, bool _async=false) {
        qi::ObjectPtr qiobj;
        qiobj = makeQiPyObjectPtr(obj);
        qi::Future<unsigned int> fut = _ses->registerService(name, qiobj);
        if (_async)
          return boost::python::object(toPyFuture(fut));
        return boost::python::object(fut.value()); //throw on error
      }

    private:
      boost::shared_ptr<qi::Session> _ses;
    };

    void export_pysession() {
      boost::python::class_<PySession>("Session")
          .def("connect", &PySession::connect, (boost::python::arg("url"), boost::python::arg("_async") = false))
          .def("close", &PySession::close, (boost::python::arg("_async") = false))
          .def("service", &PySession::service, (boost::python::arg("service"), boost::python::arg("_async") = false))
          .def("register_service", &PySession::registerService, (boost::python::arg("name"), boost::python::arg("object"), boost::python::arg("_async") = false))
          ;
    }

  }
}
