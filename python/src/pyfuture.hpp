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

    //needed in .hpp by some adapter in PySession.
    class PyFuture : public qi::Future<qi::GenericValue> {
    public:
      PyFuture() {}
      PyFuture(const PyFuture& fut)
        : qi::Future<qi::GenericValue>(fut)
      {}

      PyFuture(const qi::Future<qi::GenericValue>& fut)
        : qi::Future<qi::GenericValue>(fut)
      {}

      boost::python::object value(int msecs = qi::FutureTimeout_Infinite) const {
        qi::GenericValue gv = qi::Future<qi::GenericValue>::value(msecs);
        return gv.to<boost::python::object>();
      }

      //Future::error return a const ref... I'am lazy I dont want to tweak boost.py call policies!
      std::string error(int msecs = qi::FutureTimeout_Infinite) const {
        return qi::Future<qi::GenericValue>::error(msecs);
      }

      void add_callback(boost::python::object callable) {
        connect(boost::bind<void>(callable, this));
      }

    };

    boost::python::object makeFuture(qi::Future<qi::GenericValuePtr> fut);

    void export_pyfuture();

  }
}


#endif	    /* !QIFUTURE_PP_ */
