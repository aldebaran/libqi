/*
**
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2013 Aldebaran Robotics
*/


#ifndef   	PYFUTURE_HPP_
# define   	PYFUTURE_HPP_

#include <boost/python.hpp>
#include <qi/future.hpp>
#include <qitype/anyvalue.hpp>
#include <boost/smart_ptr/enable_shared_from_this.hpp>

namespace qi {
  namespace py {

    //all blocking function are wrapped here to unlock the GIL while blocking.
    //PyFuture should always be a shared_ptr, because boost::python provide convenient
    //converter between shared_ptr and pyobject refcount. this allow us to get the python
    //object associated to this in add_callback.
    //see the fac of boost::python for more information
    class PyFuture : public qi::Future<qi::AnyValue>, public boost::enable_shared_from_this<PyFuture> {
    public:
      PyFuture();
      PyFuture(const PyFuture& fut);
      PyFuture(const qi::Future<qi::AnyValue>& fut);
      boost::python::object value(int msecs = qi::FutureTimeout_Infinite) const;
      std::string error(int msecs = qi::FutureTimeout_Infinite) const;
      void add_callback(boost::python::object callable);
      FutureState wait(int msecs) const;
      bool        hasError(int msecs) const;
      bool        hasValue(int msecs) const;
    };


    //convert from Future to PyFuture
    template <typename T>
    PyFuture toPyFuture(qi::Future<T> fut) {
      qi::Promise<qi::AnyValue> gprom;
      qi::adaptFuture(fut, gprom);
      return gprom.future();
    }

    //convert from FutureSync to PyFuture
    template <typename T>
    PyFuture toPyFuture(qi::FutureSync<T> fut) {
      return toPyFuture(fut.async());
    }

    //async == true  => convert to PyFuture
    //async == false => convert to PyObject or throw on error
    template <typename T>
    boost::python::object toPyFutureAsync(qi::Future<T> fut, bool async) {
      if (async)
        return boost::python::object(toPyFuture(fut));
      qi::AnyReference r(fut.value());
      return r.to<boost::python::object>(); //throw on error
    }

    template <>
    inline boost::python::object toPyFutureAsync<void>(qi::Future<void> fut, bool async) {
      if (async)
        return boost::python::object(toPyFuture(fut));
      fut.value(); //wait for the result
      return boost::python::object(); //throw on error
    }

    template <typename T>
    boost::python::object toPyFutureAsync(qi::FutureSync<T> fut, bool async) {
      return toPyFutureAsync(fut.async(), async);
    }

    boost::python::object makeFuture(qi::Future<qi::AnyReference> fut);
    void export_pyfuture();

  }
}


#endif	    /* !QIFUTURE_PP_ */
