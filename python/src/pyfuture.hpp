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

    class PyFuture : public qi::Future<qi::GenericValue> {
    public:
      PyFuture();
      PyFuture(const PyFuture& fut);
      PyFuture(const qi::Future<qi::GenericValue>& fut);
      boost::python::object value(int msecs = qi::FutureTimeout_Infinite) const;
      //Future::error return a const ref...
      //I'am lazy I dont want to tweak boost.py call policies!
      //so here is the overridden version returning a simple string
      std::string error(int msecs = qi::FutureTimeout_Infinite) const;
      void add_callback(boost::python::object callable);
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
