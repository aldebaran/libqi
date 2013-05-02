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
#include <qitype/genericvalue.hpp>

namespace qi {
  namespace py {

    //all blocking function are wrapped here to unlock the GIL while blocking.
    class PyFuture : public qi::Future<qi::GenericValue> {
    public:
      PyFuture();
      PyFuture(const PyFuture& fut);
      PyFuture(const qi::Future<qi::GenericValue>& fut);
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
      qi::Promise<qi::GenericValue> gprom;
      qi::adaptFuture(fut, gprom);
      return gprom.future();
    }

    //convert from FutureSync to PyFuture
    template <typename T>
    PyFuture toPyFuture(qi::FutureSync<T> fut) {
      qi::Promise<qi::GenericValue> gprom;
      qi::adaptFuture(fut.async(), gprom);
      return gprom.future();
    }

    boost::python::object makeFuture(qi::Future<qi::GenericValuePtr> fut);
    void export_pyfuture();

  }
}


#endif	    /* !QIFUTURE_PP_ */
